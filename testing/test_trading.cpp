// testing/test_trading.cpp
#include "test_framework.hpp"
#include "tradier/trading.hpp"
#include "tradier/client.hpp"
#include "tradier/common/errors.hpp"

namespace {

void test_order_type_conversions() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::TradingService trading(client);
    
    ASSERT_EQ("market", trading.toString(tradier::OrderType::MARKET));
    ASSERT_EQ("limit", trading.toString(tradier::OrderType::LIMIT));
    ASSERT_EQ("stop", trading.toString(tradier::OrderType::STOP));
    ASSERT_EQ("stop_limit", trading.toString(tradier::OrderType::STOP_LIMIT));
    ASSERT_EQ("debit", trading.toString(tradier::OrderType::DEBIT));
    ASSERT_EQ("credit", trading.toString(tradier::OrderType::CREDIT));
}

void test_order_side_conversions() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::TradingService trading(client);
    
    ASSERT_EQ("buy", trading.toString(tradier::OrderSide::BUY));
    ASSERT_EQ("sell", trading.toString(tradier::OrderSide::SELL));
    ASSERT_EQ("buy_to_open", trading.toString(tradier::OrderSide::BUY_TO_OPEN));
    ASSERT_EQ("buy_to_close", trading.toString(tradier::OrderSide::BUY_TO_CLOSE));
    ASSERT_EQ("sell_to_open", trading.toString(tradier::OrderSide::SELL_TO_OPEN));
    ASSERT_EQ("sell_to_close", trading.toString(tradier::OrderSide::SELL_TO_CLOSE));
}

void test_order_duration_conversions() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::TradingService trading(client);
    
    ASSERT_EQ("day", trading.toString(tradier::OrderDuration::DAY));
    ASSERT_EQ("gtc", trading.toString(tradier::OrderDuration::GTC));
    ASSERT_EQ("pre", trading.toString(tradier::OrderDuration::PRE));
    ASSERT_EQ("post", trading.toString(tradier::OrderDuration::POST));
}

void test_order_status_parsing() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::TradingService trading(client);
    
    ASSERT_EQ(tradier::OrderStatus::OPEN, trading.parseOrderStatus("open"));
    ASSERT_EQ(tradier::OrderStatus::FILLED, trading.parseOrderStatus("filled"));
    ASSERT_EQ(tradier::OrderStatus::CANCELED, trading.parseOrderStatus("canceled"));
    ASSERT_EQ(tradier::OrderStatus::EXPIRED, trading.parseOrderStatus("expired"));
    ASSERT_EQ(tradier::OrderStatus::REJECTED, trading.parseOrderStatus("rejected"));
    ASSERT_EQ(tradier::OrderStatus::PENDING, trading.parseOrderStatus("unknown_status"));
}

void test_place_order_validation() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::TradingService trading(client);
    
    tradier::OrderRequest request;
    request.symbol = "AAPL";
    request.side = tradier::OrderSide::BUY;
    request.quantity = 100;
    request.type = tradier::OrderType::MARKET;
    
    ASSERT_NO_THROW(trading.placeOrder("test_account", request));
    ASSERT_NO_THROW(trading.buyStock("test_account", "AAPL", 100));
    ASSERT_NO_THROW(trading.sellStock("test_account", "AAPL", 100));
}

void test_cancel_order_validation() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::TradingService trading(client);
    
    ASSERT_NO_THROW(trading.cancelOrder("test_account", 12345));
    ASSERT_NO_THROW(trading.cancelAllOrders("test_account"));
}

void test_option_trading() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::TradingService trading(client);
    
    ASSERT_NO_THROW(trading.buyToOpenOption("test_account", "AAPL240119C00150000", 1));
    ASSERT_NO_THROW(trading.sellToCloseOption("test_account", "AAPL240119C00150000", 1));
    ASSERT_NO_THROW(trading.sellToOpenOption("test_account", "AAPL240119P00140000", 1));
    ASSERT_NO_THROW(trading.buyToCloseOption("test_account", "AAPL240119P00140000", 1));
}

}

int test_trading_main() {
    test::TestSuite suite("Trading Service Tests");
    
    suite.add_test("Order Type Conversions", test_order_type_conversions);
    suite.add_test("Order Side Conversions", test_order_side_conversions);
    suite.add_test("Order Duration Conversions", test_order_duration_conversions);
    suite.add_test("Order Status Parsing", test_order_status_parsing);
    suite.add_test("Place Order Validation", test_place_order_validation);
    suite.add_test("Cancel Order Validation", test_cancel_order_validation);
    suite.add_test("Option Trading", test_option_trading);
    
    return suite.run();
}