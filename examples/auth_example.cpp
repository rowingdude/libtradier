/*
 * Authentication Example for libtradier
 * 
 * This example demonstrates the complete OAuth2 authentication flow
 * with PKCE (Proof Key for Code Exchange) for enhanced security.
 */

#include <tradier/client.hpp>
#include <tradier/auth.hpp>
#include <tradier/common/config.hpp>
#include <tradier/common/errors.hpp>
#include <iostream>
#include <string>

using namespace tradier;

void printTokenInfo(const TokenInfo& token) {
    std::cout << "\n=== Token Information ===" << std::endl;
    std::cout << "Access Token: " << token.accessToken.substr(0, 20) << "..." << std::endl;
    std::cout << "Token Type: " << token.tokenType << std::endl;
    std::cout << "Valid: " << (token.isValid ? "Yes" : "No") << std::endl;
    std::cout << "Expires in: " << token.getSecondsUntilExpiry() << " seconds" << std::endl;
    std::cout << "Scopes: " << token.getScopeString() << std::endl;
    
    if (!token.refreshToken.empty()) {
        std::cout << "Refresh Token: " << token.refreshToken.substr(0, 20) << "..." << std::endl;
    }
    std::cout << "=========================" << std::endl;
}

int main() {
    try {
        // Load configuration from environment
        Config config = Config::fromEnvironment();
        
        // Check if we already have a token
        if (!config.accessToken.empty()) {
            std::cout << "Using existing token from environment..." << std::endl;
            
            TradierClient client(config);
            AuthService auth(client);
            
            TokenInfo existingToken = auth.getTokenInfo(config.accessToken);
            printTokenInfo(existingToken);
            
            // Test if token works
            if (auth.validateToken(config.accessToken)) {
                std::cout << "✓ Token is valid and working!" << std::endl;
                return 0;
            } else {
                std::cout << "✗ Token is invalid, need to re-authenticate" << std::endl;
            }
        }
        
        // Interactive OAuth2 flow
        std::cout << "\n=== OAuth2 Authentication Flow ===" << std::endl;
        
        std::string clientId, clientSecret, redirectUri;
        
        std::cout << "Enter your Tradier Client ID: ";
        std::getline(std::cin, clientId);
        
        std::cout << "Enter your Tradier Client Secret: ";
        std::getline(std::cin, clientSecret);
        
        std::cout << "Enter redirect URI (or press Enter for default): ";
        std::getline(std::cin, redirectUri);
        
        if (redirectUri.empty()) {
            redirectUri = "http://localhost:8080/callback";
        }
        
        // Create auth configuration
        AuthConfig authConfig = AuthService::createConfig(clientId, clientSecret, redirectUri);
        authConfig.requestedScopes = {
            TokenScope::READ, 
            TokenScope::WRITE, 
            TokenScope::TRADE, 
            TokenScope::MARKET,
            TokenScope::STREAM
        };
        
        // Initialize client and auth service
        config.sandboxMode = true; // Use sandbox for demo
        TradierClient client(config);
        AuthService auth(client, authConfig);
        
        // Set up callbacks
        auth.setTokenRefreshCallback([](const TokenInfo& token) {
            std::cout << "\n🔄 Token refreshed automatically!" << std::endl;
            printTokenInfo(token);
        });
        
        auth.setErrorCallback([](const std::string& error) {
            std::cerr << "\n❌ Auth Error: " << error << std::endl;
        });
        
        // Generate authorization URL
        std::string authUrl = auth.getAuthorizationUrl();
        
        std::cout << "\n📋 Step 1: Open this URL in your browser:" << std::endl;
        std::cout << authUrl << std::endl;
        
        std::cout << "\n📋 Step 2: After authorization, you'll be redirected to:" << std::endl;
        std::cout << redirectUri << "?code=AUTHORIZATION_CODE&state=STATE" << std::endl;
        
        std::string authCode, state;
        std::cout << "\n📋 Step 3: Copy the authorization code from the redirect URL:" << std::endl;
        std::cout << "Authorization Code: ";
        std::getline(std::cin, authCode);
        
        std::cout << "State (optional): ";
        std::getline(std::cin, state);
        
        // Exchange authorization code for tokens
        std::cout << "\n🔄 Exchanging authorization code for tokens..." << std::endl;
        
        TokenInfo tokenInfo = auth.exchangeAuthorizationCode(authCode, state);
        
        std::cout << "\n✅ Authentication successful!" << std::endl;
        printTokenInfo(tokenInfo);
        
        // Save token to file
        if (auth::saveTokenToFile(tokenInfo, "tradier_token.json")) {
            std::cout << "\n💾 Token saved to tradier_token.json" << std::endl;
        }
        
        // Test the token by making an API call
        std::cout << "\n🧪 Testing token with API call..." << std::endl;
        
        Config newConfig = config;
        newConfig.accessToken = tokenInfo.accessToken;
        TradierClient testClient(newConfig);
        
        auto response = testClient.get("/user/profile");
        if (response.success()) {
            std::cout << "✅ API test successful!" << std::endl;
            std::cout << "Profile response: " << response.body.substr(0, 200) << "..." << std::endl;
        } else {
            std::cout << "❌ API test failed: " << response.status << std::endl;
        }
        
        // Demonstrate token refresh (if refresh token available)
        if (!tokenInfo.refreshToken.empty()) {
            std::cout << "\n🔄 Testing token refresh..." << std::endl;
            
            try {
                TokenInfo refreshedToken = auth.refreshAccessToken(tokenInfo.refreshToken);
                std::cout << "✅ Token refresh successful!" << std::endl;
                printTokenInfo(refreshedToken);
            } catch (const std::exception& e) {
                std::cout << "❌ Token refresh failed: " << e.what() << std::endl;
            }
        }
        
        // Demonstrate loading token from file
        std::cout << "\n📂 Testing token loading from file..." << std::endl;
        TokenInfo loadedToken = auth::loadTokenFromFile("tradier_token.json");
        
        if (loadedToken.isValid && !loadedToken.accessToken.empty()) {
            std::cout << "✅ Token loaded successfully from file!" << std::endl;
            printTokenInfo(loadedToken);
        } else {
            std::cout << "❌ Failed to load token from file" << std::endl;
        }
        
        std::cout << "\n🎉 Authentication example completed successfully!" << std::endl;
        std::cout << "\nNext steps:" << std::endl;
        std::cout << "1. Set TRADIER_SBX_TOKEN environment variable to: " << tokenInfo.accessToken << std::endl;
        std::cout << "2. Run other examples to test trading, market data, and streaming" << std::endl;
        std::cout << "3. In production, use secure token storage and automatic refresh" << std::endl;
        
    } catch (const AuthenticationError& e) {
        std::cerr << "❌ Authentication Error: " << e.what() << std::endl;
        return 1;
    } catch (const ValidationError& e) {
        std::cerr << "❌ Validation Error: " << e.what() << std::endl;
        return 1;
    } catch (const ApiError& e) {
        std::cerr << "❌ API Error: " << e.what() << std::endl;
        return 1;
    } catch (const ConnectionError& e) {
        std::cerr << "❌ Connection Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "❌ Unexpected Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
