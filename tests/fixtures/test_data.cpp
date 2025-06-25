#include "test_data.h"
#include <stdexcept>

namespace tradier {
namespace test {

// TestData implementations
Config TestData::get_test_config() {
    ConfigBuilder builder;
    return builder
        .with_base_url("https://sandbox.tradier.com")
        .with_access_token("test-token-12345")
        .with_sandbox(true)
        .with_debug_logging(true)
        .build();
}

Config TestData::get_sandbox_config() {
    ConfigBuilder builder;
    return builder
        .with_base_url("https://sandbox.tradier.com")
        .with_access_token("sandbox-token")
        .with_sandbox(true)
        .build();
}

std::string TestData::get_test_access_token() {
    return "test-access-token-123456789";
}

std::string TestData::get_test_account_id() {
    return "123456789";
}

std::string TestData::get_sample_account_profile() {
    return R"({
        "profile": {
            "account_number": "123456789",
            "classification": "individual",
            "date_created": "2020-01-01T00:00:00.000Z",
            "day_trader": false,
            "option_level": 2,
            "status": "active",
            "type": "margin",
            "last_update_date": "2023-01-01T00:00:00.000Z"
        }
    })";
}

std::string TestData::get_sample_account_balances() {
    return R"({
        "balances": {
            "option_short_value": 0,
            "total_equity": 50000.00,
            "account_number": "123456789",
            "account_type": "margin",
            "close_pl": 0,
            "current_requirement": 0,
            "equity": 50000.00,
            "long_market_value": 45000.00,
            "market_value": 45000.00,
            "open_pl": 1000.00,
            "option_long_value": 0,
            "option_requirement": 0,
            "pending_orders_count": 0,
            "short_market_value": 0,
            "stock_long_value": 45000.00,
            "total_cash": 5000.00,
            "uncleared_funds": 0,
            "unsettled_funds": 0,
            "day_trade_buying_power": 100000.00,
            "option_buying_power": 25000.00,
            "stock_buying_power": 25000.00
        }
    })";
}

std::string TestData::get_sample_positions() {
    return R"({
        "positions": {
            "position": [
                {
                    "cost_basis": 10000.00,
                    "date_acquired": "2023-01-01T00:00:00.000Z",
                    "id": 1,
                    "quantity": 100,
                    "symbol": "AAPL"
                },
                {
                    "cost_basis": 5000.00,
                    "date_acquired": "2023-01-15T00:00:00.000Z",
                    "id": 2,
                    "quantity": 50,
                    "symbol": "GOOGL"
                }
            ]
        }
    })";
}

std::string TestData::get_sample_orders() {
    return R"({
        "orders": {
            "order": [
                {
                    "id": 123456,
                    "type": "market",
                    "symbol": "AAPL",
                    "side": "buy",
                    "quantity": 100.00000000,
                    "status": "filled",
                    "duration": "day",
                    "price": 0.00000000,
                    "avg_fill_price": 150.00000000,
                    "exec_quantity": 100.00000000,
                    "last_fill_price": 150.00000000,
                    "last_fill_quantity": 100.00000000,
                    "remaining_quantity": 0.00000000,
                    "create_date": "2023-01-01T12:00:00.000Z",
                    "transaction_date": "2023-01-01T12:00:05.000Z",
                    "class": "equity"
                }
            ]
        }
    })";
}

std::string TestData::get_sample_quotes() {
    return R"({
        "quotes": {
            "quote": [
                {
                    "symbol": "AAPL",
                    "description": "Apple Inc",
                    "exch": "NASDAQ",
                    "type": "stock",
                    "last": 150.00,
                    "change": 2.50,
                    "volume": 1000000,
                    "open": 148.00,
                    "high": 152.00,
                    "low": 147.50,
                    "close": 147.50,
                    "bid": 149.90,
                    "ask": 150.10,
                    "change_percentage": 1.69,
                    "average_volume": 950000,
                    "last_volume": 100,
                    "trade_date": 1640995200000,
                    "prevclose": 147.50,
                    "week_52_high": 180.00,
                    "week_52_low": 120.00,
                    "bidsize": 10,
                    "bidexch": "NASDAQ",
                    "bid_date": 1640995200000,
                    "asksize": 12,
                    "askexch": "NASDAQ",
                    "ask_date": 1640995200000,
                    "root_symbols": "AAPL"
                }
            ]
        }
    })";
}

