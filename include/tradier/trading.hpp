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

namespace tradier {

class TradierClient;

enum class OrderType { 
    MARKET, 
    LIMIT, 
    STOP, 
    STOP_LIMIT,
    DEBIT,     
    CREDIT     
};

enum class OrderSide { 
    BUY, 
    SELL, 
    BUY_TO_OPEN, 
    BUY_TO_CLOSE, 
    SELL_TO_OPEN, 
    SELL_TO_CLOSE 
};

enum class OrderDuration { 
    DAY, 
    GTC, 
    PRE, 
    POST 
};

enum class OrderClass {
    EQUITY,
    OPTION,
    MULTILEG,
    COMBO
};

enum class OptionAction {
    BUY_TO_OPEN,
    BUY_TO_CLOSE, 
    SELL_TO_OPEN,
    SELL_TO_CLOSE
};

enum class OrderStatus {
    OPEN,
    PARTIALLY_FILLED,
    FILLED,
    EXPIRED,
    CANCELED,
    PENDING,
    REJECTED,
    ERROR
};

struct OptionLeg {
    std::string optionSymbol;
    OptionAction action;
    double quantity = 0.0;
};

struct BracketOrder {
    std::string symbol;
    OrderSide side;
    double quantity = 0.0;
    double entryPrice = 0.0;
    double takeProfitPrice = 0.0;
    double stopLossPrice = 0.0;
    OrderDuration duration = OrderDuration::DAY;
    std::optional<std::string> tag;
};

struct MultiLegOrder {
    std::vector<OptionLeg> legs;
    OrderType type = OrderType::MARKET;
    OrderDuration duration = OrderDuration::DAY;
    std::optional<double> netDebit;
    std::optional<double> netCredit;
    std::optional<std::string> tag;
};

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

struct OrderModification {
    std::optional<OrderType> type;
    std::optional<double> price;
    std::optional<double> stop;
    std::optional<double> quantity;
    std::optional<OrderDuration> duration;
};

struct OrderPreview {
    std::string status;
    double commission = 0.0;
    double cost = 0.0;
    double fees = 0.0;
    std::string symbol;
    double quantity = 0.0;
    bool result = false;
    double orderCost = 0.0;
    double marginChange = 0.0;
    std::string requestDate;
    bool extendedHours = false;
    std::string strategy;
    int dayTrades = 0;
    OrderSide side = OrderSide::BUY;
    OrderType type = OrderType::MARKET;
    OrderDuration duration = OrderDuration::DAY;
    OrderClass orderClass = OrderClass::EQUITY;
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
    Result<OrderPreview>  previewOrder(const std::string& account, const OrderRequest& request);
    Result<OrderResponse> placeBracketOrder(const std::string& account, const BracketOrder& bracketOrder);
    Result<OrderResponse> placeMultiLegOrder(const std::string& account, const MultiLegOrder& multiLegOrder);
    Result<OrderResponse> modifyOrderAdvanced(const std::string& account, int orderId, const OrderModification& modification);

    Result<OrderResponse> buyToOpenOption(
        const std::string& account,
        const std::string& optionSymbol,
        double quantity,
        std::optional<double> price = std::nullopt
    );

    Result<OrderResponse> sellToOpenOption(
        const std::string& account,
        const std::string& optionSymbol,
        double quantity,
        std::optional<double> price = std::nullopt
    );

    Result<OrderResponse> buyToCloseOption(
        const std::string& account,
        const std::string& optionSymbol,
        double quantity,
        std::optional<double> price = std::nullopt
    );

    Result<OrderResponse> sellToCloseOption(
        const std::string& account,
        const std::string& optionSymbol,
        double quantity,
        std::optional<double> price = std::nullopt
    );

    
    
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

    Result<std::vector<OrderResponse>> cancelAllOrders(const std::string& account);
    OrderStatus parseOrderStatus(const std::string& statusString) const;
    std::string toString(OptionAction action) const;    
    
};

}
