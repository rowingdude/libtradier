#include <catch2/catch_test_macros.hpp>
#include <memory>
#include "tradier/account.hpp"
#include "tradier/client.hpp"
#include "mocks/mock_http_client.h"
#include "fixtures/test_data.h"

using namespace tradier;
using namespace tradier::test;

class AccountServiceTest : public AccountTestFixture {
public:
    void SetUp() {
        AccountTestFixture::setUp();
        mock_client = std::make_shared<MockHttpClient>();
        client = std::make_unique<TradierClient>(get_config(), mock_client);
        account_service = std::make_unique<AccountService>(*client);
    }

protected:
    std::shared_ptr<MockHttpClient> mock_client;
    std::unique_ptr<TradierClient> client;
    std::unique_ptr<AccountService> account_service;
};

TEST_CASE("AccountService - Get Profile", "[account]") {
    AccountServiceTest test;
    test.SetUp();

    SECTION("Successful profile retrieval") {
        // Setup mock response
        test.mock_client->set_response("GET", "*/v1/user/profile", 
            ApiResponseBuilder::success(test.get_test_profile_response()));

        // Execute request
        auto result = test.account_service->get_profile();

        // Verify request was made
        REQUIRE(test.mock_client->was_called("GET", "*/v1/user/profile"));
        
        // Verify result
        REQUIRE(result.is_success());
        REQUIRE(result.has_value());
        
        auto profile = result.value();
        REQUIRE(!profile.accounts.empty());
        REQUIRE(profile.accounts[0].number == "123456789");
        REQUIRE(profile.accounts[0].classification == "individual");
        REQUIRE(profile.accounts[0].type == "margin");
        REQUIRE_FALSE(profile.accounts[0].dayTrader);
        REQUIRE(profile.accounts[0].optionLevel == 2);
    }

    SECTION("Unauthorized access") {
        test.mock_client->set_response("GET", "*/v1/user/profile", 
            ApiResponseBuilder::unauthorized());

        auto result = test.account_service->get_profile();

        REQUIRE_FALSE(result.is_success());
        REQUIRE(result.error().type == ApiError::Type::Authentication);
    }

    SECTION("Network error") {
        test.mock_client->simulate_network_error(true);

        auto result = test.account_service->get_profile();

        REQUIRE_FALSE(result.is_success());
        REQUIRE(result.error().type == ApiError::Type::Network);
    }
}

TEST_CASE("AccountService - Get Balances", "[account]") {
    AccountServiceTest test;
    test.SetUp();

    SECTION("Successful balances retrieval") {
        test.mock_client->set_response("GET", "*/v1/accounts/*/balances", 
            ApiResponseBuilder::success(test.get_test_balances_response()));

        auto result = test.account_service->get_balances(test.get_account_id());

        REQUIRE(test.mock_client->was_called("GET", "*/v1/accounts/*/balances"));
        REQUIRE(result.is_success());
        REQUIRE(result.has_value());
        
        auto balances = result.value();
        REQUIRE(balances.total_equity == 50000.00);
        REQUIRE(balances.total_cash == 5000.00);
        REQUIRE(balances.day_trade_buying_power == 100000.00);
        REQUIRE(balances.option_buying_power == 25000.00);
        REQUIRE(balances.stock_buying_power == 25000.00);
    }

    SECTION("Invalid account ID") {
        test.mock_client->set_response("GET", "*/v1/accounts/*/balances", 
            ApiResponseBuilder::not_found());

        auto result = test.account_service->get_balances("invalid-account");

        REQUIRE_FALSE(result.is_success());
        REQUIRE(result.error().type == ApiError::Type::NotFound);
    }

    SECTION("Rate limit exceeded") {
        test.mock_client->set_response("GET", "*/v1/accounts/*/balances", 
            ApiResponseBuilder::rate_limit_exceeded());

        auto result = test.account_service->get_balances(test.get_account_id());

        REQUIRE_FALSE(result.is_success());
        REQUIRE(result.error().type == ApiError::Type::RateLimit);
    }
}

TEST_CASE("AccountService - Get Positions", "[account]") {
    AccountServiceTest test;
    test.SetUp();

    SECTION("Successful positions retrieval") {
        test.mock_client->set_response("GET", "*/v1/accounts/*/positions", 
            ApiResponseBuilder::success(test.get_test_positions_response()));

        auto result = test.account_service->get_positions(test.get_account_id());

        REQUIRE(test.mock_client->was_called("GET", "*/v1/accounts/*/positions"));
        REQUIRE(result.is_success());
        REQUIRE(result.has_value());
        
        auto positions = result.value();
        REQUIRE(positions.size() == 2);
        
        REQUIRE(positions[0].symbol == "AAPL");
        REQUIRE(positions[0].quantity == 100);
        REQUIRE(positions[0].costBasis == 10000.00);
        
        REQUIRE(positions[1].symbol == "GOOGL");
        REQUIRE(positions[1].quantity == 50);
        REQUIRE(positions[1].costBasis == 5000.00);
    }

    SECTION("No positions") {
        test.mock_client->set_response("GET", "*/v1/accounts/*/positions", 
            ApiResponseBuilder::success(R"({"positions": null})"));

        auto result = test.account_service->get_positions(test.get_account_id());

        REQUIRE(result.is_success());
        REQUIRE(result.has_value());
        REQUIRE(result.value().empty());
    }
}

