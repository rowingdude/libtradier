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


#include "tradier/common/json_utils.hpp"
#include "tradier/common/utils.hpp"
#include "tradier/data.hpp"
#include "tradier/trading.hpp"

namespace tradier::json {

TimePoint parseDateTime(const nlohmann::json& json, const std::string& key) {
    if (json.contains(key) && !json[key].is_null()) {
        std::string dateStr = json[key];
        return utils::parseISODateTime(dateStr);
    }
    return TimePoint{};
}

std::string formatDateTime(const TimePoint& time) {
    return utils::formatISODateTime(time);
}

Account parseAccount(const nlohmann::json& json) {
    Account account;
    account.number = json.value("account_number", "");
    account.type = json.value("type", "");
    account.status = json.value("status", "");
    account.classification = json.value("classification", "");
    account.dayTrader = json.value("day_trader", false);
    account.optionLevel = json.value("option_level", 0);
    account.dateCreated = parseDateTime(json, "date_created");
    account.lastUpdate = parseDateTime(json, "last_update_date");
    return account;
}

AccountProfile parseAccountProfile(const nlohmann::json& json) {
    AccountProfile profile;
    profile.id = json.value("id", "");
    profile.name = json.value("name", "");
    
    if (json.contains("account")) {
        const auto& accounts = json["account"];
        if (accounts.is_array()) {
            for (const auto& acc : accounts) {
                profile.accounts.push_back(parseAccount(acc));
            }
        } else if (accounts.is_object()) {
            profile.accounts.push_back(parseAccount(accounts));
        }
    }
    
    return profile;
}

Position parsePosition(const nlohmann::json& json) {
    Position position;
    position.symbol = json.value("symbol", "");
    position.quantity = json.value("quantity", 0.0);
    position.costBasis = json.value("cost_basis", 0.0);
    position.acquired = parseDateTime(json, "date_acquired");
    return position;
}

Order parseOrder(const nlohmann::json& json) {
    Order order;
    order.id = json.value("id", 0);
    order.symbol = json.value("symbol", "");
    order.type = json.value("type", "");
    order.side = json.value("side", "");
    order.status = json.value("status", "");
    order.quantity = json.value("quantity", 0.0);
    order.price = json.value("price", 0.0);
    order.filled = json.value("exec_quantity", 0.0);
    order.created = parseDateTime(json, "create_date");
    
    if (json.contains("tag") && !json["tag"].is_null()) {
        order.tag = json["tag"];
    }
    
    return order;
}

Quote parseQuote(const nlohmann::json& json) {
    Quote quote;
    quote.symbol = json.value("symbol", "");
    quote.bid = json.value("bid", 0.0);
    quote.ask = json.value("ask", 0.0);
    quote.last = json.value("last", 0.0);
    quote.volume = json.value("volume", 0);
    quote.timestamp = std::chrono::system_clock::now();
    return quote;
}

OrderResponse parseOrderResponse(const nlohmann::json& json) {
    OrderResponse response;
    
    if (json.contains("order")) {
        const auto& order = json["order"];
        response.id = order.value("id", 0);
        response.status = order.value("status", "");
        
        if (order.contains("partner_id") && !order["partner_id"].is_null()) {
            response.partnerId = order["partner_id"];
        }
    }
    
    return response;
}

std::vector<Account> parseAccounts(const nlohmann::json& json) {
    std::vector<Account> accounts;
    
    if (json.is_array()) {
        for (const auto& acc : json) {
            accounts.push_back(parseAccount(acc));
        }
    } else if (json.is_object()) {
        accounts.push_back(parseAccount(json));
    }
    
    return accounts;
}

std::vector<Position> parsePositions(const nlohmann::json& json) {
    std::vector<Position> positions;
    
    if (json.contains("position")) {
        const auto& pos = json["position"];
        if (pos.is_array()) {
            for (const auto& p : pos) {
                positions.push_back(parsePosition(p));
            }
        } else if (pos.is_object()) {
            positions.push_back(parsePosition(pos));
        }
    }
    
    return positions;
}

std::vector<Order> parseOrders(const nlohmann::json& json) {
    std::vector<Order> orders;
    
    if (json.contains("order")) {
        const auto& ord = json["order"];
        if (ord.is_array()) {
            for (const auto& o : ord) {
                orders.push_back(parseOrder(o));
            }
        } else if (ord.is_object()) {
            orders.push_back(parseOrder(ord));
        }
    }
    
    return orders;
}

std::vector<Quote> parseQuotes(const nlohmann::json& json) {
    std::vector<Quote> quotes;
    
    if (json.contains("quote")) {
        const auto& q = json["quote"];
        if (q.is_array()) {
            for (const auto& quote : q) {
                quotes.push_back(parseQuote(quote));
            }
        } else if (q.is_object()) {
            quotes.push_back(parseQuote(q));
        }
    }
    
    return quotes;
}

}