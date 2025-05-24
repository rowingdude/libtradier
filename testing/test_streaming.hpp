#include "test_framework.hpp"
#include "tradier/streaming.hpp"
#include "tradier/client.hpp"
#include "tradier/common/errors.hpp"

namespace {

void test_streaming_config() {
    tradier::StreamingConfig config;
    
    ASSERT_TRUE(config.autoReconnect);
    ASSERT_EQ(5000, config.reconnectDelay);
    ASSERT_EQ(10, config.maxReconnectAttempts);
    ASSERT_EQ(30000, config.heartbeatInterval);
    ASSERT_TRUE(config.filterDuplicates);
}

void test_stream_statistics() {
    tradier::StreamStatistics stats;
    
    ASSERT_EQ(0, stats.messagesReceived.load());
    ASSERT_EQ(0, stats.messagesProcessed.load());
    ASSERT_EQ(0, stats.errors.load());
    ASSERT_EQ(0, stats.reconnects.load());
    
    stats.messagesReceived++;
    stats.messagesProcessed++;
    stats.errors++;
    stats.reconnects++;
    
    ASSERT_EQ(1, stats.messagesReceived.load());
    ASSERT_EQ(1, stats.messagesProcessed.load());
    ASSERT_EQ(1, stats.errors.load());
    ASSERT_EQ(1, stats.reconnects.load());
    
    auto snapshot = stats.getSnapshot();
    ASSERT_EQ(1, snapshot.messagesReceived);
    ASSERT_EQ(1, snapshot.messagesProcessed);
    ASSERT_EQ(1, snapshot.errors);
    ASSERT_EQ(1, snapshot.reconnects);
    
    stats.reset();
    ASSERT_EQ(0, stats.messagesReceived.load());
    ASSERT_EQ(0, stats.messagesProcessed.load());
    ASSERT_EQ(0, stats.errors.load());
    ASSERT_EQ(0, stats.reconnects.load());
}

void test_streaming_events() {
    tradier::TradeEvent trade;
    trade.type = "trade";
    trade.symbol = "AAPL";
    trade.price = 150.0;
    trade.size = 100;
    
    ASSERT_EQ("trade", trade.type);
    ASSERT_EQ("AAPL", trade.symbol);
    ASSERT_EQ(150.0, trade.price);
    ASSERT_EQ(100, trade.size);
    
    tradier::QuoteEvent quote;
    quote.type = "quote";
    quote.symbol = "AAPL";
    quote.bid = 149.50;
    quote.ask = 150.50;
    quote.bidSize = 10;
    quote.askSize = 15;
    
    ASSERT_EQ("quote", quote.type);
    ASSERT_EQ("AAPL", quote.symbol);
    ASSERT_EQ(149.50, quote.bid);
    ASSERT_EQ(150.50, quote.ask);
    ASSERT_EQ(10, quote.bidSize);
    ASSERT_EQ(15, quote.askSize);
    
    tradier::SummaryEvent summary;
    summary.type = "summary";
    summary.symbol = "AAPL";
    summary.open = 148.0;
    summary.high = 152.0;
    summary.low = 147.0;
    summary.prevClose = 149.0;
    
    ASSERT_EQ("summary", summary.type);
    ASSERT_EQ("AAPL", summary.symbol);
    ASSERT_EQ(148.0, summary.open);
    ASSERT_EQ(152.0, summary.high);
    ASSERT_EQ(147.0, summary.low);
    ASSERT_EQ(149.0, summary.prevClose);
    
    tradier::TimesaleEvent timesale;
    timesale.type = "timesale";
    timesale.symbol = "AAPL";
    timesale.last = 150.25;
    timesale.size = 200;
    timesale.session = "regular";
    
    ASSERT_EQ("timesale", timesale.type);
    ASSERT_EQ("AAPL", timesale.symbol);
    ASSERT_EQ(150.25, timesale.last);
    ASSERT_EQ(200, timesale.size);
    ASSERT_EQ("regular", timesale.session);
    
    tradier::AccountOrderEvent order;
    order.orderId = 12345;
    order.event = "fill";
    order.status = "filled";
    order.account = "12345678";
    order.symbol = "AAPL";
    order.quantity = 100;
    order.price = 150.0;
    order.side = "buy";
    order.type = "market";
    
    ASSERT_EQ(12345, order.orderId);
    ASSERT_EQ("fill", order.event);
    ASSERT_EQ("filled", order.status);
    ASSERT_EQ("12345678", order.account);
    ASSERT_EQ("AAPL", order.symbol);
    ASSERT_EQ(100, order.quantity);
    ASSERT_EQ(150.0, order.price);
    ASSERT_EQ("buy", order.side);
    ASSERT_EQ("market", order.type);
    
    tradier::AccountPositionEvent position;
    position.account = "12345678";
    position.symbol = "AAPL";
    position.quantity = 100;
    position.costBasis = 15000.0;
    
    ASSERT_EQ("12345678", position.account);
    ASSERT_EQ("AAPL", position.symbol);
    ASSERT_EQ(100, position.quantity);
    ASSERT_EQ(15000.0, position.costBasis);
}

void test_streaming_service_creation() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::StreamingService streaming(client);
    
