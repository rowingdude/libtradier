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

#include "tradier/account.hpp"
#include "tradier/json/account.hpp"
#include "tradier/client.hpp"
#include "tradier/common/errors.hpp"
#include "tradier/common/json_utils.hpp"

namespace tradier {

Result<AccountProfile> AccountService::getProfile() {
    auto response = client_.get("/user/profile");
    return json::parseResponse<AccountProfile>(response, [](const auto& json) {
        if (!json.contains("profile")) {
            throw ApiError(400, "Invalid profile response format");
        }
        return json::parseAccountProfile(json["profile"]);
    });
}

Result<Account> AccountService::getAccount(const std::string& accountNumber) {
    if (accountNumber.empty()) {
        throw ValidationError("Account number cannot be empty");
    }
    
    auto profile = getProfile();
    if (!profile) return std::nullopt;
    
    for (const auto& account : profile->accounts) {
        if (account.number == accountNumber) {
            return account;
        }
    }
    
    return std::nullopt;
}

Result<AccountBalances> AccountService::getBalances(const std::string& accountNumber) {
    if (accountNumber.empty()) {
        throw ValidationError("Account number cannot be empty");
    }
    
    auto response = client_.get("/accounts/" + accountNumber + "/balances");
    return json::parseResponse<AccountBalances>(response, [](const auto& json) {
        if (!json.contains("balances")) {
            throw ApiError(400, "Invalid balances response format");
        }
        
        const auto& bal = json["balances"];
        AccountBalances balances;
        balances.accountNumber = bal.value("account_number", "");
        balances.accountType = bal.value("account_type", "");
        balances.totalEquity = bal.value("total_equity", 0.0);
        balances.totalCash = bal.value("total_cash", 0.0);
        balances.marketValue = bal.value("market_value", 0.0);
        balances.dayChange = bal.value("close_pl", 0.0);
        
        if (bal.contains("margin") && bal["margin"].contains("stock_buying_power")) {
            balances.buyingPower = bal["margin"]["stock_buying_power"];
        } else if (bal.contains("cash") && bal["cash"].contains("cash_available")) {
            balances.buyingPower = bal["cash"]["cash_available"];
        }
        
        return balances;
    });
}

Result<std::vector<Position>> AccountService::getPositions(const std::string& accountNumber) {
    if (accountNumber.empty()) {
        throw ValidationError("Account number cannot be empty");
    }
    
    auto response = client_.get("/accounts/" + accountNumber + "/positions");
    return json::parseResponse<std::vector<Position>>(response, [](const auto& json) {
        if (!json.contains("positions")) {
            return std::vector<Position>{};
        }
        return json::parsePositions(json["positions"]);
    });
}

Result<std::vector<Order>> AccountService::getOrders(const std::string& accountNumber) {
    if (accountNumber.empty()) {
        throw ValidationError("Account number cannot be empty");
    }
    
    auto response = client_.get("/accounts/" + accountNumber + "/orders");
    return json::parseResponse<std::vector<Order>>(response, [](const auto& json) {
        if (!json.contains("orders")) {
            return std::vector<Order>{};
        }
        return json::parseOrders(json["orders"]);
    });
}

Result<Order> AccountService::getOrder(const std::string& accountNumber, int orderId) {
    if (accountNumber.empty()) {
        throw ValidationError("Account number cannot be empty");
    }
    if (orderId <= 0) {
        throw ValidationError("Order ID must be positive");
    }
    
    auto response = client_.get("/accounts/" + accountNumber + "/orders/" + std::to_string(orderId));
    return json::parseResponse<Order>(response, [](const auto& json) {
        if (json.contains("order")) {
            return json::parseOrder(json["order"]);
        }
        return json::parseOrder(json);
    });
}

Result<std::vector<HistoryEvent>> AccountService::getHistory(
    const std::string& accountNumber,
    const std::string& startDate,
    const std::string& endDate) {
    
    if (accountNumber.empty()) {
        throw ValidationError("Account number cannot be empty");
    }
    
    QueryParams params;
    if (!startDate.empty()) params["start"] = startDate;
    if (!endDate.empty()) params["end"] = endDate;
    
    auto response = client_.get("/accounts/" + accountNumber + "/history", params);
    return json::parseResponse<std::vector<HistoryEvent>>(response, [](const auto& json) {
        std::vector<HistoryEvent> events;
        
        if (json.contains("history") && json["history"].contains("event")) {
            const auto& eventJson = json["history"]["event"];
            
            if (eventJson.is_array()) {
                for (const auto& event : eventJson) {
                    HistoryEvent histEvent;
                    histEvent.amount = event.value("amount", 0.0);
                    histEvent.date = json::parseDateTime(event, "date");
                    histEvent.type = event.value("type", "");
                    histEvent.description = event.value("description", "");
                    
                    if (event.contains("trade") && event["trade"].contains("symbol")) {
                        histEvent.symbol = event["trade"]["symbol"];
                    }
                    
                    events.push_back(histEvent);
                }
            } else if (eventJson.is_object()) {
                HistoryEvent histEvent;
                histEvent.amount = eventJson.value("amount", 0.0);
                histEvent.date = json::parseDateTime(eventJson, "date");
                histEvent.type = eventJson.value("type", "");
                histEvent.description = eventJson.value("description", "");
                events.push_back(histEvent);
            }
        }
        
        return events;
    });
}

}