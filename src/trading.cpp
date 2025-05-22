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


 #include "tradier/trading.hpp"
#include "tradier/client.hpp"
#include "tradier/common/errors.hpp"
#include "tradier/common/json_utils.hpp"
#include "tradier/common/utils.hpp"

namespace tradier {

std::string TradingService::toString(OrderType type) const {
    switch (type) {
        case OrderType::MARKET: return "market";
        case OrderType::LIMIT: return "limit";
        case OrderType::STOP: return "stop";
        case OrderType::STOP_LIMIT: return "stop_limit";
    }
    return "market";
}

std::string TradingService::toString(OrderSide side) const {
    switch (side) {
        case OrderSide::BUY: return "buy";
        case OrderSide::SELL: return "sell";
        case OrderSide::BUY_TO_OPEN: return "buy_to_open";
        case OrderSide::BUY_TO_CLOSE: return "buy_to_close";
        case OrderSide::SELL_TO_OPEN: return "sell_to_open";
        case OrderSide::SELL_TO_CLOSE: return "sell_to_close";
    }
    return "buy";
}

std::string TradingService::toString(OrderDuration duration) const {
    switch (duration) {
        case OrderDuration::DAY: return "day";
        case OrderDuration::GTC: return "gtc";
        case OrderDuration::PRE: return "pre";
        case OrderDuration::POST: return "post";
    }
    return "day";
}

FormParams TradingService::buildParams(const OrderRequest& request) const {
    FormParams params;
    
    params["class"] = request.optionSymbol ? "option" : "equity";
    params["symbol"] = request.symbol;
    params["side"] = toString(request.side);
    params["quantity"] = utils::toString(request.quantity);
    params["type"] = toString(request.type);
    params["duration"] = toString(request.duration);
    
    if (request.price) {
        params["price"] = utils::toString(*request.price);
    }
    
    if (request.stop) {
        params["stop"] = utils::toString(*request.stop);
    }
    
    if (request.optionSymbol) {
        params["option_symbol"] = *request.optionSymbol;
    }
    
    if (request.tag) {
        params["tag"] = *request.tag;
    }
    
    return params;
}

Result<OrderResponse> TradingService::placeOrder(const std::string& account, const OrderRequest& request) {
    if (account.empty()) {
        throw ValidationError("Account number cannot be empty");
    }
    
    auto params = buildParams(request);
    auto response = client_.post("/accounts/" + account + "/orders", params);
    
    return json::parseResponse<OrderResponse>(response, json::parseOrderResponse);
}

Result<OrderResponse> TradingService::cancelOrder(const std::string& account, int orderId) {
    if (account.empty()) {
        throw ValidationError("Account number cannot be empty");
    }
    if (orderId <= 0) {
        throw ValidationError("Order ID must be positive");
    }
    
    auto response = client_.del("/accounts/" + account + "/orders/" + std::to_string(orderId));
    return json::parseResponse<OrderResponse>(response, json::parseOrderResponse);
}

Result<OrderResponse> TradingService::modifyOrder(const std::string& account, int orderId, const OrderRequest& changes) {
    if (account.empty()) {
        throw ValidationError("Account number cannot be empty");
    }
    if (orderId <= 0) {
        throw ValidationError("Order ID must be positive");
    }
    
    FormParams params;
    params["type"] = toString(changes.type);
    params["duration"] = toString(changes.duration);
    
    if (changes.price) {
        params["price"] = utils::toString(*changes.price);
    }
    
    if (changes.stop) {
        params["stop"] = utils::toString(*changes.stop);
    }
    
    auto response = client_.put("/accounts/" + account + "/orders/" + std::to_string(orderId), params);
    return json::parseResponse<OrderResponse>(response, json::parseOrderResponse);
}

Result<OrderResponse> TradingService::buyStock(
    const std::string& account,
    const std::string& symbol,
    double quantity,
    std::optional<double> price) {
    
    OrderRequest request;
    request.symbol = symbol;
    request.side = OrderSide::BUY;
    request.quantity = quantity;
    request.type = price ? OrderType::LIMIT : OrderType::MARKET;
    request.price = price;
    
    return placeOrder(account, request);
}

Result<OrderResponse> TradingService::sellStock(
    const std::string& account,
    const std::string& symbol,
    double quantity,
    std::optional<double> price) {
    
    OrderRequest request;
    request.symbol = symbol;
    request.side = OrderSide::SELL;
    request.quantity = quantity;
    request.type = price ? OrderType::LIMIT : OrderType::MARKET;
    request.price = price;
    
    return placeOrder(account, request);
}

}
