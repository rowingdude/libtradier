#include "tradier/client.hpp"
#include "tradier/market.hpp"
#include "tradier/common/errors.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

void printQuotes(const std::vector<tradier::Quote>& quotes) {
    std::cout << "Found " << quotes.size() << " quotes:\n";
    for (const auto& quote : quotes) {
        std::cout << "  " << quote.symbol << " (" << quote.description << "):\n";
        std::cout << "    Last: $" << std::fixed << std::setprecision(2);
        if (quote.last) {
            std::cout << *quote.last;
        } else {
            std::cout << "N/A";
        }
        std::cout << " | Bid: $" << quote.bid << " | Ask: $" << quote.ask;
        std::cout << " | Volume: " << quote.volume << "\n";
        
        if (quote.underlying) {
            std::cout << "    Option: " << *quote.underlying << " ";
            if (quote.strike) std::cout << "$" << *quote.strike << " ";
            if (quote.optionType) std::cout << *quote.optionType << " ";
            if (quote.expirationDate) std::cout << "exp: " << *quote.expirationDate;
            std::cout << "\n";
        }
    }
    std::cout << std::endl;
}

void printOptionChain(const std::vector<tradier::OptionChain>& chain) {
    std::cout << "Found " << chain.size() << " options:\n";
    
    // Group by type
    std::vector<tradier::OptionChain> calls, puts;
    for (const auto& option : chain) {
        if (option.optionType == "call") {
            calls.push_back(option);
        } else if (option.optionType == "put") {
            puts.push_back(option);
        }
    }
    
    std::cout << "  Calls (" << calls.size() << "):\n";
    for (const auto& call : calls) {
        std::cout << "    $" << std::fixed << std::setprecision(1) << call.strike;
        std::cout << " - Bid: $" << std::setprecision(2) << call.bid;
        std::cout << " Ask: $" << call.ask;
        std::cout << " Vol: " << call.volume;
        std::cout << " OI: " << call.openInterest << "\n";
    }
    
    std::cout << "  Puts (" << puts.size() << "):\n";
    for (const auto& put : puts) {
        std::cout << "    $" << std::fixed << std::setprecision(1) << put.strike;
        std::cout << " - Bid: $" << std::setprecision(2) << put.bid;
        std::cout << " Ask: $" << put.ask;
        std::cout << " Vol: " << put.volume;
        std::cout << " OI: " << put.openInterest << "\n";
    }
    std::cout << std::endl;
}

void printHistoricalData(const std::vector<tradier::HistoricalData>& history) {
    std::cout << "Found " << history.size() << " historical data points:\n";
    
    size_t displayCount = std::min(size_t(5), history.size());
    for (size_t i = 0; i < displayCount; ++i) {
        const auto& data = history[i];
        std::cout << "  " << data.date << ": ";
        std::cout << "O: $" << std::fixed << std::setprecision(2) << data.open;
        std::cout << " H: $" << data.high;
        std::cout << " L: $" << data.low;
        std::cout << " C: $" << data.close;
        std::cout << " Vol: " << data.volume << "\n";
    }
    if (history.size() > displayCount) {
        std::cout << "  ... and " << (history.size() - displayCount) << " more entries\n";
    }
    std::cout << std::endl;
}

void printSecurities(const std::vector<tradier::Security>& securities, const std::string& title) {
    std::cout << title << " (" << securities.size() << " results):\n";
    
    size_t displayCount = std::min(size_t(10), securities.size());
    for (size_t i = 0; i < displayCount; ++i) {
        const auto& security = securities[i];
        std::cout << "  " << security.symbol << " (" << security.exchange << ") - ";
        std::cout << security.description << " [" << security.type << "]\n";
    }
    if (securities.size() > displayCount) {
        std::cout << "  ... and " << (securities.size() - displayCount) << " more results\n";
    }
    std::cout << std::endl;
}

void printMarketClock(const tradier::MarketClock& clock) {
    std::cout << "Market Clock:\n";
    std::cout << "  Date: " << clock.date << "\n";
    std::cout << "  State: " << clock.state << "\n";
    std::cout << "  Description: " << clock.description << "\n";
    std::cout << "  Next Change: " << clock.nextChange << " (" << clock.nextState << ")\n";
    std::cout << std::endl;
}

