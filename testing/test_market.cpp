#include "test_framework.hpp"
#include "tradier/market.hpp"
#include "tradier/client.hpp"
#include "tradier/common/errors.hpp"

namespace {

void test_get_quotes_validation() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::MarketService market(client);
    
    std::vector<std::string> emptySymbols;
    ASSERT_THROW(market.getQuotes(emptySymbols), tradier::ValidationError);
}

void test_get_quote_validation() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::MarketService market(client);
    
    ASSERT_THROW(market.getQuote(""), tradier::ValidationError);
}

void test_get_option_chain_validation() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::MarketService market(client);
    
    ASSERT_THROW(market.getOptionChain("", "2024-01-19"), tradier::ValidationError);
    ASSERT_THROW(market.getOptionChain("AAPL", ""), tradier::ValidationError);
}

void test_get_option_strikes_validation() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::MarketService market(client);
    
    ASSERT_THROW(market.getOptionStrikes("", "2024-01-19"), tradier::ValidationError);
    ASSERT_THROW(market.getOptionStrikes("AAPL", ""), tradier::ValidationError);
}

void test_get_option_expirations_validation() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::MarketService market(client);
    
    ASSERT_THROW(market.getOptionExpirations(""), tradier::ValidationError);
}

void test_lookup_option_symbols_validation() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::MarketService market(client);
    
    ASSERT_THROW(market.lookupOptionSymbols(""), tradier::ValidationError);
}

void test_get_historical_data_validation() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::MarketService market(client);
    
    ASSERT_THROW(market.getHistoricalData(""), tradier::ValidationError);
}

void test_get_time_sales_validation() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::MarketService market(client);
    
    ASSERT_THROW(market.getTimeSales(""), tradier::ValidationError);
}

void test_search_symbols_validation() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::MarketService market(client);
    
    ASSERT_THROW(market.searchSymbols(""), tradier::ValidationError);
}

void test_lookup_symbols_validation() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::MarketService market(client);
    
    ASSERT_THROW(market.lookupSymbols(""), tradier::ValidationError);
}

void test_fundamentals_validation() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::MarketService market(client);
    
    ASSERT_THROW(market.getCompanyInfo(""), tradier::ValidationError);
    ASSERT_THROW(market.getCorporateCalendar(""), tradier::ValidationError);
    ASSERT_THROW(market.getDividends(""), tradier::ValidationError);
    ASSERT_THROW(market.getCorporateActions(""), tradier::ValidationError);
    ASSERT_THROW(market.getFinancialRatios(""), tradier::ValidationError);
    ASSERT_THROW(market.getFinancialStatements(""), tradier::ValidationError);
    ASSERT_THROW(market.getPriceStatistics(""), tradier::ValidationError);
}

void test_valid_market_calls() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::MarketService market(client);
    
    std::vector<std::string> symbols = {"AAPL", "MSFT"};
    
    ASSERT_NO_THROW(market.getQuotes(symbols));
    ASSERT_NO_THROW(market.getQuotesPost(symbols));
    ASSERT_NO_THROW(market.getQuote("AAPL"));
    ASSERT_NO_THROW(market.getOptionChain("AAPL", "2024-01-19"));
    ASSERT_NO_THROW(market.getOptionStrikes("AAPL", "2024-01-19"));
    ASSERT_NO_THROW(market.getOptionExpirations("AAPL"));
    ASSERT_NO_THROW(market.lookupOptionSymbols("AAPL"));
    ASSERT_NO_THROW(market.getHistoricalData("AAPL"));
    ASSERT_NO_THROW(market.getTimeSales("AAPL"));
    ASSERT_NO_THROW(market.getETBList());
    ASSERT_NO_THROW(market.getClock());
    ASSERT_NO_THROW(market.getCalendar());
    ASSERT_NO_THROW(market.searchSymbols("apple"));
    ASSERT_NO_THROW(market.lookupSymbols("AAPL"));
    ASSERT_NO_THROW(market.getCompanyInfo("AAPL"));
    ASSERT_NO_THROW(market.getCorporateCalendar("AAPL"));
    ASSERT_NO_THROW(market.getDividends("AAPL"));
    ASSERT_NO_THROW(market.getCorporateActions("AAPL"));
    ASSERT_NO_THROW(market.getFinancialRatios("AAPL"));
    ASSERT_NO_THROW(market.getFinancialStatements("AAPL"));
    ASSERT_NO_THROW(market.getPriceStatistics("AAPL"));
}

}

int test_market_main() {
    test::TestSuite suite("Market Service Tests");
    
    suite.add_test("Get Quotes Validation", test_get_quotes_validation);
    suite.add_test("Get Quote Validation", test_get_quote_validation);
    suite.add_test("Get Option Chain Validation", test_get_option_chain_validation);
    suite.add_test("Get Option Strikes Validation", test_get_option_strikes_validation);
    suite.add_test("Get Option Expirations Validation", test_get_option_expirations_validation);
    suite.add_test("Lookup Option Symbols Validation", test_lookup_option_symbols_validation);
    suite.add_test("Get Historical Data Validation", test_get_historical_data_validation);
    suite.add_test("Get Time Sales Validation", test_get_time_sales_validation);
    suite.add_test("Search Symbols Validation", test_search_symbols_validation);
    suite.add_test("Lookup Symbols Validation", test_lookup_symbols_validation);
    suite.add_test("Fundamentals Validation", test_fundamentals_validation);
    suite.add_test("Valid Market Calls", test_valid_market_calls);
    
    return suite.run();
}