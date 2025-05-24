#include "test_framework.hpp"
#include "tradier/watchlist.hpp"
#include "tradier/client.hpp"
#include "tradier/common/errors.hpp"

namespace {

void test_get_watchlist_validation() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::WatchlistService watchlist(client);
    
    ASSERT_THROW(watchlist.getWatchlist(""), tradier::ValidationError);
}

void test_create_watchlist_validation() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::WatchlistService watchlist(client);
    
    ASSERT_THROW(watchlist.createWatchlist(""), tradier::ValidationError);
}

void test_update_watchlist_validation() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::WatchlistService watchlist(client);
    
    ASSERT_THROW(watchlist.updateWatchlist("", "New Name"), tradier::ValidationError);
    ASSERT_THROW(watchlist.updateWatchlist("123", ""), tradier::ValidationError);
}

void test_delete_watchlist_validation() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::WatchlistService watchlist(client);
    
    ASSERT_THROW(watchlist.deleteWatchlist(""), tradier::ValidationError);
}

void test_add_symbols_validation() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::WatchlistService watchlist(client);
    
    std::vector<std::string> emptySymbols;
    ASSERT_THROW(watchlist.addSymbols("", {"AAPL"}), tradier::ValidationError);
    ASSERT_THROW(watchlist.addSymbols("123", emptySymbols), tradier::ValidationError);
}

void test_remove_symbol_validation() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::WatchlistService watchlist(client);
    
    ASSERT_THROW(watchlist.removeSymbol("", "AAPL"), tradier::ValidationError);
    ASSERT_THROW(watchlist.removeSymbol("123", ""), tradier::ValidationError);
}

void test_valid_watchlist_calls() {
    tradier::Config config;
    config.accessToken = "test_token";
    config.sandboxMode = true;
    
    tradier::TradierClient client(config);
    tradier::WatchlistService watchlist(client);
    
    std::vector<std::string> symbols = {"AAPL", "MSFT"};
    
    ASSERT_NO_THROW(watchlist.getWatchlists());
    ASSERT_NO_THROW(watchlist.getWatchlist("123"));
    ASSERT_NO_THROW(watchlist.createWatchlist("Test List"));
    ASSERT_NO_THROW(watchlist.createWatchlist("Test List", symbols));
    ASSERT_NO_THROW(watchlist.updateWatchlist("123", "Updated List"));
    ASSERT_NO_THROW(watchlist.updateWatchlist("123", "Updated List", symbols));
    ASSERT_NO_THROW(watchlist.deleteWatchlist("123"));
    ASSERT_NO_THROW(watchlist.addSymbols("123", symbols));
    ASSERT_NO_THROW(watchlist.removeSymbol("123", "AAPL"));
}

}

int test_watchlist_main() {
    test::TestSuite suite("Watchlist Service Tests");
    
    suite.add_test("Get Watchlist Validation", test_get_watchlist_validation);
    suite.add_test("Create Watchlist Validation", test_create_watchlist_validation);
    suite.add_test("Update Watchlist Validation", test_update_watchlist_validation);
    suite.add_test("Delete Watchlist Validation", test_delete_watchlist_validation);
    suite.add_test("Add Symbols Validation", test_add_symbols_validation);
    suite.add_test("Remove Symbol Validation", test_remove_symbol_validation);
    suite.add_test("Valid Watchlist Calls", test_valid_watchlist_calls);
    
    return suite.run();
}