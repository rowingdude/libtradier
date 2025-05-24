/*
 * libtradier - Tradier API C++ Library v0.1.0
 *
 * Author: Benjamin Cance (kc8bws@kc8bws.com)
 * Date: 2025-05-22
 *
 * This software is provided free of charge under the MIT License.
 * By using it, you agree to absolve the author of all liability.
 * See LICENSE file for full terms and conditions.
 */

#include "tradier/auth.hpp"
#include "tradier/client.hpp"
#include "tradier/common/errors.hpp"
#include "tradier/common/utils.hpp"
#include <nlohmann/json.hpp>
#include <random>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <regex>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <curl/curl.h>
#include <memory>
#include <algorithm>

namespace tradier {

class SecureString {
private:
    std::vector<unsigned char> data_;
    
public:
    explicit SecureString(const std::string& str) {
        data_.resize(str.size());
        std::copy(str.begin(), str.end(), data_.begin());
    }
    
    explicit SecureString(size_t size) : data_(size) {
        if (RAND_bytes(data_.data(), static_cast<int>(size)) != 1) {
            throw std::runtime_error("Failed to generate random bytes");
        }
    }
    
    ~SecureString() {
        if (!data_.empty()) {
            OPENSSL_cleanse(data_.data(), data_.size());
        }
    }
    
    SecureString(const SecureString&) = delete;
    SecureString& operator=(const SecureString&) = delete;
    
    SecureString(SecureString&& other) noexcept : data_(std::move(other.data_)) {}
    
    SecureString& operator=(SecureString&& other) noexcept {
        if (this != &other) {
            if (!data_.empty()) {
                OPENSSL_cleanse(data_.data(), data_.size());
            }
            data_ = std::move(other.data_);
        }
        return *this;
    }
    
    std::string toString() const {
        return std::string(data_.begin(), data_.end());
    }
    
    size_t size() const { return data_.size(); }
    const unsigned char* data() const { return data_.data(); }
};

class CryptoContext {
private:
    EVP_MD_CTX* ctx_;

public:
    CryptoContext() : ctx_(EVP_MD_CTX_new()) {
        if (!ctx_) {
            throw std::runtime_error("Failed to create crypto context");
        }
    }
    
    ~CryptoContext() {
        if (ctx_) {
            EVP_MD_CTX_free(ctx_);
        }
    }
    
    CryptoContext(const CryptoContext&) = delete;
    CryptoContext& operator=(const CryptoContext&) = delete;
    
