/**
 * @file simple_trading_fixed.cpp
 * @brief Basic trading operations example with comprehensive safety checks
 * 
 * This example demonstrates:
 * 1. Order preview functionality (risk-free validation)
 * 2. Market and limit order placement
 * 3. Order status monitoring
 * 4. Order modification and cancellation
 * 5. Trading safety best practices
 * 6. Sandbox environment usage
 * 
 * **IMPORTANT SAFETY NOTICE:**
 * This example uses SANDBOX mode by default. No real trades will be executed.
 * Always test thoroughly in sandbox before using production credentials.
 * 
 * Usage:
 *   export TRADIER_ACCESS_TOKEN="your-sandbox-token"
 *   export TRADIER_ACCOUNT_NUMBER="your-sandbox-account"
 *   ./simple_trading_fixed
 * 
 * @author libtradier
 * @version 1.0
 */

#include <tradier/client.hpp>
#include <tradier/trading.hpp>
#include <tradier/account.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <thread>
#include <chrono>

using namespace tradier;

/**
 * @brief Print order preview information
 * @param preview The order preview to display
 */
void printOrderPreview(const OrderPreview& preview) {
    std::cout << "\n=== Order Preview ===" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Status: " << preview.status << std::endl;
    std::cout << "Symbol: " << preview.symbol << std::endl;
    std::cout << "Quantity: " << preview.quantity << std::endl;
    std::cout << "Side: " << (preview.side == OrderSide::BUY ? "BUY" : "SELL") << std::endl;
    std::cout << "Type: " << (preview.type == OrderType::MARKET ? "MARKET" : "LIMIT") << std::endl;
    std::cout << "Duration: " << (preview.duration == OrderDuration::DAY ? "DAY" : "GTC") << std::endl;
    std::cout << "Estimated Cost: $" << preview.cost << std::endl;
    std::cout << "Commission: $" << preview.commission << std::endl;
    std::cout << "Fees: $" << preview.fees << std::endl;
    std::cout << "Margin Change: $" << preview.marginChange << std::endl;
    std::cout << "Order Valid: " << (preview.result ? "Yes" : "No") << std::endl;
}

/**
 * @brief Print order confirmation details
 * @param order The order response to display
 */
void printOrderConfirmation(const OrderResponse& order) {
    std::cout << "\n=== Order Confirmation ===" << std::endl;
    std::cout << "Order ID: " << order.id << std::endl;
    std::cout << "Status: " << order.status << std::endl;
    if (order.partnerId) {
        std::cout << "Partner ID: " << *order.partnerId << std::endl;
    }
}

/**
 * @brief Print order status details
 * @param order The order details to display
 */
void printOrderStatus(const Order& order) {
    std::cout << "\n=== Order Status ===" << std::endl;
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Order ID: " << order.id << std::endl;
    std::cout << "Symbol: " << order.symbol << std::endl;
    std::cout << "Type: " << order.type << std::endl;
    std::cout << "Side: " << order.side << std::endl;
    std::cout << "Quantity: " << order.quantity << std::endl;
    std::cout << "Status: " << order.status << std::endl;
    
    if (order.price > 0) {
        std::cout << "Price: $" << order.price << std::endl;
    }
    if (order.filled > 0) {
        std::cout << "Filled Quantity: " << order.filled << std::endl;
    }
    
    if (order.tag) {
        std::cout << "Tag: " << *order.tag << std::endl;
    }
}

/**
 * @brief Get configuration from environment with validation
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
    
    const char* accountNum = std::getenv("TRADIER_ACCOUNT_NUMBER");
    if (!accountNum) {
        throw std::runtime_error(
            "TRADIER_ACCOUNT_NUMBER environment variable not set.\n"
            "This is required for trading operations."
        );
    }
    
    config.accessToken = token;
    config.accountNumber = accountNum;
    config.sandboxMode = true; // ALWAYS use sandbox for examples
    config.timeoutSeconds = 30;
    
    return config;
}

/**
 * @brief Wait for user confirmation before proceeding
 * @param prompt Message to display to user
 * @return true if user confirms, false otherwise
 */
bool getUserConfirmation(const std::string& prompt) {
    std::cout << "\n" << prompt << " (y/N): ";
    std::string response;
    std::getline(std::cin, response);
    return (response == "y" || response == "Y" || response == "yes" || response == "Yes");
}

/**
 * @brief Main function demonstrating trading operations
 */
