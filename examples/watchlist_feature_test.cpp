/*
 * libtradier - Tradier API C++ Library v0.1.0
 *
 * Author: Benjamin Cance (kc8bws@kc8bws.com)
 * Date: 2025-05-22
 *
 * This software is provided free of charge under the MIT License.
 * By using it, you agree to absolve the author of all liability.
 * See LICENSE file for full terms and conditions.
 */

#include "tradier/client.hpp"
#include "tradier/watchlist.hpp"
#include "tradier/common/errors.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <random>
#include <sstream>

std::string generateRandomSuffix() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    return std::to_string(dis(gen));
}

void printWatchlistSummary(const std::vector<tradier::WatchlistSummary>& watchlists) {
    std::cout << "Found " << watchlists.size() << " watchlists:\n";
    for (const auto& wl : watchlists) {
        std::cout << "  - " << wl.name << " (ID: " << wl.id << ", Public ID: " << wl.publicId << ")\n";
    }
    std::cout << std::endl;
}

void printWatchlistDetails(const tradier::Watchlist& watchlist) {
    std::cout << "Watchlist Details:\n";
    std::cout << "  Name: " << watchlist.name << "\n";
    std::cout << "  ID: " << watchlist.id << "\n";
    std::cout << "  Public ID: " << watchlist.publicId << "\n";
    std::cout << "  Symbols (" << watchlist.items.size() << "):\n";
    for (const auto& item : watchlist.items) {
        std::cout << "    - " << item.symbol << " (ID: " << item.id << ")\n";
    }
    std::cout << std::endl;
}

int main() {
    try {
        auto config = tradier::Config::fromEnvironment();
        tradier::TradierClient client(config);
        auto watchlistService = client.watchlists();
        
        // Generate unique names for this test run
        std::string randomSuffix = generateRandomSuffix();
        std::string testWatchlistName = "Test Portfolio " + randomSuffix;
        std::string updatedWatchlistName = "Updated Test Portfolio " + randomSuffix;
        
        std::cout << "=== Tradier Watchlist Feature Test ===" << std::endl;
        std::cout << "Using " << (config.sandboxMode ? "SANDBOX" : "PRODUCTION") << " environment" << std::endl;
        std::cout << "Test ID: " << randomSuffix << "\n\n";
        
        // Step 1: Query initial watchlists
        std::cout << "Step 1: Querying existing watchlists..." << std::endl;
        auto initialWatchlists = watchlistService.getWatchlists();
        if (!initialWatchlists) {
            std::cerr << "Failed to retrieve initial watchlists" << std::endl;
            return 1;
        }
        printWatchlistSummary(*initialWatchlists);
        
        // Step 2: Create a new watchlist
        std::cout << "Step 2: Creating new watchlist '" << testWatchlistName << "'..." << std::endl;
        std::vector<std::string> initialSymbols = {"AAPL", "MSFT", "GOOGL"};
        auto newWatchlist = watchlistService.createWatchlist(testWatchlistName, initialSymbols);
        if (!newWatchlist) {
            std::cerr << "Failed to create new watchlist" << std::endl;
            return 1;
        }
        std::cout << "Successfully created watchlist!\n";
        printWatchlistDetails(*newWatchlist);
        
        std::string watchlistId = newWatchlist->id;
        
        // Step 3: Query all watchlists again to verify creation
        std::cout << "Step 3: Querying all watchlists after creation..." << std::endl;
        auto updatedWatchlists = watchlistService.getWatchlists();
        if (!updatedWatchlists) {
            std::cerr << "Failed to retrieve updated watchlists" << std::endl;
            return 1;
        }
        printWatchlistSummary(*updatedWatchlists);
        
        // Step 4: Get specific watchlist details
        std::cout << "Step 4: Getting detailed view of new watchlist..." << std::endl;
        auto specificWatchlist = watchlistService.getWatchlist(watchlistId);
        if (!specificWatchlist) {
            std::cerr << "Failed to retrieve specific watchlist" << std::endl;
            return 1;
        }
        printWatchlistDetails(*specificWatchlist);
        
        // Step 5: Add symbols to the watchlist
        std::cout << "Step 5: Adding symbols (TSLA, NVDA) to watchlist..." << std::endl;
        std::vector<std::string> symbolsToAdd = {"TSLA", "NVDA"};
        auto watchlistWithAddedSymbols = watchlistService.addSymbols(watchlistId, symbolsToAdd);
        if (!watchlistWithAddedSymbols) {
            std::cerr << "Failed to add symbols to watchlist" << std::endl;
            return 1;
        }
        std::cout << "Successfully added symbols!\n";
        printWatchlistDetails(*watchlistWithAddedSymbols);
        
        // Step 6: Remove a symbol from the watchlist
        std::cout << "Step 6: Removing symbol 'GOOGL' from watchlist..." << std::endl;
        auto watchlistAfterRemoval = watchlistService.removeSymbol(watchlistId, "GOOGL");
        if (!watchlistAfterRemoval) {
            std::cerr << "Failed to remove symbol from watchlist" << std::endl;
            return 1;
        }
        std::cout << "Successfully removed symbol!\n";
        printWatchlistDetails(*watchlistAfterRemoval);
        
        // Step 7: Update watchlist name and symbols
        std::cout << "Step 7: Updating watchlist name to '" << updatedWatchlistName << "' and replacing symbols..." << std::endl;
        std::vector<std::string> newSymbols = {"SPY", "QQQ", "IWM"};
        auto updatedWatchlist = watchlistService.updateWatchlist(watchlistId, updatedWatchlistName, newSymbols);
        if (!updatedWatchlist) {
            std::cerr << "Failed to update watchlist" << std::endl;
            return 1;
        }
        std::cout << "Successfully updated watchlist!\n";
        printWatchlistDetails(*updatedWatchlist);
        
        // Brief pause before cleanup
        std::cout << "Pausing before cleanup..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // Step 8: Delete the test watchlist
        std::cout << "Step 8: Deleting test watchlist..." << std::endl;
        auto remainingWatchlists = watchlistService.deleteWatchlist(watchlistId);
        if (!remainingWatchlists) {
            std::cerr << "Failed to delete watchlist" << std::endl;
            return 1;
        }
        std::cout << "Successfully deleted test watchlist!\n";
        printWatchlistSummary(*remainingWatchlists);
        
        std::cout << "=== All Watchlist Features Tested Successfully ===" << std::endl;
        std::cout << "Test ID " << randomSuffix << " completed successfully!" << std::endl;
        
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