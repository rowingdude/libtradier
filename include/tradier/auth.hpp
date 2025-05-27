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

#pragma once

#include <string>
#include <chrono>
#include <map>
#include <functional>
#include "tradier/common/types.hpp"

namespace tradier {

class TradierClient;

enum class TokenType {
    ACCESS_TOKEN,
    REFRESH_TOKEN
};

enum class TokenScope {
    READ,
    WRITE,
    MARKET,
    TRADE,
    STREAM
};

struct TokenInfo {
    std::string accessToken;
    std::string refreshToken;
    std::string tokenType = "Bearer";
    std::chrono::system_clock::time_point expiresAt;
    std::chrono::system_clock::time_point issuedAt;
    std::vector<TokenScope> scopes;
    bool isValid = false;
    
    bool hasScope(TokenScope scope) const;
    bool isExpired() const;
    bool isExpiringSoon(std::chrono::minutes threshold = std::chrono::minutes(5)) const;
    std::string getScopeString() const;
    long getSecondsUntilExpiry() const;
};

struct AuthEndpoints {
    std::string authorizationUrl;
    std::string accessTokenUrl; 
    std::string refreshTokenUrl;
    std::string revokeTokenUrl;
    std::string userProfileUrl;
    
    static AuthEndpoints forEnvironment(bool sandbox);
};

struct AuthConfig {
    std::string clientId;
    std::string clientSecret;
    std::string redirectUri;
    std::vector<TokenScope> requestedScopes;
    bool usePKCE = true;
    std::chrono::seconds stateExpiration = std::chrono::minutes(10);
    bool autoRefresh = true;
    std::chrono::minutes refreshThreshold = std::chrono::minutes(5);
};

using TokenRefreshCallback = std::function<void(const TokenInfo&)>;
using AuthErrorCallback = std::function<void(const std::string&)>;

class AuthService {
private:
    TradierClient& client_;
    AuthConfig config_;
    AuthEndpoints endpoints_;
    
    std::map<std::string, std::chrono::system_clock::time_point> stateCache_;
    std::string currentState_;
    std::string codeVerifier_;
    std::string codeChallenge_;
    
    TokenRefreshCallback tokenRefreshCallback_;
    AuthErrorCallback errorCallback_;
    
    std::string generateRandomString(size_t length = 32) const;
    std::string generateCodeVerifier() const;
    std::string generateCodeChallenge(const std::string& verifier) const;
    std::string base64UrlEncode(const std::string& input) const;
    std::string sha256(const std::string& input) const;
    void cleanupExpiredStates();
    
public:
    explicit AuthService(TradierClient& client, const AuthConfig& config = {});
    ~AuthService() = default;

    std::string getAuthorizationUrl();
    std::string getAuthorizationUrl(const std::vector<TokenScope>& scopes);
    
    TokenInfo exchangeAuthorizationCode(const std::string& authCode, const std::string& state = "");
    TokenInfo refreshAccessToken(const std::string& refreshToken);
    bool revokeToken(const std::string& token, TokenType type = TokenType::ACCESS_TOKEN);

    bool validateToken(const std::string& token);
    TokenInfo getTokenInfo(const std::string& token);
    bool isTokenValid(const TokenInfo& tokenInfo);
    TokenInfo autoRefreshIfNeeded(const TokenInfo& tokenInfo);

    bool validateState(const std::string& state);
    void clearState();

    void setConfig(const AuthConfig& config);
    AuthConfig getConfig() const;
    void setTokenRefreshCallback(TokenRefreshCallback callback);
    void setErrorCallback(AuthErrorCallback callback);

    static std::vector<TokenScope> parseScopeString(const std::string& scopeString);
    static std::string scopesToString(const std::vector<TokenScope>& scopes);
    static std::string scopeToString(TokenScope scope);
    static AuthConfig createConfig(const std::string& clientId, const std::string& clientSecret, 
                                  const std::string& redirectUri = "");

    bool isSandboxMode() const;
    AuthEndpoints getEndpoints() const;
};

namespace auth {
    bool isValidRedirectUri(const std::string& uri);
    std::string extractAuthCodeFromUrl(const std::string& redirectUrl);
    std::string extractStateFromUrl(const std::string& redirectUrl);
    std::string extractErrorFromUrl(const std::string& redirectUrl);

    bool saveTokenToFile(const TokenInfo& token, const std::string& filepath);
    TokenInfo loadTokenFromFile(const std::string& filepath);
    bool deleteTokenFile(const std::string& filepath);
}

}