    EVP_MD_CTX* get() const { return ctx_; }
};

bool TokenInfo::hasScope(TokenScope scope) const {
    return std::find(scopes.begin(), scopes.end(), scope) != scopes.end();
}

bool TokenInfo::isExpired() const {
    return std::chrono::system_clock::now() >= expiresAt;
}

bool TokenInfo::isExpiringSoon(std::chrono::minutes threshold) const {
    auto expiryTime = expiresAt - threshold;
    return std::chrono::system_clock::now() >= expiryTime;
}

std::string TokenInfo::getScopeString() const {
    return AuthService::scopesToString(scopes);
}

long TokenInfo::getSecondsUntilExpiry() const {
    auto now = std::chrono::system_clock::now();
    if (now >= expiresAt) return 0;
    
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(expiresAt - now);
    return duration.count();
}

AuthEndpoints AuthEndpoints::forEnvironment(bool sandbox) {
    AuthEndpoints endpoints;
    
    if (sandbox) {
        endpoints.authorizationUrl = "https://sandbox.tradier.com/oauth/authorize";
        endpoints.accessTokenUrl = "https://sandbox.tradier.com/oauth/accesstoken";
        endpoints.refreshTokenUrl = "https://sandbox.tradier.com/oauth/accesstoken"; 
        endpoints.revokeTokenUrl = "https://sandbox.tradier.com/oauth/revoke";
        endpoints.userProfileUrl = "https://sandbox.tradier.com/v1/user/profile";
    } else {
        endpoints.authorizationUrl = "https://api.tradier.com/oauth/authorize";
        endpoints.accessTokenUrl = "https://api.tradier.com/oauth/accesstoken";
        endpoints.refreshTokenUrl = "https://api.tradier.com/oauth/accesstoken";
        endpoints.revokeTokenUrl = "https://api.tradier.com/oauth/revoke";
        endpoints.userProfileUrl = "https://api.tradier.com/v1/user/profile";
    }
    
    return endpoints;
}

AuthService::AuthService(TradierClient& client, const AuthConfig& config) 
    : client_(client), config_(config), endpoints_(AuthEndpoints::forEnvironment(client.config().sandboxMode)) {
}

std::string AuthService::generateRandomString(size_t length) const {
    const std::string charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~";
    
    SecureString randomBytes(length);
    std::string result;
    result.reserve(length);
    
    for (size_t i = 0; i < length; ++i) {
        result += charset[randomBytes.data()[i] % charset.size()];
    }
    
    return result;
}

std::string AuthService::generateCodeVerifier() const {
    return generateRandomString(128);
}

std::string AuthService::generateCodeChallenge(const std::string& verifier) const {
    std::string hash = sha256(verifier);
    return base64UrlEncode(hash);
}

std::string AuthService::sha256(const std::string& input) const {
    CryptoContext ctx;
    
    if (EVP_DigestInit_ex(ctx.get(), EVP_sha256(), nullptr) != 1) {
        throw std::runtime_error("Failed to initialize SHA256");
    }
    
    if (EVP_DigestUpdate(ctx.get(), input.c_str(), input.length()) != 1) {
        throw std::runtime_error("Failed to update SHA256");
    }
    
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen;
    
    if (EVP_DigestFinal_ex(ctx.get(), hash, &hashLen) != 1) {
        throw std::runtime_error("Failed to finalize SHA256");
    }
    
    return std::string(reinterpret_cast<char*>(hash), hashLen);
}

std::string AuthService::base64UrlEncode(const std::string& input) const {
    static const char base64_chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string result;
    int i = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    size_t input_len = input.size();
    const unsigned char* bytes_to_encode = 
        reinterpret_cast<const unsigned char*>(input.c_str());

    while (input_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; i < 4; i++)
                result += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for (int j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (int j = 0; j < i + 1; j++)
            result += base64_chars[char_array_4[j]];

        while (i++ < 3)
            result += '=';
    }

    std::replace(result.begin(), result.end(), '+', '-');
    std::replace(result.begin(), result.end(), '/', '_');
    result.erase(std::remove(result.begin(), result.end(), '='), result.end());
    
    return result;
}

void AuthService::cleanupExpiredStates() {
    auto now = std::chrono::system_clock::now();
    auto it = stateCache_.begin();
    
    while (it != stateCache_.end()) {
        if (now > it->second + config_.stateExpiration) {
            it = stateCache_.erase(it);
        } else {
            ++it;
        }
    }
}

std::string AuthService::getAuthorizationUrl() {
    return getAuthorizationUrl(config_.requestedScopes);
}

std::string AuthService::getAuthorizationUrl(const std::vector<TokenScope>& scopes) {
    cleanupExpiredStates();
    
    currentState_ = generateRandomString(32);
    stateCache_[currentState_] = std::chrono::system_clock::now();
    
    if (config_.usePKCE) {
        codeVerifier_ = generateCodeVerifier();
        codeChallenge_ = generateCodeChallenge(codeVerifier_);
    }
    
    std::ostringstream url;
    url << endpoints_.authorizationUrl
        << "?response_type=code"
        << "&client_id=" << utils::urlEncode(config_.clientId)
        << "&redirect_uri=" << utils::urlEncode(config_.redirectUri)
        << "&scope=" << utils::urlEncode(scopesToString(scopes))
        << "&state=" << utils::urlEncode(currentState_);
    
    if (config_.usePKCE) {
        url << "&code_challenge=" << utils::urlEncode(codeChallenge_)
            << "&code_challenge_method=S256";
    }
    
    return url.str();
}

TokenInfo AuthService::exchangeAuthorizationCode(const std::string& authCode, const std::string& state) {
    if (authCode.empty()) {
        throw ValidationError("Authorization code cannot be empty");
    }
    
    if (!state.empty() && !validateState(state)) {
        throw ValidationError("Invalid or expired state parameter");
    }
    
    FormParams params;
    params["grant_type"] = "authorization_code";
    params["code"] = authCode;
    params["client_id"] = config_.clientId;
    params["client_secret"] = config_.clientSecret;
    params["redirect_uri"] = config_.redirectUri;
    
    if (config_.usePKCE && !codeVerifier_.empty()) {
        params["code_verifier"] = codeVerifier_;
    }
    
    try {
        // Create a minimal HTTP client for auth endpoints (bypass TradierClient token requirement)
        CURL* curl = curl_easy_init();
        if (!curl) {
            throw ConnectionError("Failed to initialize CURL for auth");
        }
        
        // Prepare form data
        std::string postData;
        bool first = true;
        for (const auto& [key, value] : params) {
            if (!first) postData += "&";
            postData += key + "=" + value; // Note: should URL encode in production
            first = false;
        }
        
        std::string responseBody;
        curl_easy_setopt(curl, CURLOPT_URL, endpoints_.accessTokenUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](void* contents, size_t size, size_t nmemb, std::string* data) -> size_t {
            data->append(static_cast<char*>(contents), size * nmemb);
            return size * nmemb;
        });
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
        
        CURLcode res = curl_easy_perform(curl);
        long statusCode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);
        curl_easy_cleanup(curl);
        