std::string TestData::get_sample_options_chain() {
    return R"({
        "options": {
            "option": [
                {
                    "symbol": "AAPL230120C00150000",
                    "description": "AAPL Jan 20 2023 $150.00 Call",
                    "exch": "OPRA",
                    "type": "option",
                    "last": 5.50,
                    "change": 0.25,
                    "volume": 500,
                    "open": 5.25,
                    "high": 5.75,
                    "low": 5.20,
                    "close": 5.25,
                    "bid": 5.45,
                    "ask": 5.55,
                    "underlying": "AAPL",
                    "strike": 150.00,
                    "option_type": "call",
                    "expiration_date": "2023-01-20",
                    "expiration_type": "standard",
                    "contract_size": 100,
                    "greeks": {
                        "delta": 0.55,
                        "gamma": 0.025,
                        "theta": -0.045,
                        "vega": 0.18,
                        "rho": 0.12,
                        "phi": -0.025,
                        "bid_iv": 0.28,
                        "mid_iv": 0.29,
                        "ask_iv": 0.30,
                        "smv_vol": 0.29,
                        "updated_at": "2023-01-01T16:00:00.000Z"
                    }
                }
            ]
        }
    })";
}

std::string TestData::get_sample_market_calendar() {
    return R"({
        "calendar": {
            "days": {
                "day": [
                    {
                        "date": "2023-01-02",
                        "status": "open",
                        "description": "Regular Trading Day",
                        "premarket": {
                            "start": "04:00",
                            "end": "09:30"
                        },
                        "open": {
                            "start": "09:30",
                            "end": "16:00"
                        },
                        "postmarket": {
                            "start": "16:00",
                            "end": "20:00"
                        }
                    }
                ]
            }
        }
    })";
}

std::string TestData::get_sample_watchlists() {
    return R"({
        "watchlists": {
            "watchlist": [
                {
                    "name": "Tech Stocks",
                    "id": "default-watchlist",
                    "public_id": "12345",
                    "items": {
                        "item": [
                            {
                                "symbol": "AAPL",
                                "id": 1
                            },
                            {
                                "symbol": "GOOGL",
                                "id": 2
                            }
                        ]
                    }
                }
            ]
        }
    })";
}

std::string TestData::get_error_response(int code, const std::string& message) {
    return R"({"error": {"code": )" + std::to_string(code) + R"(, "message": ")" + message + R"("}})";
}

std::string TestData::get_rate_limit_error() {
    return get_error_response(429, "Rate limit exceeded");
}

std::string TestData::get_validation_error() {
    return get_error_response(400, "Invalid request parameters");
}

std::string TestData::get_auth_error() {
    return get_error_response(401, "Unauthorized access");
}

std::vector<std::string> TestData::get_test_symbols() {
    return {"AAPL", "GOOGL", "MSFT", "AMZN", "TSLA"};
}

std::string TestData::get_test_option_symbol() {
    return "AAPL230120C00150000";
}

std::map<std::string, std::string> TestData::get_sample_equity_order() {
    return {
        {"class", "equity"},
        {"symbol", "AAPL"},
        {"side", "buy"},
        {"quantity", "100"},
        {"type", "market"},
        {"duration", "day"}
    };
}

std::map<std::string, std::string> TestData::get_sample_option_order() {
    return {
        {"class", "option"},
        {"symbol", "AAPL230120C00150000"},
        {"side", "buy_to_open"},
        {"quantity", "1"},
        {"type", "market"},
        {"duration", "day"}
    };
}

