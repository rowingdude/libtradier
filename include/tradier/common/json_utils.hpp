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

#include <nlohmann/json.hpp>
#include "tradier/common/types.hpp"

namespace tradier {

struct Account;
struct AccountProfile;
struct Position;
struct Order;
struct Quote;
struct OrderResponse;
struct HistoryEvent;

namespace json {

TimePoint parseDateTime(const nlohmann::json& json, const std::string& key);
std::string formatDateTime(const TimePoint& time);

template<typename T>
Result<T> parseResponse(const Response& response, std::function<T(const nlohmann::json&)> parser) {
    if (!response.success()) {
        return std::nullopt;
    }
    
    try {
        auto json = nlohmann::json::parse(response.body);
        return parser(json);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

Account parseAccount(const nlohmann::json& json);
AccountProfile parseAccountProfile(const nlohmann::json& json);
Position parsePosition(const nlohmann::json& json);
Order parseOrder(const nlohmann::json& json);
Quote parseQuote(const nlohmann::json& json);
OrderResponse parseOrderResponse(const nlohmann::json& json);

std::vector<Account> parseAccounts(const nlohmann::json& json);
std::vector<Position> parsePositions(const nlohmann::json& json);
std::vector<Order> parseOrders(const nlohmann::json& json);
std::vector<Quote> parseQuotes(const nlohmann::json& json);

}
}