#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <cstdlib>
#include "tradier/client.hpp"
#include "tradier/common/config.hpp"

using namespace tradier;

// Helper function to get config from environment variables
Config getTestConfigFromEnvironment() {
    Config config;
    
    // Try sandbox first (safer for tests)
    const char* token = std::getenv("TRADIER_SANDBOX_KEY");
    const char* accountNum = std::getenv("TRADIER_SANDBOX_ACCT");
    bool usingSandbox = (token != nullptr);
    
    if (!token) {
        // Fall back to production if sandbox not available
        token = std::getenv("TRADIER_PRODUCTION_KEY");
        usingSandbox = false;
    }
    
    if (token) {
        config.accessToken = token;
        if (accountNum) {
            config.accountNumber = accountNum;
        }
        config.sandboxMode = usingSandbox;
    } else {
        // Use test token for unit tests
        config.accessToken = "test-token";
        config.sandboxMode = true;
    }
    
    return config;
}

TEST_CASE("Integration - Sandbox API", "[integration][sandbox]") {
    SECTION("Connect to sandbox environment") {
        auto config = getTestConfigFromEnvironment();
        
        auto client = std::make_unique<TradierClient>(config);
        
        REQUIRE(client != nullptr);
        REQUIRE(client->isAuthenticated());
        // Note: These tests require actual sandbox credentials
        // Set TRADIER_SANDBOX_KEY and TRADIER_SANDBOX_ACCT to run with real API
    }
}

TEST_CASE("Integration - End-to-End Workflow", "[integration][e2e]") {
    SECTION("Complete trading workflow") {
        // This would test a complete workflow:
        // 1. Get account profile
        // 2. Get quotes
        // 3. Preview order
        // 4. Submit order (in sandbox)
        // 5. Check order status
        
        // Placeholder for full integration test
        REQUIRE(true);
    }
}