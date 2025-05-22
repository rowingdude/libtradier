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


#pragma once

#include <optional>
#include "tradier/common/types.hpp"
#include "tradier/data.hpp"

namespace tradier {

class TradierClient;

enum class OrderType { MARKET, LIMIT, STOP, STOP_LIMIT };
enum class OrderSide { BUY, SELL, BUY_TO_OPEN, BUY_TO_CLOSE, SELL_TO_OPEN, SELL_TO_CLOSE };
enum class OrderDuration { DAY, GTC, PRE, POST };

struct OrderRequest {
    std::string symbol;
    OrderSide side;
    double quantity = 0.0;
    OrderType type;
    OrderDuration duration = OrderDuration::DAY;
    std::optional<double> price;
    std::optional<double> stop;
    std::optional<std::string> optionSymbol;
    std::optional<std::string> tag;
};

struct OrderResponse {
    int id = 0;
    std::string status;
    std::optional<std::string> partnerId;
};

class TradingService {
private:
    TradierClient& client_;
    
    std::string toString(OrderType type) const;
    std::string toString(OrderSide side) const;
    std::string toString(OrderDuration duration) const;
    FormParams buildParams(const OrderRequest& request) const;
    
public:
    explicit TradingService(TradierClient& client) : client_(client) {}
    
    Result<OrderResponse> placeOrder(const std::string& account, const OrderRequest& request);
    Result<OrderResponse> cancelOrder(const std::string& account, int orderId);
    Result<OrderResponse> modifyOrder(const std::string& account, int orderId, const OrderRequest& changes);
    
    Result<OrderResponse> buyStock(
        const std::string& account,
        const std::string& symbol,
        double quantity,
        std::optional<double> price = std::nullopt
    );
    
    Result<OrderResponse> sellStock(
        const std::string& account,
        const std::string& symbol,
        double quantity,
        std::optional<double> price = std::nullopt
    );
};

}