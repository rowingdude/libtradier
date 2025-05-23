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

#include <iostream>
#include <iomanip>
#include "tradier/client.hpp"
#include "tradier/account.hpp"
#include "tradier/market.hpp"
#include "tradier/common/errors.hpp"

void printSeparator(const std::string& title) {
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << title << std::endl;
    std::cout << std::string(50, '=') << std::endl;
}

int main() {
    try {
        auto config = tradier::Config::fromEnvironment();
        std::cout << "Tradier Client v" << tradier::TradierClient::version() << std::endl;
        std::cout << "Environment: " << (config.sandboxMode ? "Sandbox" : "Production") << std::endl;
        
        tradier::TradierClient client(config);
        
        printSeparator("Account Information");
        auto accountService = client.accounts();
        auto profile = accountService.getProfile();
        
        if (profile) {
            std::cout << "User ID: " << profile->id << std::endl;
            std::cout << "Name: " << profile->name << std::endl;
            std::cout << "Accounts: " << profile->accounts.size() << std::endl;
            
            for (const auto& account : profile->accounts) {
                std::cout << "\n  Account: " << account.number << std::endl;
                std::cout << "  Type: " << account.type << std::endl;
                std::cout << "  Status: " << account.status << std::endl;
                std::cout << "  Day Trader: " << (account.dayTrader ? "Yes" : "No") << std::endl;
                
                auto balances = accountService.getBalances(account.number);
                if (balances) {
                    std::cout << "  Total Equity: $" << std::fixed << std::setprecision(2) 
                              << balances->totalEquity << std::endl;
                    std::cout << "  Buying Power: $" << std::fixed << std::setprecision(2) 
                              << balances->buyingPower << std::endl;
                }
                
                auto positions = accountService.getPositions(account.number);
                if (positions && !positions->empty()) {
                    std::cout << "  Positions: " << positions->size() << std::endl;
                    for (const auto& pos : *positions) {
                        std::cout << "    " << pos.symbol << ": " << pos.quantity 
                                  << " @ $" << std::fixed << std::setprecision(2) 
                                  << pos.costBasis << std::endl;
                    }
                }
            }
        } else {
            std::cout << "Failed to retrieve account profile" << std::endl;
        }
        
        printSeparator("Market Data");
        auto marketService = client.market();
        
        auto spyQuote = marketService.getQuote("SPY");
        if (spyQuote) {
            std::cout << "SPY Quote:" << std::endl;
            std::cout << "  Bid: $" << std::fixed << std::setprecision(2) << spyQuote->bid << std::endl;
            std::cout << "  Ask: $" << std::fixed << std::setprecision(2) << spyQuote->ask << std::endl;
            std::cout << "  Last: $" << std::fixed << std::setprecision(2) << spyQuote->last << std::endl;
        }
        
        auto clock = marketService.getClock();
        if (clock) {
            std::cout << "Market Status: " << clock->state << std::endl;
            std::cout << "Description: " << clock->description << std::endl;
        }
        
    } catch (const tradier::TradierException& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}