        if (res != CURLE_OK || statusCode < 200 || statusCode >= 300) {
            throw ApiError(static_cast<int>(statusCode), "Token exchange failed: " + responseBody);
        }
        
        auto json = nlohmann::json::parse(responseBody);
        
        TokenInfo tokenInfo;
        tokenInfo.accessToken = json.value("access_token", "");
        tokenInfo.refreshToken = json.value("refresh_token", "");
        tokenInfo.tokenType = json.value("token_type", "Bearer");
        
        int expiresIn = json.value("expires_in", 86400);
        tokenInfo.issuedAt = std::chrono::system_clock::now();
        tokenInfo.expiresAt = tokenInfo.issuedAt + std::chrono::seconds(expiresIn);
        
        std::string scopeString = json.value("scope", "");
        tokenInfo.scopes = parseScopeString(scopeString);
        tokenInfo.isValid = !tokenInfo.accessToken.empty();
        
        if (tokenRefreshCallback_) {
            tokenRefreshCallback_(tokenInfo);
        }
        
        clearState();
        return tokenInfo;
        
    } catch (const nlohmann::json::exception& e) {
        throw ApiError(400, "Invalid token response: " + std::string(e.what()));
    } catch (const std::exception& e) {
        if (errorCallback_) {
            errorCallback_("Token exchange error: " + std::string(e.what()));
        }
        throw;
    }
}

TokenInfo AuthService::refreshAccessToken(const std::string& refreshToken) {
    if (refreshToken.empty()) {
        throw ValidationError("Refresh token cannot be empty");
    }
    
    FormParams params;
    params["grant_type"] = "refresh_token";
    params["refresh_token"] = refreshToken;
    params["client_id"] = config_.clientId;
    params["client_secret"] = config_.clientSecret;
    
    try {
        // Create a minimal HTTP client for auth endpoints (bypass TradierClient token requirement)
        CURL* curl = curl_easy_init();
        if (!curl) {
            throw ConnectionError("Failed to initialize CURL for auth");
        }
        
        // Prepare form data
        std::string postData;
        bool first = true;
        for (const auto& [key, value] : params) {
            if (!first) postData += "&";
            postData += key + "=" + value; // Note: should URL encode in production
            first = false;
        }
        
        std::string responseBody;
        curl_easy_setopt(curl, CURLOPT_URL, endpoints_.refreshTokenUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](void* contents, size_t size, size_t nmemb, std::string* data) -> size_t {
            data->append(static_cast<char*>(contents), size * nmemb);
            return size * nmemb;
        });
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
        
        CURLcode res = curl_easy_perform(curl);
        long statusCode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);
        curl_easy_cleanup(curl);
        
        if (res != CURLE_OK || statusCode < 200 || statusCode >= 300) {
            throw ApiError(static_cast<int>(statusCode), "Token refresh failed: " + responseBody);
        }
        
        auto json = nlohmann::json::parse(responseBody);
        
        TokenInfo tokenInfo;
        tokenInfo.accessToken = json.value("access_token", "");
        tokenInfo.refreshToken = json.value("refresh_token", refreshToken);
        tokenInfo.tokenType = json.value("token_type", "Bearer");
        
        int expiresIn = json.value("expires_in", 86400);
        tokenInfo.issuedAt = std::chrono::system_clock::now();
        tokenInfo.expiresAt = tokenInfo.issuedAt + std::chrono::seconds(expiresIn);
        
        std::string scopeString = json.value("scope", "");
        tokenInfo.scopes = parseScopeString(scopeString);
        tokenInfo.isValid = !tokenInfo.accessToken.empty();
        
        if (tokenRefreshCallback_) {
            tokenRefreshCallback_(tokenInfo);
        }
        
        return tokenInfo;
        
    } catch (const nlohmann::json::exception& e) {
        throw ApiError(400, "Invalid refresh response: " + std::string(e.what()));
    } catch (const std::exception& e) {
        if (errorCallback_) {
            errorCallback_("Token refresh error: " + std::string(e.what()));
        }
        throw;
    }
}

