#include <catch2/catch_test_macros.hpp>
#include <memory>
#include "tradier/auth.hpp"
#include "mocks/mock_http_client.h"
#include "fixtures/test_data.h"

using namespace tradier;
using namespace tradier::test;

TEST_CASE("AuthService - Token Validation", "[auth]") {
    SECTION("Valid token format") {
        std::string token = "valid-token-123456789";
        REQUIRE(AuthService::is_valid_token(token));
    }

    SECTION("Empty token") {
        std::string token = "";
        REQUIRE_FALSE(AuthService::is_valid_token(token));
    }

    SECTION("Token too short") {
        std::string token = "short";
        REQUIRE_FALSE(AuthService::is_valid_token(token));
    }
}