std::string TestData::get_sample_quote_stream() {
    return R"({
        "type": "quote",
        "symbol": "AAPL",
        "bid": 149.90,
        "bidsz": 10,
        "bidexch": "Q",
        "biddate": 1640995200000,
        "ask": 150.10,
        "asksz": 12,
        "askexch": "Q",
        "askdate": 1640995200000
    })";
}

std::string TestData::get_sample_trade_stream() {
    return R"({
        "type": "trade",
        "symbol": "AAPL",
        "exch": "Q",
        "price": 150.00,
        "size": 100,
        "cvol": 1000000,
        "date": 1640995200000,
        "last": 150.00
    })";
}

std::vector<std::string> TestData::get_invalid_symbols() {
    return {"", "INVALID", "12345", "TOOLONG"};
}

std::vector<std::string> TestData::get_invalid_account_ids() {
    return {"", "invalid", "12345678901234567890"};
}

std::vector<std::map<std::string, std::string>> TestData::get_invalid_orders() {
    return {
        {{"class", "equity"}, {"symbol", ""}, {"side", "buy"}, {"quantity", "100"}},  // Missing symbol
        {{"class", "equity"}, {"symbol", "AAPL"}, {"side", ""}, {"quantity", "100"}}, // Missing side
        {{"class", "equity"}, {"symbol", "AAPL"}, {"side", "buy"}, {"quantity", "0"}}, // Invalid quantity
        {{"class", "equity"}, {"symbol", "AAPL"}, {"side", "buy"}, {"quantity", "-100"}} // Negative quantity
    };
}

// ConfigBuilder implementations
ConfigBuilder::ConfigBuilder() {
    config_.base_url = "https://api.tradier.com";
    config_.sandbox = false;
    config_.debug_logging = false;
    config_.rate_limit_requests_per_second = 10;
    config_.timeout_seconds = 30;
    config_.max_retry_attempts = 3;
}

ConfigBuilder& ConfigBuilder::with_base_url(const std::string& url) {
    config_.base_url = url;
    return *this;
}

ConfigBuilder& ConfigBuilder::with_access_token(const std::string& token) {
    config_.access_token = token;
    return *this;
}

ConfigBuilder& ConfigBuilder::with_sandbox(bool enabled) {
    config_.sandbox = enabled;
    if (enabled) {
        config_.base_url = "https://sandbox.tradier.com";
    }
    return *this;
}

ConfigBuilder& ConfigBuilder::with_rate_limit(int requests_per_second) {
    config_.rate_limit_requests_per_second = requests_per_second;
    return *this;
}

ConfigBuilder& ConfigBuilder::with_timeout(int timeout_seconds) {
    config_.timeout_seconds = timeout_seconds;
    return *this;
}

ConfigBuilder& ConfigBuilder::with_retry_count(int retries) {
    config_.max_retry_attempts = retries;
    return *this;
}

ConfigBuilder& ConfigBuilder::with_debug_logging(bool enabled) {
    config_.debug_logging = enabled;
    return *this;
}

Config ConfigBuilder::build() {
    return config_;
}

// TestFixture implementations
TestFixture::TestFixture() 
    : config_(TestData::get_test_config()),
      account_id_(TestData::get_test_account_id()),
      access_token_(TestData::get_test_access_token()) {
}

void TestFixture::setUp() {
    // Base setup - can be overridden by derived classes
}

void TestFixture::tearDown() {
    // Base teardown - can be overridden by derived classes
}

// AccountTestFixture implementations
AccountTestFixture::AccountTestFixture() : TestFixture() {
}

void AccountTestFixture::setUp() {
    TestFixture::setUp();
    // Account-specific setup
}

std::string AccountTestFixture::get_test_profile_response() {
    return TestData::get_sample_account_profile();
}

std::string AccountTestFixture::get_test_balances_response() {
    return TestData::get_sample_account_balances();
}