bool AuthService::revokeToken(const std::string& token, TokenType type) {
    if (token.empty()) {
        throw ValidationError("Token cannot be empty");
    }
    
    FormParams params;
    params["token"] = token;
    params["token_type_hint"] = (type == TokenType::REFRESH_TOKEN) ? "refresh_token" : "access_token";
    params["client_id"] = config_.clientId;
    params["client_secret"] = config_.clientSecret;
    
    try {
        // Create a minimal HTTP client for auth endpoints (bypass TradierClient token requirement)
        CURL* curl = curl_easy_init();
        if (!curl) {
            throw ConnectionError("Failed to initialize CURL for auth");
        }
        
        // Prepare form data
        std::string postData;
        bool first = true;
        for (const auto& [key, value] : params) {
            if (!first) postData += "&";
            postData += key + "=" + value; // Note: should URL encode in production
            first = false;
        }
        
        std::string responseBody;
        curl_easy_setopt(curl, CURLOPT_URL, endpoints_.revokeTokenUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](void* contents, size_t size, size_t nmemb, std::string* data) -> size_t {
            data->append(static_cast<char*>(contents), size * nmemb);
            return size * nmemb;
        });
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
        
        CURLcode res = curl_easy_perform(curl);
        long statusCode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);
        curl_easy_cleanup(curl);
        
        return (res == CURLE_OK && statusCode >= 200 && statusCode < 300);
    } catch (const std::exception&) {
        return false;
    }
}

bool AuthService::validateToken(const std::string& token) {
    if (token.empty()) {
        return false;
    }
    
    try {
        CURL* curl = curl_easy_init();
        if (!curl) {
            return false;
        }
        
        std::string responseBody;
        std::string authHeader = "Authorization: Bearer " + token;
        
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, authHeader.c_str());
        
        curl_easy_setopt(curl, CURLOPT_URL, endpoints_.userProfileUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](void* contents, size_t size, size_t nmemb, std::string* data) -> size_t {
            data->append(static_cast<char*>(contents), size * nmemb);
            return size * nmemb;
        });
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
        
        CURLcode res = curl_easy_perform(curl);
        long statusCode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);
        
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        
        return (res == CURLE_OK && statusCode >= 200 && statusCode < 300);
    } catch (const std::exception&) {
        return false;
    }
}

TokenInfo AuthService::getTokenInfo(const std::string& token) {
    TokenInfo info;
    info.accessToken = token;
    info.isValid = validateToken(token);
    
    if (info.isValid) {
        try {
            CURL* curl = curl_easy_init();
            if (!curl) {
                info.isValid = false;
                return info;
            }
            
            std::string responseBody;
            std::string authHeader = "Authorization: Bearer " + token;
            
            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, authHeader.c_str());
            
            curl_easy_setopt(curl, CURLOPT_URL, endpoints_.userProfileUrl.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](void* contents, size_t size, size_t nmemb, std::string* data) -> size_t {
                data->append(static_cast<char*>(contents), size * nmemb);
                return size * nmemb;
            });
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
            
            CURLcode res = curl_easy_perform(curl);
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            
            if (res == CURLE_OK) {
                auto json = nlohmann::json::parse(responseBody);
                if (json.contains("profile")) {
                    info.issuedAt = std::chrono::system_clock::now();
                    info.expiresAt = info.issuedAt + std::chrono::hours(24);
                }
            }
        } catch (const std::exception&) {
            info.isValid = false;
        }
    }
    
    return info;
}

