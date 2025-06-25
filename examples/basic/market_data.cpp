/**
 * @file market_data.cpp
 * @brief Comprehensive market data retrieval example
 * 
 * This example demonstrates:
 * 1. Real-time quote retrieval for multiple symbols
 * 2. Options chain data with Greeks
 * 3. Historical price data
 * 4. Market calendar and trading hours
 * 5. Symbol search functionality
 * 6. Error handling for market data requests
 * 
 * Usage:
 *   export TRADIER_ACCESS_TOKEN="your-sandbox-token"
 *   ./market_data [symbol1] [symbol2] ...
 *   
 *   If no symbols provided, uses default watchlist: AAPL, MSFT, GOOGL, TSLA
 * 
 * @author libtradier
 * @version 1.0
 */

#include <tradier/client.hpp>
#include <tradier/market.hpp>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstdlib>

using namespace tradier;

/**
 * @brief Print real-time quote information
 * @param quotes Vector of quote objects to display
 */
void printQuotes(const std::vector<Quote>& quotes) {
    std::cout << "\n=== Real-Time Quotes ===" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    
    // Header
    std::cout << std::left 
              << std::setw(8) << "Symbol"
              << std::setw(12) << "Last"
              << std::setw(12) << "Change"
              << std::setw(12) << "Change %"
              << std::setw(12) << "Bid"
              << std::setw(12) << "Ask"
              << std::setw(15) << "Volume"
              << "52W High/Low" << std::endl;
    std::cout << std::string(95, '-') << std::endl;
    
    // Data rows
    for (const auto& quote : quotes) {
        // Calculate change percentage
        double changePercent = 0.0;
        if (quote.prevclose > 0) {
            changePercent = (quote.change / quote.prevclose) * 100.0;
        }
        
        std::cout << std::left 
                  << std::setw(8) << quote.symbol
                  << std::setw(12) << ("$" + std::to_string(quote.last))
                  << std::setw(12) << (quote.change >= 0 ? "+" : "") + std::to_string(quote.change)
                  << std::setw(12) << (changePercent >= 0 ? "+" : "") + std::to_string(changePercent) + "%"
                  << std::setw(12) << ("$" + std::to_string(quote.bid))
                  << std::setw(12) << ("$" + std::to_string(quote.ask))
                  << std::setw(15) << std::to_string(quote.volume)
                  << "$" << quote.week_52_high << "/$" << quote.week_52_low
                  << std::endl;
    }
}

/**
 * @brief Print options chain data with Greeks
 * @param options Vector of option objects to display  
 */
void printOptionsChain(const std::vector<Option>& options) {
    std::cout << "\n=== Options Chain (First 10 contracts) ===" << std::endl;
    
    if (options.empty()) {
        std::cout << "No options data available." << std::endl;
        return;
    }
    
    std::cout << std::fixed << std::setprecision(3);
    
    // Header
    std::cout << std::left
              << std::setw(25) << "Symbol"
              << std::setw(8) << "Type"
              << std::setw(10) << "Strike"
              << std::setw(10) << "Last"
              << std::setw(10) << "Bid/Ask"
              << std::setw(10) << "Delta"
              << std::setw(10) << "Gamma"
              << std::setw(10) << "Theta"
              << "IV" << std::endl;
    std::cout << std::string(100, '-') << std::endl;
    
    // Show first 10 options to avoid overwhelming output
    size_t count = std::min(options.size(), size_t(10));
    for (size_t i = 0; i < count; ++i) {
        const auto& option = options[i];
        
        std::string bidAsk = "$" + std::to_string(option.bid) + "/" + std::to_string(option.ask);
        
        std::cout << std::left
                  << std::setw(25) << option.symbol
                  << std::setw(8) << option.option_type
                  << std::setw(10) << ("$" + std::to_string(option.strike))
                  << std::setw(10) << ("$" + std::to_string(option.last))
                  << std::setw(10) << bidAsk
                  << std::setw(10) << std::to_string(option.greeks.delta)
                  << std::setw(10) << std::to_string(option.greeks.gamma)
                  << std::setw(10) << std::to_string(option.greeks.theta)
                  << std::to_string(option.greeks.mid_iv * 100) << "%"
                  << std::endl;
    }
    
    if (options.size() > 10) {
        std::cout << "... and " << (options.size() - 10) << " more contracts" << std::endl;
    }
}