int main() {
    try {
        auto config = tradier::Config::fromEnvironment();
        tradier::TradierClient client(config);
        auto marketService = client.market();
        
        std::cout << "=== Tradier Market Data Feature Test ===" << std::endl;
        std::cout << "Using " << (config.sandboxMode ? "SANDBOX" : "PRODUCTION") << " environment\n\n";
        
        // Test 1: Market Clock
        std::cout << "Test 1: Getting market status..." << std::endl;
        auto clock = marketService.getClock();
        if (!clock) {
            std::cerr << "Failed to get market clock" << std::endl;
            return 1;
        }
        printMarketClock(*clock);
        
        // Test 2: Single Stock Quote
        std::cout << "Test 2: Getting single stock quote for AAPL..." << std::endl;
        auto appleQuote = marketService.getQuote("AAPL");
        if (!appleQuote) {
            std::cerr << "Failed to get AAPL quote" << std::endl;
            return 1;
        }
        printQuotes({*appleQuote});
        
        // Test 3: Multiple Stock Quotes
        std::cout << "Test 3: Getting multiple stock quotes..." << std::endl;
        std::vector<std::string> symbols = {"AAPL", "MSFT", "GOOGL", "TSLA", "SPY"};
        auto quotes = marketService.getQuotes(symbols);
        if (!quotes) {
            std::cerr << "Failed to get multiple quotes" << std::endl;
            return 1;
        }
        printQuotes(*quotes);
        
        // Test 4: POST Method for Quotes
        std::cout << "Test 4: Testing POST method for quotes..." << std::endl;
        auto quotesPost = marketService.getQuotesPost({"QQQ", "IWM"});
        if (!quotesPost) {
            std::cerr << "Failed to get quotes via POST" << std::endl;
            return 1;
        }
        printQuotes(*quotesPost);
        
        // Test 5: Option Expirations
        std::cout << "Test 5: Getting option expirations for SPY..." << std::endl;
        auto expirations = marketService.getOptionExpirations("SPY", false, true, true, true);
        if (!expirations) {
            std::cerr << "Failed to get option expirations" << std::endl;
            return 1;
        }
        std::cout << "Found " << expirations->size() << " expiration dates:\n";
        size_t expDisplayCount = std::min(size_t(5), expirations->size());
        for (size_t i = 0; i < expDisplayCount; ++i) {
            const auto& exp = (*expirations)[i];
            std::cout << "  " << exp.date << " (" << exp.expirationType << ") - ";
            std::cout << exp.strikes.size() << " strikes, contract size: " << exp.contractSize << "\n";
        }
        if (expirations->size() > expDisplayCount) {
            std::cout << "  ... and " << (expirations->size() - expDisplayCount) << " more expirations\n";
        }
        std::cout << std::endl;
        
        // Test 6: Option Chain (using first available expiration)
        if (!expirations->empty()) {
            std::string firstExpiration = expirations->front().date;
            std::cout << "Test 6: Getting option chain for SPY " << firstExpiration << "..." << std::endl;
            auto optionChain = marketService.getOptionChain("SPY", firstExpiration, false);
            if (!optionChain) {
                std::cerr << "Failed to get option chain" << std::endl;
                return 1;
            }
            
            // Show limited results to keep output manageable
            std::vector<tradier::OptionChain> limitedChain;
            size_t maxOptions = std::min(size_t(20), optionChain->size());
            for (size_t i = 0; i < maxOptions; ++i) {
                limitedChain.push_back((*optionChain)[i]);
            }
            printOptionChain(limitedChain);
            
            // Test 7: Option Strikes for the same expiration
            std::cout << "Test 7: Getting option strikes for SPY " << firstExpiration << "..." << std::endl;
            auto strikes = marketService.getOptionStrikes("SPY", firstExpiration);
            if (!strikes) {
                std::cerr << "Failed to get option strikes" << std::endl;
                return 1;
            }
            std::cout << "Found " << strikes->size() << " strike prices: ";
            size_t strikeDisplayCount = std::min(size_t(10), strikes->size());
            for (size_t i = 0; i < strikeDisplayCount; ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << "$" << std::fixed << std::setprecision(1) << (*strikes)[i];
            }
            if (strikes->size() > strikeDisplayCount) {
                std::cout << " ... +" << (strikes->size() - strikeDisplayCount) << " more";
            }
            std::cout << "\n\n";
        }
        
        // Test 8: Historical Data
        std::cout << "Test 8: Getting historical data for AAPL..." << std::endl;
        auto history = marketService.getHistoricalData("AAPL", "daily", "2024-01-01", "2024-01-31");
        if (!history) {
            std::cerr << "Failed to get historical data" << std::endl;
            return 1;
        }
        printHistoricalData(*history);
        
        // Test 9: Time and Sales (limited time range to avoid huge data)
        std::cout << "Test 9: Getting time and sales data for SPY..." << std::endl;
        auto timeSales = marketService.getTimeSales("SPY", "5min", "2024-01-02 09:30", "2024-01-02 10:30");
        if (!timeSales) {
            std::cerr << "Failed to get time and sales data" << std::endl;
            return 1;
        }
        std::cout << "Found " << timeSales->size() << " time and sales data points:\n";
        size_t tsDisplayCount = std::min(size_t(5), timeSales->size());
        for (size_t i = 0; i < tsDisplayCount; ++i) {
            const auto& ts = (*timeSales)[i];
            std::cout << "  " << ts.time << ": $" << std::fixed << std::setprecision(2) << ts.price;
            std::cout << " (Vol: " << ts.volume << ", VWAP: $" << ts.vwap << ")\n";
        }
        if (timeSales->size() > tsDisplayCount) {
            std::cout << "  ... and " << (timeSales->size() - tsDisplayCount) << " more entries\n";
        }
        std::cout << std::endl;
        
        // Test 10: Symbol Search
        std::cout << "Test 10: Searching for 'apple' symbols..." << std::endl;
        auto searchResults = marketService.searchSymbols("apple", false);
        if (!searchResults) {
            std::cerr << "Failed to search symbols" << std::endl;
            return 1;
        }
        printSecurities(*searchResults, "Apple Search Results");
        
        // Test 11: Symbol Lookup
        std::cout << "Test 11: Looking up 'GOOG' symbols..." << std::endl;
        auto lookupResults = marketService.lookupSymbols("GOOG", "Q,N", "stock");
        if (!lookupResults) {
            std::cerr << "Failed to lookup symbols" << std::endl;
            return 1;
        }
        printSecurities(*lookupResults, "GOOG Lookup Results");
        
        // Test 12: Option Symbol Lookup
        std::cout << "Test 12: Looking up option symbols for AAPL..." << std::endl;
        auto optionSymbols = marketService.lookupOptionSymbols("AAPL");
        if (!optionSymbols) {
            std::cerr << "Failed to lookup option symbols" << std::endl;
            return 1;
        }
        std::cout << "Found " << optionSymbols->size() << " option symbol groups:\n";
        for (const auto& optSymbol : *optionSymbols) {
            std::cout << "  Root: " << optSymbol.rootSymbol << " (" << optSymbol.options.size() << " options)\n";
            if (!optSymbol.options.empty()) {
                std::cout << "    Examples: ";
                size_t optDisplayCount = std::min(size_t(3), optSymbol.options.size());
                for (size_t i = 0; i < optDisplayCount; ++i) {
                    if (i > 0) std::cout << ", ";
                    std::cout << optSymbol.options[i];
                }
                if (optSymbol.options.size() > optDisplayCount) {
                    std::cout << " ... +" << (optSymbol.options.size() - optDisplayCount) << " more";
                }
                std::cout << "\n";
            }
        }
        std::cout << std::endl;
        
        // Test 13: ETB List (Easy to Borrow securities)
        std::cout << "Test 13: Getting ETB (Easy to Borrow) list sample..." << std::endl;
        auto etbList = marketService.getETBList();
        if (!etbList) {
            std::cerr << "Failed to get ETB list" << std::endl;
            return 1;
        }
        printSecurities(*etbList, "ETB Securities Sample");
        
        // Test 14: Market Calendar
        std::cout << "Test 14: Getting market calendar for January 2024..." << std::endl;
        auto calendar = marketService.getCalendar("01", "2024");
        if (!calendar) {
            std::cerr << "Failed to get market calendar" << std::endl;
            return 1;
        }
        std::cout << "Market Calendar for " << calendar->month << "/" << calendar->year << ":\n";
        std::cout << "Found " << calendar->days.size() << " days. Sample:\n";
        size_t calDisplayCount = std::min(size_t(5), calendar->days.size());
        for (size_t i = 0; i < calDisplayCount; ++i) {
            const auto& day = calendar->days[i];
            std::cout << "  " << day.date << " - " << day.status << " (" << day.description << ")\n";
            if (day.status == "open") {
                std::cout << "    Market Hours: " << day.open.start << " - " << day.open.end << "\n";
            }
        }
        if (calendar->days.size() > calDisplayCount) {
            std::cout << "  ... and " << (calendar->days.size() - calDisplayCount) << " more days\n";
        }
        std::cout << std::endl;
        
        std::cout << "=== All Market Data Features Tested Successfully ===" << std::endl;
        std::cout << "\n📊 Test Summary:" << std::endl;
        std::cout << "✅ Market clock and status" << std::endl;
        std::cout << "✅ Single and multiple stock quotes" << std::endl;
        std::cout << "✅ GET and POST quote methods" << std::endl;
        std::cout << "✅ Option expirations and strikes" << std::endl;
        std::cout << "✅ Option chains" << std::endl;
        std::cout << "✅ Historical price data" << std::endl;
        std::cout << "✅ Time and sales data" << std::endl;
        std::cout << "✅ Symbol search and lookup" << std::endl;
        std::cout << "✅ Option symbol lookup" << std::endl;
        std::cout << "✅ ETB (Easy to Borrow) list" << std::endl;
        std::cout << "✅ Market calendar" << std::endl;
        
    } catch (const tradier::ValidationError& e) {
        std::cerr << "Validation Error: " << e.what() << std::endl;
        return 1;
    } catch (const tradier::ApiError& e) {
        std::cerr << "API Error: " << e.what() << std::endl;
        return 1;
    } catch (const tradier::ConnectionError& e) {
        std::cerr << "Connection Error: " << e.what() << std::endl;
        return 1;
    } catch (const tradier::AuthenticationError& e) {
        std::cerr << "Authentication Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}