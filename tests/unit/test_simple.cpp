#include <catch2/catch_test_macros.hpp>
#include "tradier/common/config.hpp"
#include "tradier/common/types.hpp"

using namespace tradier;

TEST_CASE("Basic Types - Response", "[types]") {
    Response response;
    response.status = 200;
    response.body = "test";
    
    REQUIRE(response.success());
    
    response.status = 404;
    REQUIRE_FALSE(response.success());
}

TEST_CASE("Basic Types - Config", "[config]") {
    Config config;
    config.accessToken = "test-token";
    config.sandboxMode = false;
    
    REQUIRE(config.accessToken == "test-token");
    REQUIRE(config.baseUrl() == "https://api.tradier.com/v1");
    REQUIRE(config.wsUrl() == "wss://api.tradier.com/v1");
    
    config.sandboxMode = true;
    REQUIRE(config.baseUrl() == "https://sandbox.tradier.com/v1");
}