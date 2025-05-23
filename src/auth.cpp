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
#include <curl/curl.h>

namespace tradier {


bool TokenInfo::hasScope(TokenScope scope) const {
    return std::find(scopes.begin(), scopes.end(), scope) != scopes.end();
}

bool TokenInfo::isExpired() const {
    return std::chrono::system_clock::now() >= expiresAt;
}

bool TokenInfo::isExpiringSoon(std::chrono::minutes threshold) const {
    auto now = std::chrono::system_clock::now();
    return (now + threshold) >= expiresAt;
}

std::string TokenInfo::getScopeString() const {
    return AuthService::scopesToString(scopes);
}

long TokenInfo::getSecondsUntilExpiry() const {
    auto now = std::chrono::system_clock::now();
    if (now >= expiresAt) return 0;
    return std::chrono::duration_cast<std::chrono::seconds>(expiresAt - now).count();
}


AuthEndpoints AuthEndpoints::forEnvironment(bool sandbox) {
    AuthEndpoints endpoints;
    
    if (sandbox) {
        endpoints.authorizationUrl = "https://sandbox.tradier.com/v1/oauth/authorize";
        endpoints.accessTokenUrl = "https://sandbox.tradier.com/v1/oauth/accesstoken";
        endpoints.refreshTokenUrl = "https://sandbox.tradier.com/v1/oauth/refreshtoken";
        endpoints.revokeTokenUrl = "https://sandbox.tradier.com/v1/oauth/revoke";
        endpoints.userProfileUrl = "https://sandbox.tradier.com/v1/user/profile";
    } else {
        endpoints.authorizationUrl = "https://api.tradier.com/v1/oauth/authorize";
        endpoints.accessTokenUrl = "https://api.tradier.com/v1/oauth/accesstoken";
        endpoints.refreshTokenUrl = "https://api.tradier.com/v1/oauth/refreshtoken";
        endpoints.revokeTokenUrl = "https://api.tradier.com/v1/oauth/revoke";
        endpoints.userProfileUrl = "https://api.tradier.com/v1/user/profile";
    }
    
    return endpoints;
}


AuthService::AuthService(TradierClient& client, const AuthConfig& config) 
    : client_(client), config_(config), endpoints_(AuthEndpoints::forEnvironment(client.config().sandboxMode)) {}

std::string AuthService::getAuthorizationUrl() {
    return getAuthorizationUrl(config_.requestedScopes);
}

std::string AuthService::getAuthorizationUrl(const std::vector<TokenScope>& scopes) {
    cleanupExpiredStates();
    
    currentState_ = generateRandomString();
    stateCache_[currentState_] = std::chrono::system_clock::now();
    
    std::ostringstream url;
    url << endpoints_.authorizationUrl;
    url << "?response_type=code";
    url << "&client_id=" << utils::urlEncode(config_.clientId);
    url << "&state=" << utils::urlEncode(currentState_);
    
    if (!config_.redirectUri.empty()) {
        url << "&redirect_uri=" << utils::urlEncode(config_.redirectUri);
    }
    
    if (!scopes.empty()) {
        url << "&scope=" << utils::urlEncode(scopesToString(scopes));
    }
    
    if (config_.usePKCE) {
        codeVerifier_ = generateCodeVerifier();
        codeChallenge_ = generateCodeChallenge(codeVerifier_);
        url << "&code_challenge=" << utils::urlEncode(codeChallenge_);
        url << "&code_challenge_method=S256";
    }
    
    return url.str();
}

TokenInfo AuthService::exchangeAuthorizationCode(const std::string& authCode, const std::string& state) {
    if (!state.empty() && !validateState(state)) {
        throw AuthenticationError("Invalid or expired state parameter");
    }
    
    if (authCode.empty()) {
        throw ValidationError("Authorization code cannot be empty");
    }
    
    FormParams params;
    params["grant_type"] = "authorization_code";
    params["code"] = authCode;
    params["client_id"] = config_.clientId;
    params["client_secret"] = config_.clientSecret;
    
    if (!config_.redirectUri.empty()) {
        params["redirect_uri"] = config_.redirectUri;
    }
    
    if (config_.usePKCE && !codeVerifier_.empty()) {
        params["code_verifier"] = codeVerifier_;
    }

    Response response = client_.post(endpoints_.accessTokenUrl, params);
    
    if (!response.success()) {
        throw AuthenticationError("Failed to exchange authorization code: HTTP " + 
                                std::to_string(response.status));
    }
    
    try {
        auto json = nlohmann::json::parse(response.body);
        
        TokenInfo token;
        token.accessToken = json.value("access_token", "");
        token.refreshToken = json.value("refresh_token", "");
        token.tokenType = json.value("token_type", "Bearer");
        token.issuedAt = std::chrono::system_clock::now();
        
        if (json.contains("expires_in")) {
            int expiresIn = json["expires_in"];
            token.expiresAt = token.issuedAt + std::chrono::seconds(expiresIn);
        }
        
        if (json.contains("scope")) {
            token.scopes = parseScopeString(json["scope"]);
        }
        
        token.isValid = !token.accessToken.empty();

        codeVerifier_.clear();
        codeChallenge_.clear();
        
        return token;
        
    } catch (const nlohmann::json::exception& e) {
        throw AuthenticationError("Invalid token response format: " + std::string(e.what()));
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
    
    Response response = client_.post(endpoints_.refreshTokenUrl, params);
    
    if (!response.success()) {
        throw AuthenticationError("Failed to refresh token: HTTP " + 
                                std::to_string(response.status));
    }
    
    try {
        auto json = nlohmann::json::parse(response.body);
        
        TokenInfo token;
        token.accessToken = json.value("access_token", "");
        token.refreshToken = json.value("refresh_token", refreshToken); // Keep old refresh token if not provided
        token.tokenType = json.value("token_type", "Bearer");
        token.issuedAt = std::chrono::system_clock::now();
        
        if (json.contains("expires_in")) {
            int expiresIn = json["expires_in"];
            token.expiresAt = token.issuedAt + std::chrono::seconds(expiresIn);
        }
        
        if (json.contains("scope")) {
            token.scopes = parseScopeString(json["scope"]);
        }
        
        token.isValid = !token.accessToken.empty();
        
        if (tokenRefreshCallback_) {
            tokenRefreshCallback_(token);
        }
        
        return token;
        
    } catch (const nlohmann::json::exception& e) {
        throw AuthenticationError("Invalid refresh response format: " + std::string(e.what()));
    }
}

bool AuthService::validateToken(const std::string& token) {
    if (token.empty()) return false;
    
    try {

        std::string currentToken = client_.config().accessToken;

        Config tempConfig = client_.config();
        tempConfig.accessToken = token;
        TradierClient tempClient(tempConfig);
        
        auto response = tempClient.get("/user/profile");
        return response.success();
        
    } catch (const std::exception&) {
        return false;
    }
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
        }
    }
    