bool AuthService::isTokenValid(const TokenInfo& tokenInfo) {
    return tokenInfo.isValid && !tokenInfo.isExpired() && !tokenInfo.accessToken.empty();
}

TokenInfo AuthService::autoRefreshIfNeeded(const TokenInfo& tokenInfo) {
    if (!config_.autoRefresh || tokenInfo.refreshToken.empty()) {
        return tokenInfo;
    }
    
    if (tokenInfo.isExpiringSoon(config_.refreshThreshold)) {
        try {
            return refreshAccessToken(tokenInfo.refreshToken);
        } catch (const std::exception& e) {
            if (errorCallback_) {
                errorCallback_("Auto-refresh failed: " + std::string(e.what()));
            }
            return tokenInfo;
        }
    }
    
    return tokenInfo;
}

bool AuthService::validateState(const std::string& state) {
    cleanupExpiredStates();
    
    auto it = stateCache_.find(state);
    if (it != stateCache_.end()) {
        auto now = std::chrono::system_clock::now();
        if (now <= it->second + config_.stateExpiration) {
            stateCache_.erase(it);
            return true;
        }
        stateCache_.erase(it);
    }
    
    return false;
}

void AuthService::clearState() {
    stateCache_.clear();
    currentState_.clear();
    codeVerifier_.clear();
    codeChallenge_.clear();
}

void AuthService::setConfig(const AuthConfig& config) {
    config_ = config;
}

AuthConfig AuthService::getConfig() const {
    return config_;
}

void AuthService::setTokenRefreshCallback(TokenRefreshCallback callback) {
    tokenRefreshCallback_ = std::move(callback);
}

void AuthService::setErrorCallback(AuthErrorCallback callback) {
    errorCallback_ = std::move(callback);
}

bool AuthService::isSandboxMode() const {
    return client_.config().sandboxMode;
}

AuthEndpoints AuthService::getEndpoints() const {
    return endpoints_;
}

std::vector<TokenScope> AuthService::parseScopeString(const std::string& scopeString) {
    std::vector<TokenScope> scopes;
    std::istringstream iss(scopeString);
    std::string scope;
    
    while (std::getline(iss, scope, ' ')) {
        if (scope == "read") {
            scopes.push_back(TokenScope::READ);
        } else if (scope == "write") {
            scopes.push_back(TokenScope::WRITE);
        } else if (scope == "market") {
            scopes.push_back(TokenScope::MARKET);
        } else if (scope == "trade") {
            scopes.push_back(TokenScope::TRADE);
        } else if (scope == "stream") {
            scopes.push_back(TokenScope::STREAM);
        }
    }
    
    return scopes;
}

std::string AuthService::scopesToString(const std::vector<TokenScope>& scopes) {
    std::vector<std::string> scopeStrings;
    
    for (const auto& scope : scopes) {
        scopeStrings.push_back(scopeToString(scope));
    }
    
    std::ostringstream oss;
    for (size_t i = 0; i < scopeStrings.size(); ++i) {
        if (i > 0) oss << " ";
        oss << scopeStrings[i];
    }
    
    return oss.str();
}

std::string AuthService::scopeToString(TokenScope scope) {
    switch (scope) {
        case TokenScope::READ: return "read";
        case TokenScope::WRITE: return "write";
        case TokenScope::MARKET: return "market";
        case TokenScope::TRADE: return "trade";
        case TokenScope::STREAM: return "stream";
        default: return "read";
    }
}

AuthConfig AuthService::createConfig(const std::string& clientId, const std::string& clientSecret, 
                                   const std::string& redirectUri) {
    AuthConfig config;
    config.clientId = clientId;
    config.clientSecret = clientSecret;
    config.redirectUri = redirectUri.empty() ? "http://localhost:8080/callback" : redirectUri;
    config.requestedScopes = {TokenScope::READ, TokenScope::WRITE, TokenScope::TRADE, TokenScope::MARKET};
    config.usePKCE = true;
    config.autoRefresh = true;
    config.stateExpiration = std::chrono::minutes(10);
    config.refreshThreshold = std::chrono::minutes(5);
    
    return config;
}