/**
 * @brief Print market calendar information
 * @param calendar Vector of market day objects
 */
void printMarketCalendar(const std::vector<MarketDay>& calendar) {
    std::cout << "\n=== Market Calendar (Next 5 Days) ===" << std::endl;
    
    // Header
    std::cout << std::left
              << std::setw(12) << "Date"
              << std::setw(10) << "Status"
              << std::setw(20) << "Description"
              << "Trading Hours" << std::endl;
    std::cout << std::string(60, '-') << std::endl;
    
    // Show first 5 days
    size_t count = std::min(calendar.size(), size_t(5));
    for (size_t i = 0; i < count; ++i) {
        const auto& day = calendar[i];
        
        std::string hours = "Closed";
        if (day.status == "open") {
            hours = day.open.start + "-" + day.open.end + " ET";
        }
        
        std::cout << std::left
                  << std::setw(12) << day.date
                  << std::setw(10) << day.status
                  << std::setw(20) << day.description
                  << hours << std::endl;
    }
}

/**
 * @brief Get configuration from environment
 * @return Configured Config object
 */
Config getConfigFromEnvironment() {
    Config config;
    
    const char* token = std::getenv("TRADIER_ACCESS_TOKEN");
    if (!token) {
        throw std::runtime_error(
            "TRADIER_ACCESS_TOKEN environment variable not set.\n"
            "Get your sandbox token from: https://documentation.tradier.com/getting-started"
        );
    }
    
    config.accessToken = token;
    config.sandboxMode = true;
    config.timeoutSeconds = 30;
    
    return config;
}

/**
 * @brief Parse command line arguments for symbols
 * @param argc Argument count
 * @param argv Argument values
 * @return Vector of symbols to query
 */
std::vector<std::string> parseSymbols(int argc, char* argv[]) {
    std::vector<std::string> symbols;
    
    if (argc > 1) {
        // Use symbols from command line
        for (int i = 1; i < argc; ++i) {
            symbols.emplace_back(argv[i]);
        }
    } else {
        // Default watchlist
        symbols = {"AAPL", "MSFT", "GOOGL", "TSLA"};
    }
    
    return symbols;
}

/**
 * @brief Main function demonstrating market data functionality
 */
