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
#include <memory>

namespace tradier {

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

class FileGuard {
private:
    std::string filepath_;
    bool shouldDelete_;

public:
    explicit FileGuard(std::string filepath, bool autoDelete = false) 
        : filepath_(std::move(filepath)), shouldDelete_(autoDelete) {}
    
    ~FileGuard() {
        if (shouldDelete_) {
            std::remove(filepath_.c_str());
        }
    }
    
    FileGuard(const FileGuard&) = delete;
    FileGuard& operator=(const FileGuard&) = delete;
    
    FileGuard(FileGuard&& other) noexcept 
        : filepath_(std::move(other.filepath_)), shouldDelete_(other.shouldDelete_) {
        other.shouldDelete_ = false;
    }
    
    FileGuard& operator=(FileGuard&& other) noexcept {
        if (this != &other) {
            if (shouldDelete_) {
                std::remove(filepath_.c_str());
            }
            filepath_ = std::move(other.filepath_);
            shouldDelete_ = other.shouldDelete_;
            other.shouldDelete_ = false;
        }
        return *this;
    }
    
    const std::string& path() const { return filepath_; }
    void setAutoDelete(bool autoDelete) { shouldDelete_ = autoDelete; }
};

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
    std::string base64 = base64Encode(input);
    
    std::replace(base64.begin(), base64.end(), '+', '-');
    std::replace(base64.begin(), base64.end(), '/', '_');
    
    base64.erase(std::remove(base64.begin(), base64.end(), '='), base64.end());
    
    return base64;
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
        Response response = client_.post(endpoints_.revokeTokenUrl, params);
        return response.success();
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
            std::string currentToken = client_.config().accessToken;
            Config tempConfig = client_.config();
            tempConfig.accessToken = token;
            TradierClient tempClient(tempConfig);
            
            auto response = tempClient.get(endpoints_.userProfileUrl);
            if (response.success()) {
                auto json = nlohmann::json::parse(response.body);
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

bool AuthService::isTokenValid(const TokenInfo& tokenInfo) {
    return tokenInfo.isValid && !tokenInfo.isExpired() && !tokenInfo.accessToken.empty();
}

void AuthService::clearState() {
    stateCache_.clear();
    currentState_.clear();
    codeVerifier_.clear();
    codeChallenge_.clear();
}

bool AuthService::isSandboxMode() const {
    return client_.config().sandboxMode;
}

AuthEndpoints AuthService::getEndpoints() const {
    return endpoints_;
}

namespace auth {
    bool isValidRedirectUri(const std::string& uri) {
        if (uri.empty()) return false;
        
        std::regex uriRegex(R"(^https?://[a-zA-Z0-9.-]+(?:\:[0-9]+)?(?:/.*)?$)");
        return std::regex_match(uri, uriRegex);
    }
    
    std::string extractErrorFromUrl(const std::string& redirectUrl) {
        std::regex errorRegex(R"([?&]error=([^&]+))");
        std::smatch matches;
        if (std::regex_search(redirectUrl, matches, errorRegex)) {
            return matches[1].str();
        }
        return "";
    }
    
    bool deleteTokenFile(const std::string& filepath) {
        try {
            FileGuard guard(filepath, true);
            return std::remove(filepath.c_str()) == 0;
        } catch (const std::exception&) {
            return false;
        }
    }
}