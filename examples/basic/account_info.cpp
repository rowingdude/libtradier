/**
 * @file account_info.cpp
 * @brief Basic example demonstrating account information retrieval
 * 
 * This example shows how to:
 * 1. Configure the Tradier client for sandbox mode
 * 2. Retrieve user profile and account details
 * 3. Get account balances and buying power
 * 4. List current positions
 * 5. Handle errors gracefully
 * 
 * Usage:
 *   export TRADIER_ACCESS_TOKEN="your-sandbox-token"
 *   ./account_info
 * 
 * @author libtradier
 * @version 1.0
 */

#include <tradier/client.hpp>
#include <tradier/account.hpp>
#include <iostream>
#include <iomanip>
#include <cstdlib>

using namespace tradier;

/**
 * @brief Print account profile information
 * @param profile The account profile to display
 */
void printAccountProfile(const AccountProfile& profile) {
    std::cout << "\n=== Account Profile ===" << std::endl;
    std::cout << "User ID: " << profile.id << std::endl;
    std::cout << "Name: " << profile.name << std::endl;
    
    for (const auto& account : profile.accounts) {
        std::cout << "\nAccount: " << account.number << std::endl;
        std::cout << "  Type: " << account.type << std::endl;
        std::cout << "  Status: " << account.status << std::endl;
        std::cout << "  Classification: " << account.classification << std::endl;
        std::cout << "  Day Trader: " << (account.dayTrader ? "Yes" : "No") << std::endl;
        std::cout << "  Option Level: " << account.optionLevel << std::endl;
    }
}

/**
 * @brief Print account balances and buying power
 * @param balances The account balances to display
 */
void printAccountBalances(const AccountBalances& balances) {
    std::cout << "\n=== Account Balances ===" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Total Equity: $" << balances.total_equity << std::endl;
    std::cout << "Total Cash: $" << balances.total_cash << std::endl;
    std::cout << "Market Value: $" << balances.market_value << std::endl;
    std::cout << "Open P&L: $" << balances.open_pl << std::endl;
    std::cout << "Close P&L: $" << balances.close_pl << std::endl;
    
    std::cout << "\n--- Buying Power ---" << std::endl;
    std::cout << "Stock Buying Power: $" << balances.stock_buying_power << std::endl;
    std::cout << "Option Buying Power: $" << balances.option_buying_power << std::endl;
    std::cout << "Day Trade Buying Power: $" << balances.day_trade_buying_power << std::endl;
}

/**
 * @brief Print current positions
 * @param positions Vector of current positions
 */
void printPositions(const std::vector<Position>& positions) {
    std::cout << "\n=== Current Positions ===" << std::endl;
    
    if (positions.empty()) {
        std::cout << "No positions found." << std::endl;
        return;
    }
    
    std::cout << std::fixed << std::setprecision(2);
    std::cout << std::left << std::setw(8) << "Symbol" 
              << std::setw(12) << "Quantity" 
              << std::setw(15) << "Cost Basis"
              << "Date Acquired" << std::endl;
    std::cout << std::string(50, '-') << std::endl;
    
    for (const auto& position : positions) {
        std::cout << std::left << std::setw(8) << position.symbol
                  << std::setw(12) << position.quantity  
                  << std::setw(15) << ("$" + std::to_string(position.costBasis))
                  << "TBD" << std::endl; // Date formatting would need utilities
    }
}

/**
 * @brief Get configuration from environment variables
 * @return Configured Config object
 */
Config getConfigFromEnvironment() {
    Config config;
    
    // Get access token from environment
    const char* token = std::getenv("TRADIER_ACCESS_TOKEN");
    if (!token) {
        throw std::runtime_error(
            "TRADIER_ACCESS_TOKEN environment variable not set.\n"
            "Get your sandbox token from: https://documentation.tradier.com/getting-started"
        );
    }
    
    config.accessToken = token;
    config.sandboxMode = true;  // Use sandbox for examples
    config.timeoutSeconds = 30;
    
    // Optional: Override with account number if provided
    const char* accountNum = std::getenv("TRADIER_ACCOUNT_NUMBER");
    if (accountNum) {
        config.accountNumber = accountNum;
    }
    
    return config;
}

/**
 * @brief Main function demonstrating account information retrieval
 */
