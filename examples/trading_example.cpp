#include <iostream>
#include <iomanip>
#include "tradier/client.hpp"
#include "tradier/account.hpp"
#include "tradier/trading.hpp"
#include "tradier/common/errors.hpp"

int main() {
    try {
        auto config = tradier::Config::fromEnvironment();
        tradier::TradierClient client(config);
        
        auto accountService = client.accounts();
        auto tradingService = client.trading();
        
        auto profile = accountService.getProfile();
        if (!profile || profile->accounts.empty()) {
            std::cerr << "No accounts found" << std::endl;
            return 1;
        }
        
        const auto& account = profile->accounts[0];
        std::cout << "Using account: " << account.number << std::endl;
        
        std::cout << "\n=== Current Orders ===" << std::endl;
        auto orders = accountService.getOrders(account.number);
        if (orders) {
            std::cout << "Found " << orders->size() << " orders" << std::endl;
            for (const auto& order : *orders) {
                std::cout << "Order " << order.id << ": " << order.side << " " 
                          << order.quantity << " " << order.symbol 
                          << " @ $" << std::fixed << std::setprecision(2) << order.price 
                          << " (" << order.status << ")" << std::endl;
            }
        }
        
        std::cout << "\n=== Place Test Order ===" << std::endl;
        std::cout << "Placing a test buy order for 1 share of SPY..." << std::endl;
        
        auto orderResponse = tradingService.buyStock(account.number, "SPY", 1.0, 400.0);
        if (orderResponse) {
            std::cout << "Order placed successfully!" << std::endl;
            std::cout << "Order ID: " << orderResponse->id << std::endl;
            std::cout << "Status: " << orderResponse->status << std::endl;
            
            if (orderResponse->partnerId) {
                std::cout << "Partner ID: " << *orderResponse->partnerId << std::endl;
            }
            
            std::cout << "\nCancelling the test order..." << std::endl;
            auto cancelResponse = tradingService.cancelOrder(account.number, orderResponse->id);
            if (cancelResponse) {
                std::cout << "Order cancelled: " << cancelResponse->status << std::endl;
            }
        } else {
            std::cout << "Failed to place order" << std::endl;
        }
        
    } catch (const tradier::TradierException& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}