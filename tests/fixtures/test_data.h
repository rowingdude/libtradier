#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "tradier/common/config.hpp"

namespace tradier {
namespace test {

/**
 * Common test data and fixtures for libtradier tests
 */
class TestData {
public:
    // Test configuration
    static Config get_test_config();
    static Config get_sandbox_config();
    
    // Test credentials
    static std::string get_test_access_token();
    static std::string get_test_account_id();
    
    // Sample API responses
    static std::string get_sample_account_profile();
    static std::string get_sample_account_balances();
    static std::string get_sample_positions();
    static std::string get_sample_orders();
    static std::string get_sample_quotes();
    static std::string get_sample_options_chain();
    static std::string get_sample_market_calendar();
    static std::string get_sample_watchlists();
    
    // Error responses
    static std::string get_error_response(int code, const std::string& message);
    static std::string get_rate_limit_error();
    static std::string get_validation_error();
    static std::string get_auth_error();
    
    // Test symbols
    static std::vector<std::string> get_test_symbols();
    static std::string get_test_option_symbol();
    
    // Test orders
    static std::map<std::string, std::string> get_sample_equity_order();
    static std::map<std::string, std::string> get_sample_option_order();
    
    // Streaming data
    static std::string get_sample_quote_stream();
    static std::string get_sample_trade_stream();
    
    // Validation test cases
    static std::vector<std::string> get_invalid_symbols();
    static std::vector<std::string> get_invalid_account_ids();
    static std::vector<std::map<std::string, std::string>> get_invalid_orders();
    
private:
    TestData() = default;
};

/**
 * Helper class for creating test configurations
 */
class ConfigBuilder {
public:
    ConfigBuilder();
    
    ConfigBuilder& with_base_url(const std::string& url);
    ConfigBuilder& with_access_token(const std::string& token);
    ConfigBuilder& with_sandbox(bool enabled);
    ConfigBuilder& with_rate_limit(int requests_per_second);
    ConfigBuilder& with_timeout(int timeout_seconds);
    ConfigBuilder& with_retry_count(int retries);
    ConfigBuilder& with_debug_logging(bool enabled);
    
    Config build();
    
private:
    Config config_;
};

/**
 * Test fixtures for common test scenarios
 */
class TestFixture {
public:
    TestFixture();
    virtual ~TestFixture() = default;
    
    // Setup and teardown
    virtual void setUp();
    virtual void tearDown();
    
    // Common test utilities
    Config& get_config() { return config_; }
    std::string get_account_id() const { return account_id_; }
    std::string get_access_token() const { return access_token_; }
    
protected:
    Config config_;
    std::string account_id_;
    std::string access_token_;
};

/**
 * Account service test fixture
 */
class AccountTestFixture : public TestFixture {
public:
    AccountTestFixture();
    void setUp() override;
    
    // Account-specific test data
    std::string get_test_profile_response();
    std::string get_test_balances_response();
    std::string get_test_positions_response();
    std::string get_test_orders_response();
    std::string get_test_gainloss_response();
};

/**
 * Market service test fixture
 */
class MarketTestFixture : public TestFixture {
public:
    MarketTestFixture();
    void setUp() override;
    
    // Market-specific test data
    std::string get_test_quotes_response();
    std::string get_test_options_chain_response();
    std::string get_test_historical_data_response();
    std::string get_test_calendar_response();
    std::string get_test_clock_response();
    std::string get_test_search_response();
    
    std::vector<std::string> get_test_symbols();
};

/**
 * Trading service test fixture
 */
class TradingTestFixture : public TestFixture {
public:
    TradingTestFixture();
    void setUp() override;
    
    // Trading-specific test data
    std::string get_test_preview_response();
    std::string get_test_order_response();
    std::string get_test_modify_response();
    std::string get_test_cancel_response();
    
    std::map<std::string, std::string> get_test_equity_order();
    std::map<std::string, std::string> get_test_option_order();
    std::map<std::string, std::string> get_test_multileg_order();
};

/**
 * Streaming service test fixture
 */
class StreamingTestFixture : public TestFixture {
public:
    StreamingTestFixture();
    void setUp() override;
    
    // Streaming-specific test data
    std::string get_test_session_response();
    std::string get_test_quote_event();
    std::string get_test_trade_event();
    std::string get_test_summary_event();
    std::string get_test_timesale_event();
    std::string get_test_tradex_event();
    std::string get_test_option_event();
};

/**
 * Watchlist service test fixture
 */
class WatchlistTestFixture : public TestFixture {
public:
    WatchlistTestFixture();
    void setUp() override;
    
    // Watchlist-specific test data
    std::string get_test_watchlists_response();
    std::string get_test_watchlist_response();
    std::string get_test_create_response();
    std::string get_test_update_response();
    std::string get_test_delete_response();
    
    std::map<std::string, std::string> get_test_watchlist_data();
};

} // namespace test
} // namespace tradier