int main() {
    try {
        std::cout << "=== libtradier Account Information Example ===" << std::endl;
        std::cout << "This example demonstrates retrieving account information." << std::endl;
        
        // 1. Configure client
        auto config = getConfigFromEnvironment();
        std::cout << "\n✓ Configuration loaded" << std::endl;
        std::cout << "  Environment: " << (config.sandboxMode ? "Sandbox" : "Production") << std::endl;
        std::cout << "  Base URL: " << config.baseUrl() << std::endl;
        
        // 2. Create client  
        TradierClient client(config);
        std::cout << "✓ Tradier client created" << std::endl;
        std::cout << "  Authenticated: " << (client.isAuthenticated() ? "Yes" : "No") << std::endl;
        
        // 3. Create account service
        auto accountService = client.accounts();
        std::cout << "✓ Account service initialized" << std::endl;
        
        // 4. Get user profile
        std::cout << "\n--- Retrieving User Profile ---" << std::endl;
        auto profileResult = accountService.getProfile();
        
        if (profileResult.isSuccess()) {
            std::cout << "✓ Profile retrieved successfully" << std::endl;
            printAccountProfile(profileResult.value());
        } else {
            std::cerr << "✗ Failed to get profile: " << profileResult.error().message << std::endl;
            return 1;
        }
        
        // 5. Get account balances (use first account if no default set)
        std::string accountId = config.accountNumber;
        if (accountId.empty() && !profileResult.value().accounts.empty()) {
            accountId = profileResult.value().accounts[0].number;
        }
        
        if (!accountId.empty()) {
            std::cout << "\n--- Retrieving Account Balances ---" << std::endl;
            auto balancesResult = accountService.getBalances(accountId);
            
            if (balancesResult.isSuccess()) {
                std::cout << "✓ Balances retrieved successfully" << std::endl;
                printAccountBalances(balancesResult.value());
            } else {
                std::cerr << "✗ Failed to get balances: " << balancesResult.error().message << std::endl;
            }
            
            // 6. Get current positions
            std::cout << "\n--- Retrieving Current Positions ---" << std::endl;
            auto positionsResult = accountService.getPositions(accountId);
            
            if (positionsResult.isSuccess()) {
                std::cout << "✓ Positions retrieved successfully" << std::endl;
                printPositions(positionsResult.value());
            } else {
                std::cerr << "✗ Failed to get positions: " << positionsResult.error().message << std::endl;
            }
        } else {
            std::cout << "\n! No account number available for balance/position queries" << std::endl;
        }
        
        std::cout << "\n=== Example completed successfully ===" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Error: " << e.what() << std::endl;
        std::cerr << "\nTroubleshooting:" << std::endl;
        std::cerr << "1. Ensure TRADIER_ACCESS_TOKEN is set with a valid sandbox token" << std::endl;
        std::cerr << "2. Check your internet connection" << std::endl;
        std::cerr << "3. Verify the Tradier sandbox API is accessible" << std::endl;
        return 1;
    }
}

/* 
Expected Output:
================

=== libtradier Account Information Example ===
This example demonstrates retrieving account information.

✓ Configuration loaded
  Environment: Sandbox
  Base URL: https://sandbox.tradier.com/v1

✓ Tradier client created
  Authenticated: Yes

✓ Account service initialized

--- Retrieving User Profile ---
✓ Profile retrieved successfully

=== Account Profile ===
User ID: user123
Name: John Doe

Account: 123456789
  Type: margin
  Status: active
  Classification: individual
  Day Trader: No
  Option Level: 2

--- Retrieving Account Balances ---
✓ Balances retrieved successfully

=== Account Balances ===
Total Equity: $50000.00
Total Cash: $25000.00
Market Value: $25000.00
Open P&L: $1250.00
Close P&L: $0.00

--- Buying Power ---
Stock Buying Power: $50000.00
Option Buying Power: $25000.00
Day Trade Buying Power: $100000.00

--- Retrieving Current Positions ---
✓ Positions retrieved successfully

=== Current Positions ===
Symbol   Quantity     Cost Basis     Date Acquired
--------------------------------------------------
AAPL     100          $15000.00      TBD
MSFT     50           $10000.00      TBD

=== Example completed successfully ===
*/