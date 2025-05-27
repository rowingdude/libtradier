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
#include "tradier/json/trading.hpp"

namespace tradier {

std::string TradingService::toString(OrderType type) const {
    switch (type) {
        case OrderType::MARKET: return "market";
        case OrderType::LIMIT: return "limit";
        case OrderType::STOP: return "stop";
        case OrderType::STOP_LIMIT: return "stop_limit";
        case OrderType::DEBIT: return "debit";
        case OrderType::CREDIT: return "credit";
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
    return tryExecute<OrderResponse>([&]() -> OrderResponse {
        if (account.empty()) {
            throw ValidationError("Account number cannot be empty");
        }
        
        auto params = buildParams(request);
        auto response = client_.post("/accounts/" + account + "/orders", params);
        
        if (!response.success()) {
            throw ApiError(response.status, "Failed to place order: " + response.body);
        }
        
        auto parsed = json::parseResponse<OrderResponse>(response, json::parseOrderResponse);
        if (!parsed) {
            throw std::runtime_error("Failed to parse order response");
        }
        
        return *parsed;
    }, "placeOrder");
}

Result<OrderResponse> TradingService::cancelOrder(const std::string& account, int orderId) {
    return tryExecute<OrderResponse>([&]() -> OrderResponse {
        if (account.empty()) {
            throw ValidationError("Account number cannot be empty");
        }
        if (orderId <= 0) {
            throw ValidationError("Order ID must be positive");
        }
        
        auto response = client_.del("/accounts/" + account + "/orders/" + std::to_string(orderId));
        
        if (!response.success()) {
            throw ApiError(response.status, "Failed to cancel order: " + response.body);
        }
        
        auto parsed = json::parseResponse<OrderResponse>(response, json::parseOrderResponse);
        if (!parsed) {
            throw std::runtime_error("Failed to parse cancel order response");
        }
        
        return *parsed;
    }, "cancelOrder");
}

Result<OrderResponse> TradingService::modifyOrder(const std::string& account, int orderId, const OrderRequest& changes) {
    return tryExecute<OrderResponse>([&]() -> OrderResponse {
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
        
        if (!response.success()) {
            throw ApiError(response.status, "Failed to modify order: " + response.body);
        }
        
        auto parsed = json::parseResponse<OrderResponse>(response, json::parseOrderResponse);
        if (!parsed) {
            throw std::runtime_error("Failed to parse modify order response");
        }
        
        return *parsed;
    }, "modifyOrder");
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

std::string TradingService::toString(OptionAction action) const {
    switch (action) {
        case OptionAction::BUY_TO_OPEN: return "buy_to_open";
        case OptionAction::BUY_TO_CLOSE: return "buy_to_close";
        case OptionAction::SELL_TO_OPEN: return "sell_to_open";
        case OptionAction::SELL_TO_CLOSE: return "sell_to_close";
    }
    return "buy_to_open";
}

OrderStatus TradingService::parseOrderStatus(const std::string& statusString) const {
    if (statusString == "open") return OrderStatus::OPEN;
    if (statusString == "partially_filled") return OrderStatus::PARTIALLY_FILLED;
    if (statusString == "filled") return OrderStatus::FILLED;
    if (statusString == "expired") return OrderStatus::EXPIRED;
    if (statusString == "canceled") return OrderStatus::CANCELED;
    if (statusString == "pending") return OrderStatus::PENDING;
    if (statusString == "rejected") return OrderStatus::REJECTED;
    if (statusString == "error") return OrderStatus::ERROR;
    return OrderStatus::PENDING;
}

Result<OrderPreview> TradingService::previewOrder(const std::string& account, const OrderRequest& request) {
    return tryExecute<OrderPreview>([&]() -> OrderPreview {
        if (account.empty()) {
            throw ValidationError("Account number cannot be empty");
        }
        
        auto params = buildParams(request);
        auto response = client_.post("/accounts/" + account + "/orders/preview", params);
        
        if (!response.success()) {
            throw ApiError(response.status, "Failed to preview order: " + response.body);
        }
        
        auto parsed = json::parseResponse<OrderPreview>(response, json::parseOrderPreview);
        if (!parsed) {
            throw std::runtime_error("Failed to parse order preview response");
        }
        
        return *parsed;
    }, "previewOrder");
}

Result<OrderResponse> TradingService::placeBracketOrder(const std::string& account, const BracketOrder& bracketOrder) {
    return tryExecute<OrderResponse>([&]() -> OrderResponse {
        if (account.empty()) {
            throw ValidationError("Account number cannot be empty");
        }
        
        FormParams params;
        params["class"] = "oto";
        params["symbol"] = bracketOrder.symbol;
        params["side"] = toString(bracketOrder.side);
        params["quantity"] = utils::toString(bracketOrder.quantity);
        params["type"] = "limit";
        params["price"] = utils::toString(bracketOrder.entryPrice);
        params["duration"] = toString(bracketOrder.duration);
        
        params["oto[0][instrument]"] = "equity";
        params["oto[0][symbol]"] = bracketOrder.symbol;
        params["oto[0][side]"] = bracketOrder.side == OrderSide::BUY ? "sell" : "buy";
        params["oto[0][quantity]"] = utils::toString(bracketOrder.quantity);
        params["oto[0][type]"] = "limit";
        params["oto[0][price]"] = utils::toString(bracketOrder.takeProfitPrice);
        
        params["oto[1][instrument]"] = "equity";
        params["oto[1][symbol]"] = bracketOrder.symbol;
        params["oto[1][side]"] = bracketOrder.side == OrderSide::BUY ? "sell" : "buy";
        params["oto[1][quantity]"] = utils::toString(bracketOrder.quantity);
        params["oto[1][type]"] = "stop";
        params["oto[1][stop]"] = utils::toString(bracketOrder.stopLossPrice);
        
        if (bracketOrder.tag) {
            params["tag"] = *bracketOrder.tag;
        }
        
        auto response = client_.post("/accounts/" + account + "/orders", params);
        
        if (!response.success()) {
            throw ApiError(response.status, "Failed to place bracket order: " + response.body);
        }
        
        auto parsed = json::parseResponse<OrderResponse>(response, json::parseOrderResponse);
        if (!parsed) {
            throw std::runtime_error("Failed to parse bracket order response");
        }
        
        return *parsed;
    }, "placeBracketOrder");
}

Result<OrderResponse> TradingService::placeMultiLegOrder(const std::string& account, const MultiLegOrder& multiLegOrder) {
    return tryExecute<OrderResponse>([&]() -> OrderResponse {
        if (account.empty()) {
            throw ValidationError("Account number cannot be empty");
        }
        if (multiLegOrder.legs.empty()) {
            throw ValidationError("Multi-leg order must have at least one leg");
        }
        
        FormParams params;
        params["class"] = "multileg";
        params["type"] = toString(multiLegOrder.type);
        params["duration"] = toString(multiLegOrder.duration);
        
        if (multiLegOrder.netDebit) {
            params["price"] = utils::toString(*multiLegOrder.netDebit);
        }
        if (multiLegOrder.netCredit) {
            params["price"] = utils::toString(*multiLegOrder.netCredit);
        }
        
        for (size_t i = 0; i < multiLegOrder.legs.size(); ++i) {
            const auto& leg = multiLegOrder.legs[i];
            std::string prefix = "option[" + std::to_string(i) + "]";
            
            params[prefix + "[option_symbol]"] = leg.optionSymbol;
            params[prefix + "[side]"] = toString(static_cast<OrderSide>(leg.action));
            params[prefix + "[quantity]"] = utils::toString(leg.quantity);
        }
        
        if (multiLegOrder.tag) {
            params["tag"] = *multiLegOrder.tag;
        }
        
        auto response = client_.post("/accounts/" + account + "/orders", params);
        
        if (!response.success()) {
            throw ApiError(response.status, "Failed to place multi-leg order: " + response.body);
        }
        
        auto parsed = json::parseResponse<OrderResponse>(response, json::parseOrderResponse);
        if (!parsed) {
            throw std::runtime_error("Failed to parse multi-leg order response");
        }
        
        return *parsed;
    }, "placeMultiLegOrder");
}

Result<OrderResponse> TradingService::modifyOrderAdvanced(const std::string& account, int orderId, const OrderModification& modification) {
    return tryExecute<OrderResponse>([&]() -> OrderResponse {
        if (account.empty()) {
            throw ValidationError("Account number cannot be empty");
        }
        if (orderId <= 0) {
            throw ValidationError("Order ID must be positive");
        }
        
        FormParams params;
        
        if (modification.type) {
            params["type"] = toString(*modification.type);
        }
        if (modification.price) {
            params["price"] = utils::toString(*modification.price);
        }
        if (modification.stop) {
            params["stop"] = utils::toString(*modification.stop);
        }
        if (modification.quantity) {
            params["quantity"] = utils::toString(*modification.quantity);
        }
        if (modification.duration) {
            params["duration"] = toString(*modification.duration);
        }
        
        auto response = client_.put("/accounts/" + account + "/orders/" + std::to_string(orderId), params);
        
        if (!response.success()) {
            throw ApiError(response.status, "Failed to modify order: " + response.body);
        }
        
        auto parsed = json::parseResponse<OrderResponse>(response, json::parseOrderResponse);
        if (!parsed) {
            throw std::runtime_error("Failed to parse order modification response");
        }
        
        return *parsed;
    }, "modifyOrderAdvanced");
}

Result<OrderResponse> TradingService::buyToOpenOption(
    const std::string& account,
    const std::string& optionSymbol,
    double quantity,
    std::optional<double> price) {
    
    OrderRequest request;
    request.optionSymbol = optionSymbol;
    request.side = OrderSide::BUY_TO_OPEN;
    request.quantity = quantity;
    request.type = price ? OrderType::LIMIT : OrderType::MARKET;
    request.price = price;
    
    return placeOrder(account, request);
}

Result<OrderResponse> TradingService::sellToOpenOption(
    const std::string& account,
    const std::string& optionSymbol,
    double quantity,
    std::optional<double> price) {
    
    OrderRequest request;
    request.optionSymbol = optionSymbol;
    request.side = OrderSide::SELL_TO_OPEN;
    request.quantity = quantity;
    request.type = price ? OrderType::LIMIT : OrderType::MARKET;
    request.price = price;
    
    return placeOrder(account, request);
}

Result<OrderResponse> TradingService::buyToCloseOption(
    const std::string& account,
    const std::string& optionSymbol,
    double quantity,
    std::optional<double> price) {
    
    OrderRequest request;
    request.optionSymbol = optionSymbol;
    request.side = OrderSide::BUY_TO_CLOSE;
    request.quantity = quantity;
    request.type = price ? OrderType::LIMIT : OrderType::MARKET;
    request.price = price;
    
    return placeOrder(account, request);
}

Result<OrderResponse> TradingService::sellToCloseOption(
    const std::string& account,
    const std::string& optionSymbol,
    double quantity,
    std::optional<double> price) {
    
    OrderRequest request;
    request.optionSymbol = optionSymbol;
    request.side = OrderSide::SELL_TO_CLOSE;
    request.quantity = quantity;
    request.type = price ? OrderType::LIMIT : OrderType::MARKET;
    request.price = price;
    
    return placeOrder(account, request);
}

Result<std::vector<OrderResponse>> TradingService::cancelAllOrders(const std::string& account) {
    return tryExecute<std::vector<OrderResponse>>([&]() -> std::vector<OrderResponse> {
        if (account.empty()) {
            throw ValidationError("Account number cannot be empty");
        }
        
        auto response = client_.del("/accounts/" + account + "/orders");
        
        if (!response.success()) {
            throw ApiError(response.status, "Failed to cancel all orders: " + response.body);
        }
        
        auto parsed = json::parseResponse<std::vector<OrderResponse>>(response, [](const auto& json) {
            std::vector<OrderResponse> responses;
            
            if (json.contains("orders") && json["orders"].contains("order")) {
                const auto& orders = json["orders"]["order"];
                if (orders.is_array()) {
                    for (const auto& order : orders) {
                        responses.push_back(json::parseOrderResponse(order));
                    }
                } else if (orders.is_object()) {
                    responses.push_back(json::parseOrderResponse(orders));
                }
            }
            
            return responses;
        });
        
        if (!parsed) {
            throw std::runtime_error("Failed to parse cancel all orders response");
        }
        
        return *parsed;
    }, "cancelAllOrders");
}

}