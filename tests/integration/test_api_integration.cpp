#include <catch2/catch_test_macros.hpp>
#include <memory>
#include "tradier/client.hpp"
#include "tradier/common/config.hpp"

using namespace tradier;

TEST_CASE("Integration - Sandbox API", "[integration][sandbox]") {
    SECTION("Connect to sandbox environment") {
        Config config;
        config.accessToken = "test-token";
        config.sandboxMode = true;
        
        auto client = std::make_unique<TradierClient>(config);
        
        REQUIRE(client != nullptr);
        REQUIRE(client->isAuthenticated());
        // Note: These tests require actual sandbox credentials
        // They should be run separately from unit tests
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