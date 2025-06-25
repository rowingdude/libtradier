#include <catch2/catch_test_macros.hpp>
#include <memory>
#include "tradier/watchlist.hpp"
#include "tradier/client.hpp"
#include "mocks/mock_http_client.h"
#include "fixtures/test_data.h"

using namespace tradier;
using namespace tradier::test;

class WatchlistServiceTest : public WatchlistTestFixture {
public:
    void SetUp() {
        WatchlistTestFixture::setUp();
        mock_client = std::make_shared<MockHttpClient>();
        client = std::make_unique<TradierClient>(get_config(), mock_client);
        watchlist_service = std::make_unique<WatchlistService>(*client);
    }

protected:
    std::shared_ptr<MockHttpClient> mock_client;
    std::unique_ptr<TradierClient> client;
    std::unique_ptr<WatchlistService> watchlist_service;
};

TEST_CASE("WatchlistService - Get Watchlists", "[watchlist]") {
    WatchlistServiceTest test;
    test.SetUp();

    SECTION("Get all watchlists") {
        test.mock_client->set_response("GET", "*/v1/watchlists", 
            ApiResponseBuilder::success(test.get_test_watchlists_response()));

        auto result = test.watchlist_service->get_watchlists();

        REQUIRE(test.mock_client->was_called("GET", "*/v1/watchlists"));
        REQUIRE(result.is_success());
        REQUIRE(result.has_value());
        
        auto watchlists = result.value();
        REQUIRE(watchlists.size() == 1);
        REQUIRE(watchlists[0].name == "Tech Stocks");
        REQUIRE(watchlists[0].id == "default-watchlist");
        REQUIRE(watchlists[0].items.size() == 2);
    }
}

TEST_CASE("WatchlistService - Create Watchlist", "[watchlist]") {
    WatchlistServiceTest test;
    test.SetUp();

    SECTION("Create new watchlist") {
        test.mock_client->set_response("POST", "*/v1/watchlists", 
            ApiResponseBuilder::success(test.get_test_create_response()));

        auto watchlist_data = test.get_test_watchlist_data();
        auto result = test.watchlist_service->create_watchlist(watchlist_data);

        REQUIRE(test.mock_client->was_called("POST", "*/v1/watchlists"));
        REQUIRE(result.is_success());
        REQUIRE(result.has_value());
        
        auto watchlist = result.value();
        REQUIRE(watchlist.name == "New Watchlist");
        REQUIRE(watchlist.id == "new-watchlist-id");
    }
}