std::string AccountTestFixture::get_test_positions_response() {
    return TestData::get_sample_positions();
}

std::string AccountTestFixture::get_test_orders_response() {
    return TestData::get_sample_orders();
}

std::string AccountTestFixture::get_test_gainloss_response() {
    return R"({
        "gainloss": {
            "closed_position": [
                {
                    "close_date": "2023-01-15T16:00:00.000Z",
                    "cost": 10000.00,
                    "gain_loss": 500.00,
                    "gain_loss_percent": 5.00,
                    "open_date": "2023-01-01T09:30:00.000Z",
                    "proceeds": 10500.00,
                    "quantity": 100,
                    "symbol": "AAPL",
                    "term": "short"
                }
            ]
        }
    })";
}

// MarketTestFixture implementations
MarketTestFixture::MarketTestFixture() : TestFixture() {
}

void MarketTestFixture::setUp() {
    TestFixture::setUp();
    // Market-specific setup
}

std::string MarketTestFixture::get_test_quotes_response() {
    return TestData::get_sample_quotes();
}

std::string MarketTestFixture::get_test_options_chain_response() {
    return TestData::get_sample_options_chain();
}

std::string MarketTestFixture::get_test_historical_data_response() {
    return R"({
        "history": {
            "day": [
                {
                    "date": "2023-01-01",
                    "open": 148.00,
                    "high": 152.00,
                    "low": 147.50,
                    "close": 150.00,
                    "volume": 1000000
                }
            ]
        }
    })";
}

std::string MarketTestFixture::get_test_calendar_response() {
    return TestData::get_sample_market_calendar();
}

std::string MarketTestFixture::get_test_clock_response() {
    return R"({
        "clock": {
            "date": "2023-01-02",
            "description": "Regular Trading Day",
            "state": "open",
            "timestamp": 1640995200,
            "time_zone": "EST",
            "next_state": "closed",
            "next_change": 1641034800
        }
    })";
}

std::string MarketTestFixture::get_test_search_response() {
    return R"({
        "securities": {
            "security": [
                {
                    "symbol": "AAPL",
                    "exchange": "NASDAQ",
                    "type": "stock",
                    "description": "Apple Inc"
                }
            ]
        }
    })";
}

std::vector<std::string> MarketTestFixture::get_test_symbols() {
    return TestData::get_test_symbols();
}

// TradingTestFixture implementations
TradingTestFixture::TradingTestFixture() : TestFixture() {
}

void TradingTestFixture::setUp() {
    TestFixture::setUp();
    // Trading-specific setup
}

std::string TradingTestFixture::get_test_preview_response() {
    return R"({
        "order": {
            "status": "ok",
            "commission": 0.00,
            "cost": 15000.00,
            "fees": 0.00,
            "symbol": "AAPL",
            "quantity": 100,
            "side": "buy",
            "type": "market",
            "duration": "day",
            "result": true,
            "order_cost": 15000.00,
            "margin_change": 7500.00,
            "request_date": "2023-01-01T12:00:00.000Z",
            "extended_hours": false,
            "class": "equity",
            "strategy": "equity"
        }
    })";
}

std::string TradingTestFixture::get_test_order_response() {
    return R"({
        "order": {
            "id": 123456,
            "status": "submitted",
            "partner_id": "test-partner"
        }
    })";
}

std::string TradingTestFixture::get_test_modify_response() {
    return R"({
        "order": {
            "id": 123456,
            "status": "submitted",
            "partner_id": "test-partner"
        }
    })";
}

std::string TradingTestFixture::get_test_cancel_response() {
    return R"({
        "order": {
            "id": 123456,
            "status": "cancelled"
        }
    })";
}

std::map<std::string, std::string> TradingTestFixture::get_test_equity_order() {
    return TestData::get_sample_equity_order();
}

std::map<std::string, std::string> TradingTestFixture::get_test_option_order() {
    return TestData::get_sample_option_order();
}

