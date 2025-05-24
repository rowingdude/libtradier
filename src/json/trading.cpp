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

#include "tradier/json/trading.hpp"
#include "tradier/common/json_utils.hpp"

namespace tradier {
namespace json {

OrderResponse parseOrderResponse(const nlohmann::json& json) {
    OrderResponse response;
    
    if (!json.is_null() && json.is_object() && json.contains("order") && !json["order"].is_null() && json["order"].is_object()) {
        const auto& order = json["order"];
        response.id = order.value("id", 0);
        response.status = order.value("status", "");
        
        if (order.contains("partner_id") && !order["partner_id"].is_null() && order["partner_id"].is_string()) {
            response.partnerId = order["partner_id"];
        }
    }
    
    return response;
}

OrderPreview parseOrderPreview(const nlohmann::json& json) {
    OrderPreview preview;
    
    if (!json.is_null() && json.is_object() && json.contains("order") && !json["order"].is_null() && json["order"].is_object()) {
        const auto& order = json["order"];
        
        preview.status = order.value("status", "");
        preview.commission = order.value("commission", 0.0);
        preview.cost = order.value("cost", 0.0);
        preview.fees = order.value("fees", 0.0);
        preview.symbol = order.value("symbol", "");
        preview.quantity = order.value("quantity", 0.0);
        preview.result = order.value("result", false);
        preview.orderCost = order.value("order_cost", 0.0);
        preview.marginChange = order.value("margin_change", 0.0);
        preview.requestDate = order.value("request_date", "");
        preview.extendedHours = order.value("extended_hours", false);
        preview.strategy = order.value("strategy", "");
        preview.dayTrades = order.value("day_trades", 0);
        
        std::string sideStr = order.value("side", "");
        if (!sideStr.empty()) {
            if (sideStr == "buy") preview.side = OrderSide::BUY;
            else if (sideStr == "sell") preview.side = OrderSide::SELL;
            else if (sideStr == "buy_to_open") preview.side = OrderSide::BUY_TO_OPEN;
            else if (sideStr == "buy_to_close") preview.side = OrderSide::BUY_TO_CLOSE;
            else if (sideStr == "sell_to_open") preview.side = OrderSide::SELL_TO_OPEN;
            else if (sideStr == "sell_to_close") preview.side = OrderSide::SELL_TO_CLOSE;
            else preview.side = OrderSide::BUY;
        } else {
            preview.side = OrderSide::BUY;
        }
        
        std::string typeStr = order.value("type", "");
        if (!typeStr.empty()) {
            if (typeStr == "market") preview.type = OrderType::MARKET;
            else if (typeStr == "limit") preview.type = OrderType::LIMIT;
            else if (typeStr == "stop") preview.type = OrderType::STOP;
            else if (typeStr == "stop_limit") preview.type = OrderType::STOP_LIMIT;
            else if (typeStr == "debit") preview.type = OrderType::DEBIT;
            else if (typeStr == "credit") preview.type = OrderType::CREDIT;
            else preview.type = OrderType::MARKET;
        } else {
            preview.type = OrderType::MARKET;
        }
        
        std::string durationStr = order.value("duration", "");
        if (!durationStr.empty()) {
            if (durationStr == "day") preview.duration = OrderDuration::DAY;
            else if (durationStr == "gtc") preview.duration = OrderDuration::GTC;
            else if (durationStr == "pre") preview.duration = OrderDuration::PRE;
            else if (durationStr == "post") preview.duration = OrderDuration::POST;
            else preview.duration = OrderDuration::DAY;
        } else {
            preview.duration = OrderDuration::DAY;
        }
        
        std::string classStr = order.value("class", "");
        if (!classStr.empty()) {
            if (classStr == "equity") preview.orderClass = OrderClass::EQUITY;
            else if (classStr == "option") preview.orderClass = OrderClass::OPTION;
            else if (classStr == "multileg") preview.orderClass = OrderClass::MULTILEG;
            else if (classStr == "combo") preview.orderClass = OrderClass::COMBO;
            else preview.orderClass = OrderClass::EQUITY;
        } else {
            preview.orderClass = OrderClass::EQUITY;
        }
    }
    
    return preview;
}

} // namespace json
} // namespace tradier