    return tokenInfo;
}


std::string AuthService::scopeToString(TokenScope scope) {
    switch (scope) {
        case TokenScope::READ: return "read";
        case TokenScope::WRITE: return "write";
        case TokenScope::MARKET: return "market";
        case TokenScope::TRADE: return "trade";
        case TokenScope::STREAM: return "stream";
        default: return "";
    }
}

std::string AuthService::scopesToString(const std::vector<TokenScope>& scopes) {
    std::ostringstream ss;
    for (size_t i = 0; i < scopes.size(); ++i) {
        if (i > 0) ss << ",";
        ss << scopeToString(scopes[i]);
    }
    return ss.str();
}

std::vector<TokenScope> AuthService::parseScopeString(const std::string& scopeString) {
    std::vector<TokenScope> scopes;
    std::istringstream ss(scopeString);
    std::string scope;
    
    while (std::getline(ss, scope, ',')) {
        if (scope == "read") scopes.push_back(TokenScope::READ);
        else if (scope == "write") scopes.push_back(TokenScope::WRITE);
        else if (scope == "market") scopes.push_back(TokenScope::MARKET);
        else if (scope == "trade") scopes.push_back(TokenScope::TRADE);
        else if (scope == "stream") scopes.push_back(TokenScope::STREAM);
    }
    
    return scopes;
}

AuthConfig AuthService::createConfig(const std::string& clientId, const std::string& clientSecret, const std::string& redirectUri) {
    AuthConfig config;
    config.clientId = clientId;
    config.clientSecret = clientSecret;
    config.redirectUri = redirectUri;
    config.requestedScopes = {TokenScope::READ, TokenScope::WRITE, TokenScope::MARKET, TokenScope::TRADE};
    return config;
}

std::string AuthService::generateRandomString(size_t length) const {
    static const char chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(chars) - 2);
    
    std::string result;
    result.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        result += chars[dis(gen)];
    }
    return result;
}

std::string AuthService::generateCodeVerifier() const {
    return generateRandomString(128);
}

std::string AuthService::generateCodeChallenge(const std::string& verifier) const {
    return base64UrlEncode(sha256(verifier));
}

bool AuthService::validateState(const std::string& state) {
    cleanupExpiredStates();
    auto it = stateCache_.find(state);
    if (it != stateCache_.end()) {
        stateCache_.erase(it);
        return true;
    }
    return false;
}

void AuthService::cleanupExpiredStates() {
    auto now = std::chrono::system_clock::now();
    auto it = stateCache_.begin();
    while (it != stateCache_.end()) {
        if ((now - it->second) > config_.stateExpiration) {
            it = stateCache_.erase(it);
        } else {
            ++it;
        }
    }
}


namespace auth {
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
            j["scopes"] = AuthService::scopesToString(token.scopes);
            j["is_valid"] = token.isValid;
            
            std::ofstream file(filepath);
            file << j.dump(2);
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }
    
    TokenInfo loadTokenFromFile(const std::string& filepath) {
        TokenInfo token;
        try {
            std::ifstream file(filepath);
            nlohmann::json j;
            file >> j;
            
            token.accessToken = j.value("access_token", "");
            token.refreshToken = j.value("refresh_token", "");
            token.tokenType = j.value("token_type", "Bearer");
            token.isValid = j.value("is_valid", false);
            
            if (j.contains("expires_at")) {
                token.expiresAt = std::chrono::system_clock::from_time_t(j["expires_at"]);
            }
            if (j.contains("issued_at")) {
                token.issuedAt = std::chrono::system_clock::from_time_t(j["issued_at"]);
            }
            if (j.contains("scopes")) {
                token.scopes = AuthService::parseScopeString(j["scopes"]);
            }
            
        } catch (const std::exception&) {
            token.isValid = false;
        }
        return token;
    }
}

}