std::map<std::string, std::string> TradingTestFixture::get_test_multileg_order() {
    return {
        {"class", "multileg"},
        {"symbol", "AAPL"},
        {"type", "market"},
        {"duration", "day"},
        {"option_symbol[0]", "AAPL230120C00150000"},
        {"side[0]", "buy_to_open"},
        {"quantity[0]", "1"},
        {"option_symbol[1]", "AAPL230120P00140000"},
        {"side[1]", "sell_to_open"},
        {"quantity[1]", "1"}
    };
}

// StreamingTestFixture implementations
StreamingTestFixture::StreamingTestFixture() : TestFixture() {
}

void StreamingTestFixture::setUp() {
    TestFixture::setUp();
    // Streaming-specific setup
}

std::string StreamingTestFixture::get_test_session_response() {
    return R"({
        "stream": {
            "url": "wss://ws.tradier.com/v1/markets/events",
            "sessionid": "test-session-12345"
        }
    })";
}

std::string StreamingTestFixture::get_test_quote_event() {
    return TestData::get_sample_quote_stream();
}

std::string StreamingTestFixture::get_test_trade_event() {
    return TestData::get_sample_trade_stream();
}

std::string StreamingTestFixture::get_test_summary_event() {
    return R"({
        "type": "summary",
        "symbol": "AAPL",
        "open": 148.00,
        "high": 152.00,
        "low": 147.50,
        "prevclose": 147.50,
        "close": 150.00
    })";
}

std::string StreamingTestFixture::get_test_timesale_event() {
    return R"({
        "type": "timesale",
        "symbol": "AAPL",
        "exch": "Q",
        "bid": 149.90,
        "ask": 150.10,
        "last": 150.00,
        "size": 100,
        "date": 1640995200000
    })";
}

std::string StreamingTestFixture::get_test_tradex_event() {
    return R"({
        "type": "tradex",
        "symbol": "AAPL",
        "exch": "Q",
        "price": 150.00,
        "size": 100,
        "cvol": 1000000,
        "date": 1640995200000,
        "last": 150.00
    })";
}

std::string StreamingTestFixture::get_test_option_event() {
    return R"({
        "type": "option",
        "symbol": "AAPL230120C00150000",
        "bid": 5.45,
        "ask": 5.55,
        "last": 5.50,
        "size": 10,
        "date": 1640995200000
    })";
}

// WatchlistTestFixture implementations
WatchlistTestFixture::WatchlistTestFixture() : TestFixture() {
}

void WatchlistTestFixture::setUp() {
    TestFixture::setUp();
    // Watchlist-specific setup
}

std::string WatchlistTestFixture::get_test_watchlists_response() {
    return TestData::get_sample_watchlists();
}

std::string WatchlistTestFixture::get_test_watchlist_response() {
    return R"({
        "watchlist": {
            "name": "Tech Stocks",
            "id": "default-watchlist",
            "public_id": "12345",
            "items": {
                "item": [
                    {
                        "symbol": "AAPL",
                        "id": 1
                    },
                    {
                        "symbol": "GOOGL",
                        "id": 2
                    }
                ]
            }
        }
    })";
}

std::string WatchlistTestFixture::get_test_create_response() {
    return R"({
        "watchlist": {
            "name": "New Watchlist",
            "id": "new-watchlist-id",
            "public_id": "54321"
        }
    })";
}

std::string WatchlistTestFixture::get_test_update_response() {
    return R"({
        "watchlist": {
            "name": "Updated Watchlist",
            "id": "watchlist-id",
            "public_id": "12345"
        }
    })";
}

std::string WatchlistTestFixture::get_test_delete_response() {
    return R"({
        "watchlist": {
            "result": "Watchlist deleted successfully"
        }
    })";
}

std::map<std::string, std::string> WatchlistTestFixture::get_test_watchlist_data() {
    return {
        {"name", "Test Watchlist"},
        {"symbols", "AAPL,GOOGL,MSFT"}
    };
}

} // namespace test
} // namespace tradier