TEST_CASE("AccountService - Get Orders", "[account]") {
    AccountServiceTest test;
    test.SetUp();

    SECTION("Successful orders retrieval") {
        test.mock_client->set_response("GET", "*/v1/accounts/*/orders", 
            ApiResponseBuilder::success(test.get_test_orders_response()));

        auto result = test.account_service->get_orders(test.get_account_id());

        REQUIRE(test.mock_client->was_called("GET", "*/v1/accounts/*/orders"));
        REQUIRE(result.is_success());
        REQUIRE(result.has_value());
        
        auto orders = result.value();
        REQUIRE(orders.size() == 1);
        
        auto& order = orders[0];
        REQUIRE(order.id == 123456);
        REQUIRE(order.symbol == "AAPL");
        REQUIRE(order.side == "buy");
        REQUIRE(order.quantity == 100.0);
        REQUIRE(order.status == "filled");
        REQUIRE(order.type == "market");
    }

    SECTION("Filter by status") {
        test.mock_client->set_response("GET", "*/v1/accounts/*/orders*status=open*", 
            ApiResponseBuilder::success(R"({"orders": {"order": []}})"));

        auto result = test.account_service->get_orders(test.get_account_id(), "open");

        REQUIRE(test.mock_client->was_called("GET", "*/v1/accounts/*/orders*status=open*"));
        REQUIRE(result.is_success());
        REQUIRE(result.has_value());
        REQUIRE(result.value().empty());
    }
}

TEST_CASE("AccountService - Get Gain/Loss", "[account]") {
    AccountServiceTest test;
    test.SetUp();

    SECTION("Successful gain/loss retrieval") {
        test.mock_client->set_response("GET", "*/v1/accounts/*/gainloss", 
            ApiResponseBuilder::success(test.get_test_gainloss_response()));

        auto result = test.account_service->get_gainloss(test.get_account_id());

        REQUIRE(test.mock_client->was_called("GET", "*/v1/accounts/*/gainloss"));
        REQUIRE(result.is_success());
        REQUIRE(result.has_value());
        
        auto gainloss = result.value();
        REQUIRE(gainloss.size() == 1);
        
        auto& position = gainloss[0];
        REQUIRE(position.symbol == "AAPL");
        REQUIRE(position.cost == 10000.00);
        REQUIRE(position.proceeds == 10500.00);
        REQUIRE(position.gain_loss == 500.00);
        REQUIRE(position.gain_loss_percent == 5.00);
        REQUIRE(position.quantity == 100);
        REQUIRE(position.term == "short");
    }

    SECTION("Date range filter") {
        test.mock_client->set_response("GET", "*/v1/accounts/*/gainloss*start=*end=*", 
            ApiResponseBuilder::success(test.get_test_gainloss_response()));

        auto result = test.account_service->get_gainloss(test.get_account_id(), "2023-01-01", "2023-12-31");

        REQUIRE(test.mock_client->was_called("GET", "*/v1/accounts/*/gainloss*start=*end=*"));
        REQUIRE(result.is_success());
    }
}

TEST_CASE("AccountService - Input Validation", "[account][validation]") {
    AccountServiceTest test;
    test.SetUp();

    SECTION("Empty account ID validation") {
        auto result = test.account_service->get_balances("");

        REQUIRE_FALSE(result.is_success());
        REQUIRE(result.error().type == ApiError::Type::Validation);
        REQUIRE(result.error().message.find("Account ID") != std::string::npos);
    }

    SECTION("Invalid date format validation") {
        auto result = test.account_service->get_gainloss(test.get_account_id(), "invalid-date", "2023-12-31");

        REQUIRE_FALSE(result.is_success());
        REQUIRE(result.error().type == ApiError::Type::Validation);
        REQUIRE(result.error().message.find("date") != std::string::npos);
    }
}

TEST_CASE("AccountService - Error Handling", "[account][error]") {
    AccountServiceTest test;
    test.SetUp();

    SECTION("Server error handling") {
        test.mock_client->set_response("GET", "*/v1/user/profile", 
            ApiResponseBuilder::server_error());

        auto result = test.account_service->get_profile();

        REQUIRE_FALSE(result.is_success());
        REQUIRE(result.error().type == ApiError::Type::Server);
    }

    SECTION("Timeout handling") {
        test.mock_client->set_response("GET", "*/v1/user/profile", 
            ApiResponseBuilder::network_timeout());

        auto result = test.account_service->get_profile();

        REQUIRE_FALSE(result.is_success());
        REQUIRE(result.error().type == ApiError::Type::Network);
    }

    SECTION("Malformed JSON response") {
        test.mock_client->set_response("GET", "*/v1/user/profile", 
            ApiResponseBuilder::success("invalid json"));

        auto result = test.account_service->get_profile();

        REQUIRE_FALSE(result.is_success());
        REQUIRE(result.error().type == ApiError::Type::Parse);
    }
}

TEST_CASE("AccountService - Request Headers", "[account][headers]") {
    AccountServiceTest test;
    test.SetUp();

    test.mock_client->set_response("GET", "*/v1/user/profile", 
        ApiResponseBuilder::success(test.get_test_profile_response()));

    auto result = test.account_service->get_profile();

    // Verify authorization header was set
    auto last_request = test.mock_client->get_last_request();
    REQUIRE(last_request.headers.find("Authorization") != last_request.headers.end());
    REQUIRE(last_request.headers.at("Authorization").find("Bearer") != std::string::npos);
    
    // Verify content type
    REQUIRE(last_request.headers.find("Accept") != last_request.headers.end());
    REQUIRE(last_request.headers.at("Accept") == "application/json");
}