#include "test_framework.hpp"
#include "tradier/auth.hpp"
#include "tradier/client.hpp"
#include "tradier/common/errors.hpp"
#include <chrono>

namespace {

void test_token_scope() {
    using tradier::TokenScope;
    using tradier::AuthService;
    
    ASSERT_EQ("read", AuthService::scopeToString(TokenScope::READ));
    ASSERT_EQ("write", AuthService::scopeToString(TokenScope::WRITE));
    ASSERT_EQ("market", AuthService::scopeToString(TokenScope::MARKET));
    ASSERT_EQ("trade", AuthService::scopeToString(TokenScope::TRADE));
    ASSERT_EQ("stream", AuthService::scopeToString(TokenScope::STREAM));
    
    std::vector<TokenScope> scopes = {TokenScope::READ, TokenScope::WRITE, TokenScope::TRADE};
    std::string scopeString = AuthService::scopesToString(scopes);
    ASSERT_EQ("read write trade", scopeString);
    
    auto parsedScopes = AuthService::parseScopeString("read write market");
    ASSERT_EQ(3, parsedScopes.size());
    ASSERT_EQ(TokenScope::READ, parsedScopes[0]);
    ASSERT_EQ(TokenScope::WRITE, parsedScopes[1]);
    ASSERT_EQ(TokenScope::MARKET, parsedScopes[2]);
}

void test_token_info() {
    tradier::TokenInfo token;
    token.accessToken = "test_access_token";
    token.tokenType = "Bearer";
    token.isValid = true;
    token.scopes = {tradier::TokenScope::READ, tradier::TokenScope::TRADE};
    
    auto now = std::chrono::system_clock::now();
    token.issuedAt = now;
    token.expiresAt = now + std::chrono::hours(1);
    
    ASSERT_TRUE(token.hasScope(tradier::TokenScope::READ));
    ASSERT_TRUE(token.hasScope(tradier::TokenScope::TRADE));
    ASSERT_FALSE(token.hasScope(tradier::TokenScope::STREAM));
    
    ASSERT_FALSE(token.isExpired());
    ASSERT_FALSE(token.isExpiringSoon(std::chrono::minutes(30)));
    ASSERT_TRUE(token.isExpiringSoon(std::chrono::minutes(90)));
    
    ASSERT_EQ("read trade", token.getScopeString());
    ASSERT_TRUE(token.getSecondsUntilExpiry() > 3500);
    ASSERT_TRUE(token.getSecondsUntilExpiry() < 3700);
    
    token.expiresAt = now - std::chrono::hours(1);
    ASSERT_TRUE(token.isExpired());
    ASSERT_EQ(0, token.getSecondsUntilExpiry());
}

void test_auth_endpoints() {
    auto sandboxEndpoints = tradier::AuthEndpoints::forEnvironment(true);
    ASSERT_EQ("https://sandbox.tradier.com/oauth/authorize", sandboxEndpoints.authorizationUrl);
    ASSERT_EQ("https://sandbox.tradier.com/oauth/accesstoken", sandboxEndpoints.accessTokenUrl);
    ASSERT_EQ("https://sandbox.tradier.com/oauth/accesstoken", sandboxEndpoints.refreshTokenUrl);
    ASSERT_EQ("https://sandbox.tradier.com/oauth/revoke", sandboxEndpoints.revokeTokenUrl);
    ASSERT_EQ("https://sandbox.tradier.com/v1/user/profile", sandboxEndpoints.userProfileUrl);
    
    auto prodEndpoints = tradier::AuthEndpoints::forEnvironment(false);
    ASSERT_EQ("https://api.tradier.com/oauth/authorize", prodEndpoints.authorizationUrl);
    ASSERT_EQ("https://api.tradier.com/oauth/accesstoken", prodEndpoints.accessTokenUrl);
    ASSERT_EQ("https://api.tradier.com/oauth/accesstoken", prodEndpoints.refreshTokenUrl);
    ASSERT_EQ("https://api.tradier.com/oauth/revoke", prodEndpoints.revokeTokenUrl);
    ASSERT_EQ("https://api.tradier.com/v1/user/profile", prodEndpoints.userProfileUrl);
}

void test_auth_config() {
    auto config = tradier::AuthService::createConfig("client123", "secret456", "https://example.com/callback");
    
    ASSERT_EQ("client123", config.clientId);
    ASSERT_EQ("secret456", config.clientSecret);
    ASSERT_EQ("https://example.com/callback", config.redirectUri);
    ASSERT_TRUE(config.usePKCE);
    ASSERT_TRUE(config.autoRefresh);
    ASSERT_EQ(4, config.requestedScopes.size());
    
    auto defaultConfig = tradier::AuthService::createConfig("client123", "secret456");
    ASSERT_EQ("http://localhost:8080/callback", defaultConfig.redirectUri);
}

void test_auth_service_creation() {
    tradier::Config clientConfig;
    clientConfig.accessToken = "test_token";
    clientConfig.sandboxMode = true;
    
    tradier::TradierClient client(clientConfig);
    
    tradier::AuthConfig authConfig;
    authConfig.clientId = "test_client";
    authConfig.clientSecret = "test_secret";
    authConfig.redirectUri = "https://example.com/callback";
    
    tradier::AuthService auth(client, authConfig);
    
    ASSERT_TRUE(auth.isSandboxMode());
    ASSERT_EQ(authConfig.clientId, auth.getConfig().clientId);
    ASSERT_EQ(authConfig.clientSecret, auth.getConfig().clientSecret);
    ASSERT_EQ(authConfig.redirectUri, auth.getConfig().redirectUri);
}

void test_authorization_url_generation() {
    tradier::Config clientConfig;
    clientConfig.accessToken = "test_token";
    clientConfig.sandboxMode = true;
    
    tradier::TradierClient client(clientConfig);
    
    tradier::AuthConfig authConfig;
    authConfig.clientId = "test_client";
    authConfig.clientSecret = "test_secret";
    authConfig.redirectUri = "https://example.com/callback";
    authConfig.requestedScopes = {tradier::TokenScope::READ, tradier::TokenScope::TRADE};
    
    tradier::AuthService auth(client, authConfig);
    
    std::string authUrl = auth.getAuthorizationUrl();
    
    ASSERT_FALSE(authUrl.empty());
    ASSERT_TRUE(authUrl.find("sandbox.tradier.com") != std::string::npos);
    ASSERT_TRUE(authUrl.find("client_id=test_client") != std::string::npos);
    ASSERT_TRUE(authUrl.find("redirect_uri=") != std::string::npos);
    ASSERT_TRUE(authUrl.find("scope=") != std::string::npos);
    ASSERT_TRUE(authUrl.find("state=") != std::string::npos);
    ASSERT_TRUE(authUrl.find("code_challenge=") != std::string::npos);
    ASSERT_TRUE(authUrl.find("code_challenge_method=S256") != std::string::npos);
}

void test_exchange_auth_code_validation() {
    tradier::Config clientConfig;
    clientConfig.accessToken = "test_token";
    clientConfig.sandboxMode = true;
    
    tradier::TradierClient client(clientConfig);
    
    tradier::AuthConfig authConfig;
    authConfig.clientId = "test_client";
    authConfig.clientSecret = "test_secret";
    authConfig.redirectUri = "https://example.com/callback";
    
    tradier::AuthService auth(client, authConfig);
    
    ASSERT_THROW(auth.exchangeAuthorizationCode(""), tradier::ValidationError);
}

void test_refresh_token_validation() {
    tradier::Config clientConfig;
    clientConfig.accessToken = "test_token";
    clientConfig.sandboxMode = true;
    
    tradier::TradierClient client(clientConfig);
    
    tradier::AuthConfig authConfig;
    authConfig.clientId = "test_client";
    authConfig.clientSecret = "test_secret";
    authConfig.redirectUri = "https://example.com/callback";
    
    tradier::AuthService auth(client, authConfig);
    
    ASSERT_THROW(auth.refreshAccessToken(""), tradier::ValidationError);
}

void test_revoke_token_validation() {
    tradier::Config clientConfig;
    clientConfig.accessToken = "test_token";
    clientConfig.sandboxMode = true;
    
    tradier::TradierClient client(clientConfig);
    
    tradier::AuthConfig authConfig;
    authConfig.clientId = "test_client";
    authConfig.clientSecret = "test_secret";
    authConfig.redirectUri = "https://example.com/callback";
    
    tradier::AuthService auth(client, authConfig);
    
    ASSERT_THROW(auth.revokeToken(""), tradier::ValidationError);
}

void test_auth_callbacks() {
    tradier::Config clientConfig;
    clientConfig.accessToken = "test_token";
    clientConfig.sandboxMode = true;
    
    tradier::TradierClient client(clientConfig);
    
    tradier::AuthConfig authConfig;
    authConfig.clientId = "test_client";
    authConfig.clientSecret = "test_secret";
    authConfig.redirectUri = "https://example.com/callback";
    
    tradier::AuthService auth(client, authConfig);
    
    bool tokenRefreshCalled = false;
    bool errorCalled = false;
    
    auth.setTokenRefreshCallback([&](const tradier::TokenInfo& token) {
        tokenRefreshCalled = true;
    });
    
    auth.setErrorCallback([&](const std::string& error) {
        errorCalled = true;
    });
    
    ASSERT_FALSE(tokenRefreshCalled);
    ASSERT_FALSE(errorCalled);
}

void test_auth_helper_functions() {
    ASSERT_TRUE(tradier::auth::isValidRedirectUri("https://example.com/callback"));
    ASSERT_TRUE(tradier::auth::isValidRedirectUri("http://localhost:8080/auth"));
    ASSERT_FALSE(tradier::auth::isValidRedirectUri(""));
    ASSERT_FALSE(tradier::auth::isValidRedirectUri("not-a-url"));
    
    std::string redirectUrl = "https://example.com/callback?code=auth123&state=state456&error=access_denied";
    
    ASSERT_EQ("auth123", tradier::auth::extractAuthCodeFromUrl(redirectUrl));
    ASSERT_EQ("state456", tradier::auth::extractStateFromUrl(redirectUrl));
    ASSERT_EQ("access_denied", tradier::auth::extractErrorFromUrl(redirectUrl));
    
    std::string noCodeUrl = "https://example.com/callback?state=state456";
    ASSERT_EQ("", tradier::auth::extractAuthCodeFromUrl(noCodeUrl));
}

void test_token_file_operations() {
    tradier::TokenInfo token;
    token.accessToken = "test_access_token";
    token.refreshToken = "test_refresh_token";
    token.tokenType = "Bearer";
    token.isValid = true;
    token.scopes = {tradier::TokenScope::READ, tradier::TokenScope::TRADE};
    
    auto now = std::chrono::system_clock::now();
    token.issuedAt = now;
    token.expiresAt = now + std::chrono::hours(1);
    
    std::string testFile = "/tmp/test_token.json";
    
    ASSERT_TRUE(tradier::auth::saveTokenToFile(token, testFile));
    
    auto loadedToken = tradier::auth::loadTokenFromFile(testFile);
    ASSERT_EQ(token.accessToken, loadedToken.accessToken);
    ASSERT_EQ(token.refreshToken, loadedToken.refreshToken);
    ASSERT_EQ(token.tokenType, loadedToken.tokenType);
    ASSERT_EQ(token.isValid, loadedToken.isValid);
    ASSERT_EQ(token.scopes.size(), loadedToken.scopes.size());
    
    ASSERT_TRUE(tradier::auth::deleteTokenFile(testFile));
    
    auto emptyToken = tradier::auth::loadTokenFromFile("/nonexistent/file.json");
    ASSERT_TRUE(emptyToken.accessToken.empty());
}

}

int test_auth_main() {
    test::TestSuite suite("Authentication Tests");
    
    suite.add_test("Token Scope", test_token_scope);
    suite.add_test("Token Info", test_token_info);
    suite.add_test("Auth Endpoints", test_auth_endpoints);
    suite.add_test("Auth Config", test_auth_config);
    suite.add_test("Auth Service Creation", test_auth_service_creation);
    suite.add_test("Authorization URL Generation", test_authorization_url_generation);
    suite.add_test("Exchange Auth Code Validation", test_exchange_auth_code_validation);
    suite.add_test("Refresh Token Validation", test_refresh_token_validation);
    suite.add_test("Revoke Token Validation", test_revoke_token_validation);
    suite.add_test("Auth Callbacks", test_auth_callbacks);
    suite.add_test("Auth Helper Functions", test_auth_helper_functions);
    suite.add_test("Token File Operations", test_token_file_operations);
    
    return suite.run();
}