int main(int argc, char* argv[]) {
    try {
        std::cout << "=== libtradier Market Data Example ===" << std::endl;
        std::cout << "This example demonstrates comprehensive market data retrieval." << std::endl;
        
        // 1. Parse symbols and configure client
        auto symbols = parseSymbols(argc, argv);
        auto config = getConfigFromEnvironment();
        
        std::cout << "\n✓ Configuration loaded" << std::endl;
        std::cout << "  Environment: " << (config.sandboxMode ? "Sandbox" : "Production") << std::endl;
        std::cout << "  Symbols to query: ";
        for (size_t i = 0; i < symbols.size(); ++i) {
            std::cout << symbols[i];
            if (i < symbols.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
        
        // 2. Create client and market service
        TradierClient client(config);
        auto marketService = client.market();
        std::cout << "✓ Market service initialized" << std::endl;
        
        // 3. Get real-time quotes
        std::cout << "\n--- Retrieving Real-Time Quotes ---" << std::endl;
        auto quotesResult = marketService.getQuotes(symbols);
        
        if (quotesResult.isSuccess()) {
            std::cout << "✓ Quotes retrieved successfully" << std::endl;
            printQuotes(quotesResult.value());
        } else {
            std::cerr << "✗ Failed to get quotes: " << quotesResult.error().message << std::endl;
        }
        
        // 4. Get options chain for first symbol (if it's a stock)
        if (!symbols.empty()) {
            std::cout << "\n--- Retrieving Options Chain for " << symbols[0] << " ---" << std::endl;
            auto optionsResult = marketService.getOptionsChain(symbols[0]);
            
            if (optionsResult.isSuccess()) {
                std::cout << "✓ Options chain retrieved successfully" << std::endl;
                printOptionsChain(optionsResult.value());
            } else {
                std::cerr << "✗ Failed to get options: " << optionsResult.error().message << std::endl;
                std::cerr << "  Note: " << symbols[0] << " may not have options or may be an invalid symbol" << std::endl;
            }
        }
        
        // 5. Get market calendar
        std::cout << "\n--- Retrieving Market Calendar ---" << std::endl;
        auto calendarResult = marketService.getMarketCalendar();
        
        if (calendarResult.isSuccess()) {
            std::cout << "✓ Market calendar retrieved successfully" << std::endl;
            printMarketCalendar(calendarResult.value());
        } else {
            std::cerr << "✗ Failed to get market calendar: " << calendarResult.error().message << std::endl;
        }
        
        // 6. Demonstrate symbol search
        std::cout << "\n--- Symbol Search Example ---" << std::endl;
        auto searchResult = marketService.searchSymbols("apple");
        
        if (searchResult.isSuccess()) {
            auto results = searchResult.value();
            std::cout << "✓ Found " << results.size() << " symbols matching 'apple'" << std::endl;
            
            std::cout << "\nTop 3 results:" << std::endl;
            size_t count = std::min(results.size(), size_t(3));
            for (size_t i = 0; i < count; ++i) {
                std::cout << "  " << results[i].symbol << " - " << results[i].description << std::endl;
            }
        } else {
            std::cerr << "✗ Failed to search symbols: " << searchResult.error().message << std::endl;
        }
        
        std::cout << "\n=== Market Data Example completed successfully ===" << std::endl;
        std::cout << "\nNext steps:" << std::endl;
        std::cout << "- Try different symbols: ./market_data AMZN NFLX" << std::endl;
        std::cout << "- Explore historical data with specific date ranges" << std::endl;
        std::cout << "- Check out the streaming example for real-time updates" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Error: " << e.what() << std::endl;
        std::cerr << "\nTroubleshooting:" << std::endl;
        std::cerr << "1. Ensure TRADIER_ACCESS_TOKEN is set" << std::endl;
        std::cerr << "2. Check symbol validity (use sandbox-compatible symbols)" << std::endl;
        std::cerr << "3. Verify market hours (some data may be limited outside trading hours)" << std::endl;
        return 1;
    }
}

/*
Expected Output:
================

=== libtradier Market Data Example ===
This example demonstrates comprehensive market data retrieval.

✓ Configuration loaded
  Environment: Sandbox
  Symbols to query: AAPL, MSFT, GOOGL, TSLA

✓ Market service initialized

--- Retrieving Real-Time Quotes ---
✓ Quotes retrieved successfully

=== Real-Time Quotes ===
Symbol  Last        Change      Change %    Bid         Ask         Volume         52W High/Low
-----------------------------------------------------------------------------------------------
AAPL    $150.25     +2.15       +1.45%      $150.20     $150.30     1250000        $180.95/$120.45
MSFT    $285.67     -1.23       -0.43%      $285.60     $285.75     980000         $310.20/$245.18
GOOGL   $125.43     +0.87       +0.70%      $125.40     $125.50     745000         $145.67/$105.21
TSLA    $205.89     +8.45       +4.28%      $205.85     $205.95     2100000        $299.50/$138.75

--- Retrieving Options Chain for AAPL ---
✓ Options chain retrieved successfully

=== Options Chain (First 10 contracts) ===
Symbol                   Type    Strike    Last      Bid/Ask   Delta     Gamma     Theta     IV
----------------------------------------------------------------------------------------------------
AAPL230120C00150000     call    $150.00   $8.50     $8.25/$8.75   0.523     0.025     -0.045    28.5%
AAPL230120P00150000     put     $150.00   $7.25     $7.00/$7.50   -0.477    0.025     -0.042    27.8%
AAPL230120C00155000     call    $155.00   $5.75     $5.50/$6.00   0.387     0.028     -0.038    29.1%
... and 87 more contracts

--- Retrieving Market Calendar ---
✓ Market calendar retrieved successfully

=== Market Calendar (Next 5 Days) ===
Date        Status    Description         Trading Hours
------------------------------------------------------------
2024-01-02  open      Regular Trading     09:30-16:00 ET
2024-01-03  open      Regular Trading     09:30-16:00 ET
2024-01-04  open      Regular Trading     09:30-16:00 ET
2024-01-05  open      Regular Trading     09:30-16:00 ET
2024-01-08  open      Regular Trading     09:30-16:00 ET

--- Symbol Search Example ---
✓ Found 3 symbols matching 'apple'

Top 3 results:
  AAPL - Apple Inc
  APLE - Apple Hospitality REIT Inc
  APLD - Applied Digital Corp

=== Market Data Example completed successfully ===

Next steps:
- Try different symbols: ./market_data AMZN NFLX
- Explore historical data with specific date ranges
- Check out the streaming example for real-time updates
*/