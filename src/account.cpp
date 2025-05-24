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
#include "tradier/common/json_utils.hpp"
#include "tradier/common/errors.hpp"
#include "tradier/common/types.hpp"
#include "tradier/client.hpp"
#include <nlohmann/json.hpp>

namespace tradier {

std::optional<Account> AccountService::getAccount(const std::string& accountNumber) {
    if (accountNumber.empty()) {
        throw ValidationError("Account number cannot be empty");
    }
    
    auto response = client_.get("/accounts/" + accountNumber);
    return json::parseResponseSafe<Account>(response, [](const json::SafeJsonParser& json) {
        if (!json.contains("account")) {
            throw ApiError(400, "Invalid account response format");
        }
        return json::parseAccount(json[std::string("account")]);
    });
}

std::optional<AccountProfile> AccountService::getProfile() {
    auto response = client_.get("/accounts/" + client_.config().accountNumber + "/profile");
    return json::parseResponseSafe<AccountProfile>(response, [](const json::SafeJsonParser& json) {
        if (!json.contains("profile")) {
            throw ApiError(400, "Invalid profile response format");
        }
        return json::parseAccountProfile(json[std::string("profile")]);
    });
}

std::optional<AccountBalances> AccountService::getBalances(const std::string& accountNumber) {
    auto response = client_.get("/accounts/" + accountNumber + "/balances");
    return json::parseResponseSafe<AccountBalances>(response, [](const json::SafeJsonParser& json) {
        if (!json.contains("balances")) {
            throw ApiError(400, "Invalid balances response format");
        }

        const auto& bal = json[std::string("balances")];
        AccountBalances balances;
        balances.accountNumber = bal.value("account_number", "");
        balances.accountType = bal.value("account_type", "");
        balances.totalEquity = bal.value("total_equity", 0.0);
        balances.totalCash = bal.value("total_cash", 0.0);
        balances.marketValue = bal.value("market_value", 0.0);
        balances.dayChange = bal.value("close_pl", 0.0);

        if (bal.contains("margin") && bal[std::string("margin")].contains("stock_buying_power")) {
            balances.buyingPower = bal[std::string("margin")][std::string("stock_buying_power")];
        } else if (bal.contains("cash") && bal[std::string("cash")].contains("cash_available")) {
            balances.buyingPower = bal[std::string("cash")][std::string("cash_available")];
        }

        return balances;
    });
}

std::optional<std::vector<Position>> AccountService::getPositions(const std::string& accountNumber) {
    auto response = client_.get("/accounts/" + accountNumber + "/positions");
    return json::parseResponseSafe<std::vector<Position>>(response, [](const json::SafeJsonParser& json) {
        return json::parsePositions(json);
    });
}

std::optional<std::vector<Order>> AccountService::getOrders(const std::string& accountNumber) {
    auto response = client_.get("/accounts/" + accountNumber + "/orders");
    return json::parseResponseSafe<std::vector<Order>>(response, [](const json::SafeJsonParser& json) {
        return json::parseOrders(json);
    });
}

std::optional<Order> AccountService::getOrder(const std::string& accountNumber, int orderId) {
    auto response = client_.get("/accounts/" + accountNumber + "/orders/" + std::to_string(orderId));
    return json::parseResponseSafe<Order>(response, [](const json::SafeJsonParser& json) {
        if (!json.contains("order")) {
            throw ApiError(400, "Invalid order response format");
        }
        return json::parseOrder(json[std::string("order")]);
    });
}

std::optional<std::vector<HistoryEvent>> AccountService::getHistory(
    const std::string& accountNumber,
    const std::string& startDate,
    const std::string& endDate) {
    std::string endpoint = "/accounts/" + accountNumber + "/history";
    QueryParams query;
    if (!startDate.empty()) query["start"] = startDate;
    if (!endDate.empty()) query["end"] = endDate;

    auto response = client_.get(endpoint, query);
    return json::parseResponseSafe<std::vector<HistoryEvent>>(response, [](const json::SafeJsonParser& json) {
        std::vector<HistoryEvent> events;
        if (json.contains("history") && json[std::string("history")].contains("event")) {
            const auto& evts = json[std::string("history")][std::string("event")];
            if (evts.get().is_array()) {
                for (const auto& evt : evts.get()) {
                    HistoryEvent event;
                    event.type = evt.value("type", "");
                    event.date = json::parseDateTime(evt, "date");
                    event.amount = evt.value("amount", 0.0);
                    events.push_back(event);
                }
            } else if (evts.get().is_object()) {
                HistoryEvent event;
                event.type = evts.value("type", "");
                event.date = json::parseDateTime(evts, "date");
                event.amount = evts.value("amount", 0.0);
                events.push_back(event);
            }
        }
        return events;
    });
}

} // namespace tradier
