#include <catch2/catch_test_macros.hpp>
#include <memory>
#include "tradier/market.hpp"
#include "tradier/client.hpp"
#include "mocks/mock_http_client.h"
#include "fixtures/test_data.h"

using namespace tradier;
using namespace tradier::test;

class MarketServiceTest : public MarketTestFixture {
public:
    void SetUp() {
        MarketTestFixture::setUp();
        mock_client = std::make_shared<MockHttpClient>();
        client = std::make_unique<TradierClient>(get_config(), mock_client);
        market_service = std::make_unique<MarketService>(*client);
    }

protected:
    std::shared_ptr<MockHttpClient> mock_client;
    std::unique_ptr<TradierClient> client;
    std::unique_ptr<MarketService> market_service;
};

TEST_CASE("MarketService - Get Quotes", "[market]") {
    MarketServiceTest test;
    test.SetUp();

    SECTION("Single symbol quote") {
        test.mock_client->set_response("GET", "*/v1/markets/quotes*", 
            ApiResponseBuilder::quotes({"AAPL"}));

        auto result = test.market_service->get_quotes({"AAPL"});

        REQUIRE(test.mock_client->was_called("GET", "*/v1/markets/quotes*"));
        REQUIRE(result.is_success());
        REQUIRE(result.has_value());
        
        auto quotes = result.value();
        REQUIRE(quotes.size() == 1);
        REQUIRE(quotes[0].symbol == "AAPL");
        REQUIRE(quotes[0].last == 150.00);
        REQUIRE(quotes[0].bid == 149.90);
        REQUIRE(quotes[0].ask == 150.10);
    }

    SECTION("Multiple symbol quotes") {
        auto symbols = test.get_test_symbols();
        test.mock_client->set_response("GET", "*/v1/markets/quotes*", 
            ApiResponseBuilder::quotes(symbols));

        auto result = test.market_service->get_quotes(symbols);

        REQUIRE(result.is_success());
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == symbols.size());
    }

    SECTION("Empty symbol list") {
        auto result = test.market_service->get_quotes({});

        REQUIRE_FALSE(result.is_success());
        REQUIRE(result.error().type == ApiError::Type::Validation);
    }
}

TEST_CASE("MarketService - Get Options Chain", "[market]") {
    MarketServiceTest test;
    test.SetUp();

    SECTION("Basic options chain") {
        test.mock_client->set_response("GET", "*/v1/markets/options/chains*", 
            ApiResponseBuilder::options_chain("AAPL"));

        auto result = test.market_service->get_options_chain("AAPL");

        REQUIRE(test.mock_client->was_called("GET", "*/v1/markets/options/chains*"));
        REQUIRE(result.is_success());
        REQUIRE(result.has_value());
        
        auto options = result.value();
        REQUIRE(options.size() == 1);
        REQUIRE(options[0].symbol == "AAPL230120C00150000");
        REQUIRE(options[0].underlying == "AAPL");
        REQUIRE(options[0].strike == 150.00);
        REQUIRE(options[0].option_type == "call");
    }

    SECTION("Invalid symbol") {
        auto result = test.market_service->get_options_chain("");

        REQUIRE_FALSE(result.is_success());
        REQUIRE(result.error().type == ApiError::Type::Validation);
    }
}

TEST_CASE("MarketService - Get Market Calendar", "[market]") {
    MarketServiceTest test;
    test.SetUp();

    SECTION("Market calendar retrieval") {
        test.mock_client->set_response("GET", "*/v1/markets/calendar*", 
            ApiResponseBuilder::market_calendar());

        auto result = test.market_service->get_market_calendar();

        REQUIRE(test.mock_client->was_called("GET", "*/v1/markets/calendar*"));
        REQUIRE(result.is_success());
        REQUIRE(result.has_value());
        
        auto calendar = result.value();
        REQUIRE(calendar.size() >= 1);
        REQUIRE(calendar[0].date == "2023-01-02");
        REQUIRE(calendar[0].status == "open");
    }
}