int main() {
    try {
        std::cout << "=== libtradier Simple Trading Example ===" << std::endl;
        std::cout << "This example demonstrates basic trading operations in SANDBOX mode." << std::endl;
        std::cout << "\n⚠️  SAFETY NOTICE: This example uses sandbox mode only." << std::endl;
        std::cout << "    No real money or securities will be traded." << std::endl;
        
        // 1. Configure client
        auto config = getConfigFromEnvironment();
        std::cout << "\n✓ Configuration loaded" << std::endl;
        std::cout << "  Environment: " << (config.sandboxMode ? "Sandbox 🔒" : "Production ⚠️") << std::endl;
        std::cout << "  Account: " << config.accountNumber << std::endl;
        
        // 2. Create client and services
        TradierClient client(config);
        auto tradingService = client.trading();
        auto accountService = client.accounts();
        
        std::cout << "✓ Trading service initialized" << std::endl;
        
        // 3. Example trading parameters
        const std::string symbol = "AAPL";
        const int quantity = 1; // Small quantity for example
        
        // 4. Create and preview a market buy order - CORRECTED: Proper OrderRequest setup
        std::cout << "\n--- Market Order Preview ---" << std::endl;
        OrderRequest marketOrder;
        marketOrder.symbol = symbol;
        marketOrder.side = OrderSide::BUY;
        marketOrder.quantity = quantity;
        marketOrder.type = OrderType::MARKET;
        marketOrder.duration = OrderDuration::DAY;
        // Note: price is optional for market orders, so we don't set it
        
        std::cout << "Previewing market buy order for " << quantity << " shares of " << symbol << "..." << std::endl;
        
        // CORRECTED: Pass account number as first parameter
        auto previewResult = tradingService.previewOrder(config.accountNumber, marketOrder);
        if (previewResult.isSuccess()) {
            std::cout << "✓ Order preview successful" << std::endl;
            printOrderPreview(previewResult.value());
            
            // Ask user if they want to proceed (in sandbox this is safe)
            if (getUserConfirmation("Proceed with placing this market order? (Sandbox only)")) {
                
                // 5. Place the market order - CORRECTED: Pass account number
                std::cout << "\n--- Placing Market Order ---" << std::endl;
                auto orderResult = tradingService.placeOrder(config.accountNumber, marketOrder);
                
                if (orderResult.isSuccess()) {
                    std::cout << "✓ Market order placed successfully!" << std::endl;
                    auto placedOrder = orderResult.value();
                    printOrderConfirmation(placedOrder);
                    
                    // 6. Monitor order status
                    std::cout << "\n--- Monitoring Order Status ---" << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(2)); // Brief delay
                    
                    auto statusResult = accountService.getOrder(config.accountNumber, placedOrder.id);
                    if (statusResult.isSuccess()) {
                        std::cout << "✓ Order status retrieved" << std::endl;
                        printOrderStatus(statusResult.value());
                        
                        // 7. Demonstrate order cancellation (if still pending)
                        auto currentOrder = statusResult.value();
                        if (currentOrder.status == "open" || currentOrder.status == "pending") {
                            if (getUserConfirmation("Order is still pending. Cancel it?")) {
                                std::cout << "\n--- Cancelling Order ---" << std::endl;
                                auto cancelResult = tradingService.cancelOrder(config.accountNumber, placedOrder.id);
                                
                                if (cancelResult.isSuccess()) {
                                    std::cout << "✓ Order cancelled successfully" << std::endl;
                                } else {
                                    std::cerr << "✗ Failed to cancel order: " << cancelResult.error().what() << std::endl;
                                }
                            }
                        } else {
                            std::cout << "Order has been " << currentOrder.status << " and cannot be cancelled." << std::endl;
                        }
                    } else {
                        std::cerr << "✗ Failed to get order status: " << statusResult.error().what() << std::endl;
                    }
                    
                } else {
                    std::cerr << "✗ Failed to place order: " << orderResult.error().what() << std::endl;
                }
            } else {
                std::cout << "Order placement cancelled by user." << std::endl;
            }
            
        } else {
            std::cerr << "✗ Order preview failed: " << previewResult.error().what() << std::endl;
        }
        
        // 8. Demonstrate limit order creation (preview only) - CORRECTED: Use optional price
        std::cout << "\n--- Limit Order Example (Preview Only) ---" << std::endl;
        OrderRequest limitOrder;
        limitOrder.symbol = symbol;
        limitOrder.side = OrderSide::BUY;
        limitOrder.quantity = quantity;
        limitOrder.type = OrderType::LIMIT;
        limitOrder.duration = OrderDuration::DAY;
        limitOrder.price = 149.00; // CORRECTED: std::optional assignment
        
        std::cout << "Previewing limit buy order for " << quantity << " shares of " << symbol 
                  << " at $" << std::fixed << std::setprecision(2) << *limitOrder.price << "..." << std::endl;
        
        auto limitPreviewResult = tradingService.previewOrder(config.accountNumber, limitOrder);
        if (limitPreviewResult.isSuccess()) {
            std::cout << "✓ Limit order preview successful" << std::endl;
            printOrderPreview(limitPreviewResult.value());
        } else {
            std::cerr << "✗ Limit order preview failed: " << limitPreviewResult.error().what() << std::endl;
        }
        
        std::cout << "\n=== Trading Example completed successfully ===" << std::endl;
        std::cout << "\nKey takeaways:" << std::endl;
        std::cout << "- Always preview orders before placing them" << std::endl;
        std::cout << "- Monitor order status after placement" << std::endl;
        std::cout << "- Use appropriate order types for your strategy" << std::endl;
        std::cout << "- Test thoroughly in sandbox before production use" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Error: " << e.what() << std::endl;
        std::cerr << "\nTroubleshooting:" << std::endl;
        std::cerr << "1. Ensure both TRADIER_ACCESS_TOKEN and TRADIER_ACCOUNT_NUMBER are set" << std::endl;
        std::cerr << "2. Verify your sandbox credentials are valid" << std::endl;
        std::cerr << "3. Check that the account has sufficient buying power" << std::endl;
        std::cerr << "4. Ensure you're using a valid, tradeable symbol" << std::endl;
        return 1;
    }
}