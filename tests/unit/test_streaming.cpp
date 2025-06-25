#include <catch2/catch_test_macros.hpp>
#include <memory>
#include "tradier/streaming.hpp"
#include "tradier/client.hpp"
#include "mocks/mock_http_client.h"
#include "fixtures/test_data.h"

using namespace tradier;
using namespace tradier::test;

class StreamingServiceTest : public StreamingTestFixture {
public:
    void SetUp() {
        StreamingTestFixture::setUp();
        mock_client = std::make_shared<MockHttpClient>();
        client = std::make_unique<TradierClient>(get_config(), mock_client);
        streaming_service = std::make_unique<StreamingService>(*client);
    }

protected:
    std::shared_ptr<MockHttpClient> mock_client;
    std::unique_ptr<TradierClient> client;
    std::unique_ptr<StreamingService> streaming_service;
};

TEST_CASE("StreamingService - Create Session", "[streaming]") {
    StreamingServiceTest test;
    test.SetUp();

    SECTION("Create streaming session") {
        test.mock_client->set_response("POST", "*/v1/markets/events/sessions", 
            ApiResponseBuilder::success(test.get_test_session_response()));

        auto result = test.streaming_service->create_session();

        REQUIRE(test.mock_client->was_called("POST", "*/v1/markets/events/sessions"));
        REQUIRE(result.is_success());
        REQUIRE(result.has_value());
        
        auto session = result.value();
        REQUIRE(session.url == "wss://ws.tradier.com/v1/markets/events");
        REQUIRE(session.sessionid == "test-session-12345");
    }
}

TEST_CASE("StreamingService - Event Processing", "[streaming]") {
    StreamingServiceTest test;
    test.SetUp();

    SECTION("Process quote event") {
        auto quote_data = test.get_test_quote_event();
        // Test event processing logic would go here
        // This would typically test the event parsing and callback system
        REQUIRE(!quote_data.empty());
    }

    SECTION("Process trade event") {
        auto trade_data = test.get_test_trade_event();
        REQUIRE(!trade_data.empty());
    }
}