    ASSERT_FALSE(streaming.isConnected());
    ASSERT_EQ("Disconnected", streaming.getConnectionStatus());
    
    auto stats = streaming.getStatistics();
    ASSERT_EQ(0, stats.messagesReceived);
    ASSERT_EQ(0, stats.messagesProcessed);
    ASSERT_EQ(0, stats.errors);
    ASSERT_EQ(0, stats.reconnects);
}

void test_streaming_config_management() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::StreamingService streaming(client);
    
    tradier::StreamingConfig streamConfig;
    streamConfig.autoReconnect = false;
    streamConfig.reconnectDelay = 10000;
    streamConfig.maxReconnectAttempts = 5;
    streamConfig.heartbeatInterval = 60000;
    streamConfig.filterDuplicates = false;
    
    streaming.setConfig(streamConfig);
    
    auto retrievedConfig = streaming.getConfig();
    ASSERT_FALSE(retrievedConfig.autoReconnect);
    ASSERT_EQ(10000, retrievedConfig.reconnectDelay);
    ASSERT_EQ(5, retrievedConfig.maxReconnectAttempts);
    ASSERT_EQ(60000, retrievedConfig.heartbeatInterval);
    ASSERT_FALSE(retrievedConfig.filterDuplicates);
}

void test_streaming_symbol_management() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::StreamingService streaming(client);
    
    std::vector<std::string> symbols1 = {"AAPL", "MSFT"};
    std::vector<std::string> symbols2 = {"GOOGL", "TSLA"};
    
    ASSERT_TRUE(streaming.addSymbols(symbols1));
    ASSERT_TRUE(streaming.addSymbols(symbols2));
    
    auto subscribedSymbols = streaming.getSubscribedSymbols();
    ASSERT_EQ(4, subscribedSymbols.size());
    
    ASSERT_TRUE(streaming.removeSymbols({"MSFT"}));
    
    subscribedSymbols = streaming.getSubscribedSymbols();
    ASSERT_EQ(3, subscribedSymbols.size());
    
    streaming.setSymbolFilter({"AAPL", "GOOGL"});
    streaming.setExchangeFilter({"NASDAQ", "NYSE"});
    
    streaming.clearFilters();
}

void test_streaming_error_handling() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::StreamingService streaming(client);
    
    bool errorReceived = false;
    std::string errorMessage;
    
    streaming.setErrorHandler([&](const std::string& error) {
        errorReceived = true;
        errorMessage = error;
    });
    
    ASSERT_FALSE(errorReceived);
}

void test_streaming_session_management() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::StreamingService streaming(client);
    
    auto marketSession = streaming.createMarketSession();
    auto accountSession = streaming.createAccountSession();
    
    ASSERT_NO_THROW(streaming.disconnect());
}

}

int test_streaming_main() {
    test::TestSuite suite("Streaming Service Tests");
    
    suite.add_test("Streaming Config", test_streaming_config);
    suite.add_test("Stream Statistics", test_stream_statistics);
    suite.add_test("Streaming Events", test_streaming_events);
    suite.add_test("Streaming Service Creation", test_streaming_service_creation);
    suite.add_test("Streaming Config Management", test_streaming_config_management);
    suite.add_test("Streaming Symbol Management", test_streaming_symbol_management);
    suite.add_test("Streaming Error Handling", test_streaming_error_handling);
    suite.add_test("Streaming Session Management", test_streaming_session_management);
    
    return suite.run();
}