/**
 * @file account_info_fixed.cpp
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
 *   export TRADIER_SANDBOX_KEY="your-sandbox-token"
 *   export TRADIER_SANDBOX_ACCT="your-sandbox-account"
 *   ./account_info_fixed
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
    std::cout << "Account Number: " << balances.accountNumber << std::endl;
    std::cout << "Account Type: " << balances.accountType << std::endl;
    std::cout << "Total Equity: $" << balances.totalEquity << std::endl;
    std::cout << "Total Cash: $" << balances.totalCash << std::endl;
    std::cout << "Market Value: $" << balances.marketValue << std::endl;
    std::cout << "Buying Power: $" << balances.buyingPower << std::endl;
    std::cout << "Day Change: $" << balances.dayChange << std::endl;
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
              << std::setw(15) << "Cost Basis" << std::endl;
    std::cout << std::string(35, '-') << std::endl;
    
    for (const auto& position : positions) {
        std::cout << std::left << std::setw(8) << position.symbol
                  << std::setw(12) << position.quantity  
                  << std::setw(15) << ("$" + std::to_string(position.costBasis)) << std::endl;
    }
}

/**
 * @brief Get configuration from environment variables
 * @return Configured Config object
 */
Config getConfigFromEnvironment() {
    Config config;
    
    // Try sandbox first (safer for examples)
    const char* token = std::getenv("TRADIER_SANDBOX_KEY");
    bool usingSandbox = (token != nullptr);
    
    if (!token) {
        // Fall back to production if sandbox not available
        token = std::getenv("TRADIER_PRODUCTION_KEY");
        usingSandbox = false;
    }
    
    if (!token) {
        throw std::runtime_error(
            "Neither TRADIER_SANDBOX_KEY nor TRADIER_PRODUCTION_KEY environment variables are set.\n"
            "Set TRADIER_SANDBOX_KEY for safe testing or TRADIER_PRODUCTION_KEY for live trading.\n"
            "Get your tokens from: https://documentation.tradier.com/getting-started"
        );
    }
    
    config.accessToken = token;
    config.sandboxMode = usingSandbox;
    config.timeoutSeconds = 30;
    
    // Get account number from environment
    const char* accountNum = nullptr;
    if (usingSandbox) {
        accountNum = std::getenv("TRADIER_SANDBOX_ACCT");
    } else {
        // Could add TRADIER_PRODUCTION_ACCT if needed
        accountNum = std::getenv("TRADIER_PRODUCTION_ACCT");
    }
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
        
        // 3. Create account service - CORRECTED: Use client.accounts()
        auto accountService = client.accounts();
        std::cout << "✓ Account service initialized" << std::endl;
        
        // 4. Get user profile
        std::cout << "\n--- Retrieving User Profile ---" << std::endl;
        auto profileResult = accountService.getProfile();
        
        if (profileResult.isSuccess()) {
            std::cout << "✓ Profile retrieved successfully" << std::endl;
            printAccountProfile(profileResult.value());
        } else {
            std::cerr << "✗ Failed to get profile: " << profileResult.error().what() << std::endl;
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
                std::cerr << "✗ Failed to get balances: " << balancesResult.error().what() << std::endl;
            }
            
            // 6. Get current positions
            std::cout << "\n--- Retrieving Current Positions ---" << std::endl;
            auto positionsResult = accountService.getPositions(accountId);
            
            if (positionsResult.isSuccess()) {
                std::cout << "✓ Positions retrieved successfully" << std::endl;
                printPositions(positionsResult.value());
            } else {
                std::cerr << "✗ Failed to get positions: " << positionsResult.error().what() << std::endl;
            }
        } else {
            std::cout << "\n! No account number available for balance/position queries" << std::endl;
        }
        
        std::cout << "\n=== Example completed successfully ===" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Error: " << e.what() << std::endl;
        std::cerr << "\nTroubleshooting:" << std::endl;
        std::cerr << "1. Ensure TRADIER_SANDBOX_KEY/TRADIER_SANDBOX_ACCT or TRADIER_PRODUCTION_KEY are set" << std::endl;
        std::cerr << "2. Check your internet connection" << std::endl;
        std::cerr << "3. Verify the Tradier sandbox API is accessible" << std::endl;
        return 1;
    }
}