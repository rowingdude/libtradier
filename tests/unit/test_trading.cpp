#include <catch2/catch_test_macros.hpp>
#include <memory>
#include "tradier/trading.hpp"
#include "tradier/client.hpp"
#include "mocks/mock_http_client.h"
#include "fixtures/test_data.h"

using namespace tradier;
using namespace tradier::test;

class TradingServiceTest : public TradingTestFixture {
public:
    void SetUp() {
        TradingTestFixture::setUp();
        mock_client = std::make_shared<MockHttpClient>();
        client = std::make_unique<TradierClient>(get_config(), mock_client);
        trading_service = std::make_unique<TradingService>(*client);
    }

protected:
    std::shared_ptr<MockHttpClient> mock_client;
    std::unique_ptr<TradierClient> client;
    std::unique_ptr<TradingService> trading_service;
};

TEST_CASE("TradingService - Preview Order", "[trading]") {
    TradingServiceTest test;
    test.SetUp();

    SECTION("Preview equity order") {
        test.mock_client->set_response("POST", "*/v1/accounts/*/orders", 
            ApiResponseBuilder::order_preview());

        auto order_data = test.get_test_equity_order();
        order_data["preview"] = "true";
        
        auto result = test.trading_service->preview_order(test.get_account_id(), order_data);

        REQUIRE(test.mock_client->was_called("POST", "*/v1/accounts/*/orders"));
        REQUIRE(result.is_success());
        REQUIRE(result.has_value());
        
        auto preview = result.value();
        REQUIRE(preview.status == "ok");
        REQUIRE(preview.cost == 15000.00);
        REQUIRE(preview.symbol == "AAPL");
        REQUIRE(preview.quantity == 100);
    }

    SECTION("Invalid order data") {
        auto result = test.trading_service->preview_order(test.get_account_id(), {});

        REQUIRE_FALSE(result.is_success());
        REQUIRE(result.error().type == ApiError::Type::Validation);
    }
}

TEST_CASE("TradingService - Submit Order", "[trading]") {
    TradingServiceTest test;
    test.SetUp();

    SECTION("Submit equity order") {
        test.mock_client->set_response("POST", "*/v1/accounts/*/orders", 
            ApiResponseBuilder::order_submitted("123456"));

        auto order_data = test.get_test_equity_order();
        auto result = test.trading_service->submit_order(test.get_account_id(), order_data);

        REQUIRE(test.mock_client->was_called("POST", "*/v1/accounts/*/orders"));
        REQUIRE(result.is_success());
        REQUIRE(result.has_value());
        
        auto order = result.value();
        REQUIRE(order.id == 123456);
        REQUIRE(order.status == "submitted");
    }

    SECTION("Submit option order") {
        test.mock_client->set_response("POST", "*/v1/accounts/*/orders", 
            ApiResponseBuilder::order_submitted("789012"));

        auto order_data = test.get_test_option_order();
        auto result = test.trading_service->submit_order(test.get_account_id(), order_data);

        REQUIRE(result.is_success());
        REQUIRE(result.has_value());
        REQUIRE(result.value().id == 789012);
    }
}