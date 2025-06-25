#include <catch2/catch_test_macros.hpp>
#include <memory>
#include "tradier/client.hpp"
#include "mocks/mock_http_client.h"
#include "fixtures/test_data.h"

using namespace tradier;
using namespace tradier::test;

TEST_CASE("TradierClient - Construction", "[client]") {
    SECTION("Valid configuration") {
        auto config = TestData::get_test_config();
        auto client = std::make_unique<TradierClient>(config);
        
        REQUIRE(client != nullptr);
    }

    SECTION("Empty access token") {
        auto config = TestData::get_test_config();
        config.access_token = "";
        
        REQUIRE_THROWS_AS(TradierClient(config), std::invalid_argument);
    }
}

TEST_CASE("TradierClient - Rate Limiting", "[client]") {
    SECTION("Rate limit configuration") {
        auto config = TestData::get_test_config();
        config.rate_limit_requests_per_second = 5;
        
        auto client = std::make_unique<TradierClient>(config);
        REQUIRE(client != nullptr);
    }
}