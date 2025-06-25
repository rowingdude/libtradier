/**
 * @file simple_trading.cpp
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
 *   ./simple_trading
 * 
 * @author libtradier
 * @version 1.0
 */

#include <tradier/client.hpp>
#include <tradier/trading.hpp>
#include <tradier/market.hpp>
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
    std::cout << "Side: " << preview.side << std::endl;
    std::cout << "Type: " << preview.type << std::endl;
    std::cout << "Duration: " << preview.duration << std::endl;
    std::cout << "Estimated Cost: $" << preview.cost << std::endl;
    std::cout << "Commission: $" << preview.commission << std::endl;
    std::cout << "Fees: $" << preview.fees << std::endl;
    std::cout << "Margin Change: $" << preview.margin_change << std::endl;
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
    if (!order.partner_id.empty()) {
        std::cout << "Partner ID: " << order.partner_id << std::endl;
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
    std::cout << "Duration: " << order.duration << std::endl;
    
    if (order.price > 0) {
        std::cout << "Price: $" << order.price << std::endl;
    }
    if (order.avg_fill_price > 0) {
        std::cout << "Average Fill Price: $" << order.avg_fill_price << std::endl;
    }
    if (order.exec_quantity > 0) {
        std::cout << "Executed Quantity: " << order.exec_quantity << std::endl;
    }
    if (order.remaining_quantity > 0) {
        std::cout << "Remaining Quantity: " << order.remaining_quantity << std::endl;
    }
    
    std::cout << "Class: " << order.order_class << std::endl;
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
 * @brief Get current quote for a symbol
 * @param marketService Reference to market service
 * @param symbol Symbol to quote
 * @return Current quote or empty optional if failed
 */
std::optional<Quote> getCurrentQuote(MarketService& marketService, const std::string& symbol) {
    auto quotesResult = marketService.getQuotes({symbol});
    if (quotesResult.isSuccess() && !quotesResult.value().empty()) {
        return quotesResult.value()[0];
    }
    return std::nullopt;
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
        auto marketService = client.market();
        
        std::cout << "✓ Trading service initialized" << std::endl;
        
        // 3. Example trading parameters
        const std::string symbol = "AAPL";
        const int quantity = 1; // Small quantity for example
        
        // 4. Get current market data
        std::cout << "\n--- Getting Current Market Data ---" << std::endl;
        auto quote = getCurrentQuote(marketService, symbol);
        
        if (!quote) {
            std::cerr << "✗ Failed to get quote for " << symbol << std::endl;
            std::cerr << "Cannot proceed with trading example." << std::endl;
            return 1;
        }
        
        std::cout << "✓ Current quote for " << symbol << ":" << std::endl;
        std::cout << "  Last: $" << std::fixed << std::setprecision(2) << quote->last << std::endl;
        std::cout << "  Bid: $" << quote->bid << " | Ask: $" << quote->ask << std::endl;
        
        // 5. Create and preview a market buy order
        std::cout << "\n--- Market Order Preview ---" << std::endl;
        OrderRequest marketOrder;
        marketOrder.accountId = config.accountNumber;
        marketOrder.symbol = symbol;
        marketOrder.side = OrderSide::BUY;
        marketOrder.quantity = quantity;
        marketOrder.type = OrderType::MARKET;
        marketOrder.duration = OrderDuration::DAY;
        
        std::cout << "Previewing market buy order for " << quantity << " shares of " << symbol << "..." << std::endl;
        
        auto previewResult = tradingService.previewOrder(marketOrder);
        if (previewResult.isSuccess()) {
            std::cout << "✓ Order preview successful" << std::endl;
            printOrderPreview(previewResult.value());
            
            // Ask user if they want to proceed (in sandbox this is safe)
            if (getUserConfirmation("Proceed with placing this market order? (Sandbox only)")) {
                
                // 6. Place the market order
                std::cout << "\n--- Placing Market Order ---" << std::endl;
                auto orderResult = tradingService.placeOrder(marketOrder);
                
                if (orderResult.isSuccess()) {
                    std::cout << "✓ Market order placed successfully!" << std::endl;
                    auto placedOrder = orderResult.value();
                    printOrderConfirmation(placedOrder);
                    
                    // 7. Monitor order status
                    std::cout << "\n--- Monitoring Order Status ---" << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(2)); // Brief delay
                    
                    auto statusResult = tradingService.getOrder(config.accountNumber, placedOrder.id);
                    if (statusResult.isSuccess()) {
                        std::cout << "✓ Order status retrieved" << std::endl;
                        printOrderStatus(statusResult.value());
                        
                        // 8. Demonstrate order cancellation (if still pending)
                        auto currentOrder = statusResult.value();
                        if (currentOrder.status == "open" || currentOrder.status == "pending") {
                            if (getUserConfirmation("Order is still pending. Cancel it?")) {
                                std::cout << "\n--- Cancelling Order ---" << std::endl;
                                auto cancelResult = tradingService.cancelOrder(config.accountNumber, placedOrder.id);
                                
                                if (cancelResult.isSuccess()) {
                                    std::cout << "✓ Order cancelled successfully" << std::endl;
                                } else {
                                    std::cerr << "✗ Failed to cancel order: " << cancelResult.error().message << std::endl;
                                }
                            }
                        } else {
                            std::cout << "Order has been " << currentOrder.status << " and cannot be cancelled." << std::endl;
                        }
                    } else {
                        std::cerr << "✗ Failed to get order status: " << statusResult.error().message << std::endl;
                    }
                    
                } else {
                    std::cerr << "✗ Failed to place order: " << orderResult.error().message << std::endl;
                }
            } else {
                std::cout << "Order placement cancelled by user." << std::endl;
            }
            
        } else {
            std::cerr << "✗ Order preview failed: " << previewResult.error().message << std::endl;
        }
        
        // 9. Demonstrate limit order creation (preview only)
        std::cout << "\n--- Limit Order Example (Preview Only) ---" << std::endl;
        OrderRequest limitOrder;
        limitOrder.accountId = config.accountNumber;
        limitOrder.symbol = symbol;
        limitOrder.side = OrderSide::BUY;
        limitOrder.quantity = quantity;
        limitOrder.type = OrderType::LIMIT;
        limitOrder.duration = OrderDuration::DAY;
        limitOrder.price = quote->bid - 1.00; // Limit price below current bid
        
        std::cout << "Previewing limit buy order for " << quantity << " shares of " << symbol 
                  << " at $" << std::fixed << std::setprecision(2) << limitOrder.price << "..." << std::endl;
        
        auto limitPreviewResult = tradingService.previewOrder(limitOrder);
        if (limitPreviewResult.isSuccess()) {
            std::cout << "✓ Limit order preview successful" << std::endl;
            printOrderPreview(limitPreviewResult.value());
        } else {
            std::cerr << "✗ Limit order preview failed: " << limitPreviewResult.error().message << std::endl;
        }
        
        std::cout << "\n=== Trading Example completed successfully ===" << std::endl;
        std::cout << "\nKey takeaways:" << std::endl;
        std::cout << "- Always preview orders before placing them" << std::endl;
        std::cout << "- Monitor order status after placement" << std::endl;
        std::cout << "- Use appropriate order types for your strategy" << std::endl;
        std::cout << "- Test thoroughly in sandbox before production use" << std::endl;
        std::cout << "\nNext steps:" << std::endl;
        std::cout << "- Explore options trading examples" << std::endl;
        std::cout << "- Learn about stop-loss and bracket orders" << std::endl;
        std::cout << "- Study the advanced trading bot example" << std::endl;
        
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

/*
Expected Output:
================

=== libtradier Simple Trading Example ===
This example demonstrates basic trading operations in SANDBOX mode.

⚠️  SAFETY NOTICE: This example uses sandbox mode only.
    No real money or securities will be traded.

✓ Configuration loaded
  Environment: Sandbox 🔒
  Account: 123456789

✓ Trading service initialized

--- Getting Current Market Data ---
✓ Current quote for AAPL:
  Last: $150.25
  Bid: $150.20 | Ask: $150.30

--- Market Order Preview ---
Previewing market buy order for 1 shares of AAPL...
✓ Order preview successful

=== Order Preview ===
Status: ok
Symbol: AAPL
Quantity: 1
Side: buy
Type: market
Duration: day
Estimated Cost: $150.30
Commission: $0.00
Fees: $0.00
Margin Change: $75.15
Order Valid: Yes

Proceed with placing this market order? (Sandbox only) (y/N): y

--- Placing Market Order ---
✓ Market order placed successfully!

=== Order Confirmation ===
Order ID: 12345
Status: submitted
Partner ID: sandbox-partner

--- Monitoring Order Status ---
✓ Order status retrieved

=== Order Status ===
Order ID: 12345
Symbol: AAPL
Type: market
Side: buy
Quantity: 1.0000
Status: filled
Duration: day
Average Fill Price: $150.25
Executed Quantity: 1.0000
Remaining Quantity: 0.0000
Class: equity

Order has been filled and cannot be cancelled.

--- Limit Order Example (Preview Only) ---
Previewing limit buy order for 1 shares of AAPL at $149.20...
✓ Limit order preview successful

=== Order Preview ===
Status: ok
Symbol: AAPL
Quantity: 1
Side: buy
Type: limit
Duration: day
Estimated Cost: $149.20
Commission: $0.00
Fees: $0.00
Margin Change: $74.60
Order Valid: Yes

=== Trading Example completed successfully ===

Key takeaways:
- Always preview orders before placing them
- Monitor order status after placement
- Use appropriate order types for your strategy
- Test thoroughly in sandbox before production use

Next steps:
- Explore options trading examples
- Learn about stop-loss and bracket orders
- Study the advanced trading bot example
*/