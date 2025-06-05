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

#include "tradier/json/account.hpp"
#include "tradier/common/json_utils.hpp"
#include "tradier/common/debug.hpp"

namespace tradier {
namespace json {

Account parseAccount(const nlohmann::json& json) {
    Account account;
    
    try {
        if (!json.is_null() && json.is_object()) {
            account.number = json.value("account_number", "");
            account.type = json.value("type", "");
            account.status = json.value("status", "");
            account.classification = json.value("classification", "");
            account.dayTrader = json.value("day_trader", false);
            account.optionLevel = json.value("option_level", 0);
            
            if (json.contains("date_created") && !json["date_created"].is_null() && json["date_created"].is_string()) {
                account.dateCreated = parseDateTime(json, "date_created");
            }
            if (json.contains("last_update_date") && !json["last_update_date"].is_null() && json["last_update_date"].is_string()) {
                account.lastUpdate = parseDateTime(json, "last_update_date");
            }
        }
    } catch (const std::exception& e) {
        throw ParseError("Failed to parse account: " + std::string(e.what()));
    }
    
    return account;
}

AccountProfile parseAccountProfile(const nlohmann::json& json) {
    AccountProfile profile;
    
    try {
        if (!json.is_null() && json.is_object()) {
            profile.id = json.value("id", "");
            profile.name = json.value("name", "");
            
            if (json.contains("account") && !json["account"].is_null()) {
                const auto& accounts = json["account"];
                if (accounts.is_array()) {
                    for (const auto& acc : accounts) {
                        if (!acc.is_null()) {
                            profile.accounts.push_back(parseAccount(acc));
                        }
                    }
                } else if (accounts.is_object()) {
                    profile.accounts.push_back(parseAccount(accounts));
                }
            }
        }
    } catch (const std::exception& e) {
        throw ParseError("Failed to parse account profile: " + std::string(e.what()));
    }
    
    return profile;
}

Position parsePosition(const nlohmann::json& json) {
    Position position;
    
    try {
        if (!json.is_null() && json.is_object()) {
            position.symbol = json.value("symbol", "");
            position.quantity = json.value("quantity", 0.0);
            position.costBasis = json.value("cost_basis", 0.0);
            
            if (json.contains("date_acquired") && !json["date_acquired"].is_null() && json["date_acquired"].is_string()) {
                position.acquired = parseDateTime(json, "date_acquired");
            }
        }
    } catch (const std::exception& e) {
        throw ParseError("Failed to parse position: " + std::string(e.what()));
    }
    
    return position;
}

Order parseOrder(const nlohmann::json& json) {
    Order order;
    
    try {
        if (!json.is_null() && json.is_object()) {
            order.id = json.value("id", 0);
            order.symbol = json.value("symbol", "");
            order.type = json.value("type", "");
            order.side = json.value("side", "");
            order.status = json.value("status", "");
            order.quantity = json.value("quantity", 0.0);
            order.price = json.value("price", 0.0);
            order.filled = json.value("exec_quantity", 0.0);
            
            if (json.contains("create_date") && !json["create_date"].is_null() && json["create_date"].is_string()) {
                order.created = parseDateTime(json, "create_date");
            }
            
            if (json.contains("tag") && !json["tag"].is_null() && json["tag"].is_string()) {
                order.tag = json["tag"];
            }
        }
    } catch (const std::exception& e) {
        throw ParseError("Failed to parse order: " + std::string(e.what()));
    }
    
    return order;
}

std::vector<Account> parseAccounts(const nlohmann::json& json) {
    std::vector<Account> accounts;
    
    try {
        if (!json.is_null()) {
            if (json.is_array()) {
                for (const auto& acc : json) {
                    if (!acc.is_null()) {
                        try {
                            accounts.push_back(parseAccount(acc));
                        } catch (const ParseError& e) {
                            // Log individual parsing error but continue with other accounts
                            // This allows partial success rather than complete failure
                            DEBUG_LOG("Failed to parse individual account: " + std::string(e.what()));
                        }
                    }
                }
            } else if (json.is_object()) {
                accounts.push_back(parseAccount(json));
            }
        }
    } catch (const std::exception& e) {
        throw ParseError("Failed to parse accounts array: " + std::string(e.what()));
    }
    
    return accounts;
}

std::vector<Position> parsePositions(const nlohmann::json& json) {
    std::vector<Position> positions;
    
    try {
        if (!json.is_null() && json.is_object() && json.contains("position") && !json["position"].is_null()) {
            const auto& pos = json["position"];
            if (pos.is_array()) {
                for (const auto& p : pos) {
                    if (!p.is_null()) {
                        positions.push_back(parsePosition(p));
                    }
                }
            } else if (pos.is_object()) {
                positions.push_back(parsePosition(pos));
            }
        }
    } catch (const std::exception& e) {
        throw ParseError("Failed to parse positions array: " + std::string(e.what()));
    }
    
    return positions;
}

std::vector<Order> parseOrders(const nlohmann::json& json) {
    std::vector<Order> orders;
    
    try {
        if (!json.is_null() && json.is_object() && json.contains("order") && !json["order"].is_null()) {
            const auto& ord = json["order"];
            if (ord.is_array()) {
                for (const auto& o : ord) {
                    if (!o.is_null()) {
                        orders.push_back(parseOrder(o));
                    }
                }
            } else if (ord.is_object()) {
                orders.push_back(parseOrder(ord));
            }
        }
    } catch (const std::exception& e) {
        throw ParseError("Failed to parse orders array: " + std::string(e.what()));
    }
    
    return orders;
}

} // namespace json
} // namespace tradier