namespace auth {
    bool isValidRedirectUri(const std::string& uri) {
        if (uri.empty()) return false;
        
        std::regex uriRegex(R"(^https?://[a-zA-Z0-9.-]+(?:\:[0-9]+)?(?:/.*)?$)");
        return std::regex_match(uri, uriRegex);
    }
    
    std::string extractAuthCodeFromUrl(const std::string& redirectUrl) {
        std::regex codeRegex(R"([?&]code=([^&]+))");
        std::smatch matches;
        if (std::regex_search(redirectUrl, matches, codeRegex)) {
            return matches[1].str();
        }
        return "";
    }
    
    std::string extractStateFromUrl(const std::string& redirectUrl) {
        std::regex stateRegex(R"([?&]state=([^&]+))");
        std::smatch matches;
        if (std::regex_search(redirectUrl, matches, stateRegex)) {
            return matches[1].str();
        }
        return "";
    }
    
    std::string extractErrorFromUrl(const std::string& redirectUrl) {
        std::regex errorRegex(R"([?&]error=([^&]+))");
        std::smatch matches;
        if (std::regex_search(redirectUrl, matches, errorRegex)) {
            return matches[1].str();
        }
        return "";
    }
    
    bool saveTokenToFile(const TokenInfo& token, const std::string& filepath) {
        try {
            nlohmann::json j;
            j["access_token"] = token.accessToken;
            j["refresh_token"] = token.refreshToken;
            j["token_type"] = token.tokenType;
            j["expires_at"] = std::chrono::duration_cast<std::chrono::seconds>(
                token.expiresAt.time_since_epoch()).count();
            j["issued_at"] = std::chrono::duration_cast<std::chrono::seconds>(
                token.issuedAt.time_since_epoch()).count();
            j["is_valid"] = token.isValid;
            
            std::vector<std::string> scopeStrings;
            for (const auto& scope : token.scopes) {
                scopeStrings.push_back(AuthService::scopeToString(scope));
            }
            j["scopes"] = scopeStrings;
            
            std::ofstream file(filepath);
            if (!file.is_open()) {
                return false;
            }
            
            file << j.dump(4);
            return file.good();
            
        } catch (const std::exception&) {
            return false;
        }
    }
    
    TokenInfo loadTokenFromFile(const std::string& filepath) {
        TokenInfo token;
        
        try {
            std::ifstream file(filepath);
            if (!file.is_open()) {
                return token;
            }
            
            nlohmann::json j;
            file >> j;
            
            token.accessToken = j.value("access_token", "");
            token.refreshToken = j.value("refresh_token", "");
            token.tokenType = j.value("token_type", "Bearer");
            token.isValid = j.value("is_valid", false);
            
            if (j.contains("expires_at")) {
                long expiresAtSeconds = j["expires_at"];
                token.expiresAt = std::chrono::system_clock::from_time_t(expiresAtSeconds);
            }
            
            if (j.contains("issued_at")) {
                long issuedAtSeconds = j["issued_at"];
                token.issuedAt = std::chrono::system_clock::from_time_t(issuedAtSeconds);
            }
            
            if (j.contains("scopes") && j["scopes"].is_array()) {
                for (const auto& scopeStr : j["scopes"]) {
                    std::string scope = scopeStr;
                    if (scope == "read") token.scopes.push_back(TokenScope::READ);
                    else if (scope == "write") token.scopes.push_back(TokenScope::WRITE);
                    else if (scope == "market") token.scopes.push_back(TokenScope::MARKET);
                    else if (scope == "trade") token.scopes.push_back(TokenScope::TRADE);
                    else if (scope == "stream") token.scopes.push_back(TokenScope::STREAM);
                }
            }
            
        } catch (const std::exception&) {
            token = TokenInfo{};
        }
        
        return token;
    }
    
    bool deleteTokenFile(const std::string& filepath) {
        try {
            return std::remove(filepath.c_str()) == 0;
        } catch (const std::exception&) {
            return false;
        }
    }
}

}
