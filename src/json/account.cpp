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
#include "tradier/account.hpp"
#include "tradier/common/utils.hpp"

namespace tradier {
namespace json {

Account parseAccount(const nlohmann::json& json) {
    Account account;
    
    account.accountNumber = json.value("account_number", "");
    account.classification = json.value("classification", "");
    
    if (json.contains("date_created") && !json["date_created"].is_null()) {
        account.dateCreated = parseISODateTime(json["date_created"]);
    }
    
    account.dayTrader = json.value("day_trader", false);
    account.optionLevel = json.value("option_level", 0);
    account.status = json.value("status", "");
    account.type = json.value("type", "");
    
    if (json.contains("last_update_date") && !json["last_update_date"].is_null()) {
        account.lastUpdateDate = parseISODateTime(json["last_update_date"]);
    }
    
    return account;
}

AccountProfile parseAccountProfile(const nlohmann::json& json) {
    AccountProfile profile;
    
    if (json.contains("id") && !json["id"].is_null()) {
        profile.id = json["id"];
    }
    
    if (json.contains("name") && !json["name"].is_null()) {
        profile.name = json["name"];
    }
    
    if (json.contains("account")) {
        auto accountsJson = json["account"];
        
        if (accountsJson.is_array()) {
            for (const auto& accountJson : accountsJson) {
                profile.accounts.push_back(parseAccount(accountJson));
            }
        } else if (accountsJson.is_object()) {
            profile.accounts.push_back(parseAccount(accountsJson));
        }
    }
    
    return profile;
}

MarginBalances parseMarginBalances(const nlohmann::json& json) {
    MarginBalances margin;
    
    margin.fedCall = json.value("fed_call", 0.0);
    margin.maintenanceCall = json.value("maintenance_call", 0.0);
    margin.optionBuyingPower = json.value("option_buying_power", 0.0);
    margin.stockBuyingPower = json.value("stock_buying_power", 0.0);
    margin.stockShortValue = json.value("stock_short_value", 0.0);
    margin.sweep = json.value("sweep", 0.0);
    
    return margin;
}

CashBalances parseCashBalances(const nlohmann::json& json) {
    CashBalances cash;
    
    cash.cashAvailable = json.value("cash_available", 0.0);
    cash.sweep = json.value("sweep", 0.0);
    cash.unsettledFunds = json.value("unsettled_funds", 0.0);
    
    return cash;
}

PDTBalances parsePDTBalances(const nlohmann::json& json) {
    PDTBalances pdt;
    
    pdt.fedCall = json.value("fed_call", 0.0);
    pdt.maintenanceCall = json.value("maintenance_call", 0.0);
    pdt.optionBuyingPower = json.value("option_buying_power", 0.0);
    pdt.stockBuyingPower = json.value("stock_buying_power", 0.0);
    pdt.stockShortValue = json.value("stock_short_value", 0.0);
    
    return pdt;
}

AccountBalances parseAccountBalances(const nlohmann::json& json) {
    AccountBalances balances;
    
    balances.optionShortValue = json.value("option_short_value", 0.0);
    balances.totalEquity = json.value("total_equity", 0.0);
    balances.accountNumber = json.value("account_number", "");
    balances.accountType = json.value("account_type", "");
    balances.closePL = json.value("close_pl", 0.0);
    balances.currentRequirement = json.value("current_requirement", 0.0);
    balances.equity = json.value("equity", 0.0);
    balances.longMarketValue = json.value("long_market_value", 0.0);
    balances.marketValue = json.value("market_value", 0.0);
    balances.openPL = json.value("open_pl", 0.0);
    balances.optionLongValue = json.value("option_long_value", 0.0);
    balances.optionRequirement = json.value("option_requirement", 0.0);
    balances.pendingOrdersCount = json.value("pending_orders_count", 0);
    balances.shortMarketValue = json.value("short_market_value", 0.0);
    balances.stockLongValue = json.value("stock_long_value", 0.0);
    balances.totalCash = json.value("total_cash", 0.0);
    balances.unclearedFunds = json.value("uncleared_funds", 0.0);
    balances.pendingCash = json.value("pending_cash", 0.0);
    
    if (json.contains("margin") && !json["margin"].is_null()) {
        balances.margin = parseMarginBalances(json["margin"]);
    }
    
    if (json.contains("cash") && !json["cash"].is_null()) {
        balances.cash = parseCashBalances(json["cash"]);
    }
    
    if (json.contains("pdt") && !json["pdt"].is_null()) {
        balances.pdt = parsePDTBalances(json["pdt"]);
    }
    
    return balances;
}

Position parsePosition(const nlohmann::json& json) {
    Position position;
    
    position.costBasis = json.value("cost_basis", 0.0);
    
    if (json.contains("date_acquired") && !json["date_acquired"].is_null()) {
        position.dateAcquired = parseISODateTime(json["date_acquired"]);
    }
    
    position.id = json.value("id", 0);
    position.quantity = json.value("quantity", 0.0);
    position.symbol = json.value("symbol", "");
    
    return position;
}

Positions parsePositions(const nlohmann::json& json) {
    Positions positions;
    
    if (json.contains("position")) {
        auto positionsJson = json["position"];
        
        if (positionsJson.is_array()) {
            for (const auto& positionJson : positionsJson) {
                positions.push_back(parsePosition(positionJson));
            }
        } else if (positionsJson.is_object()) {
            positions.push_back(parsePosition(positionsJson));
        }
    }
    
    return positions;
}

OrderLeg parseOrderLeg(const nlohmann::json& json) {
    OrderLeg leg;
    
    leg.id = json.value("id", 0);
    leg.type = json.value("type", "");
    leg.symbol = json.value("symbol", "");
    leg.side = json.value("side", "");
    leg.quantity = json.value("quantity", 0.0);
    leg.status = json.value("status", "");
    leg.duration = json.value("duration", "");
    leg.price = json.value("price", 0.0);
    leg.avgFillPrice = json.value("avg_fill_price", 0.0);
    leg.execQuantity = json.value("exec_quantity", 0.0);
    leg.lastFillPrice = json.value("last_fill_price", 0.0);
    leg.lastFillQuantity = json.value("last_fill_quantity", 0.0);
    leg.remainingQuantity = json.value("remaining_quantity", 0.0);
    
    if (json.contains("create_date") && !json["create_date"].is_null()) {
        leg.createDate = parseISODateTime(json["create_date"]);
    }
    
    if (json.contains("transaction_date") && !json["transaction_date"].is_null()) {
        leg.transactionDate = parseISODateTime(json["transaction_date"]);
    }
    
    leg.orderClass = json.value("class", "");
    
    if (json.contains("option_symbol")) {
        leg.optionSymbol = json.value("option_symbol", "");
    }
    
    return leg;
}

Order parseOrder(const nlohmann::json& json) {
    Order order;
    
    order.id = json.value("id", 0);
    order.type = json.value("type", "");
    order.symbol = json.value("symbol", "");
    order.side = json.value("side", "");
    order.quantity = json.value("quantity", 0.0);
    order.status = json.value("status", "");
    order.duration = json.value("duration", "");
    
    if (json.contains("price")) {
        order.price = json.value("price", 0.0);
    }
    
    order.avgFillPrice = json.value("avg_fill_price", 0.0);
    order.execQuantity = json.value("exec_quantity", 0.0);
    order.lastFillPrice = json.value("last_fill_price", 0.0);
    order.lastFillQuantity = json.value("last_fill_quantity", 0.0);
    order.remainingQuantity = json.value("remaining_quantity", 0.0);
    
    if (json.contains("create_date") && !json["create_date"].is_null()) {
        order.createDate = parseISODateTime(json["create_date"]);
    }
    
    if (json.contains("transaction_date") && !json["transaction_date"].is_null()) {
        order.transactionDate = parseISODateTime(json["transaction_date"]);
    }
    
    order.orderClass = json.value("class", "");
    
    if (json.contains("option_symbol")) {
        order.optionSymbol = json.value("option_symbol", "");
    }
    
    if (json.contains("num_legs")) {
        order.numLegs = json.value("num_legs", 0);
    }
    
    if (json.contains("strategy")) {
        order.strategy = json.value("strategy", "");
    }
    
    if (json.contains("leg")) {
        auto legsJson = json["leg"];
        
        if (legsJson.is_array()) {
            for (const auto& legJson : legsJson) {
                order.legs.push_back(parseOrderLeg(legJson));
            }
        } else if (legsJson.is_object()) {
            order.legs.push_back(parseOrderLeg(legsJson));
        }
    }
    
    if (json.contains("tag") && !json["tag"].is_null()) {
        order.tag = json["tag"];
    }
    
    return order;
}

Order parseSingleOrder(const nlohmann::json& json) {
    if (json.contains("order") && !json["order"].is_null()) {
        return parseOrder(json["order"]);
    }
    
    return parseOrder(json);
}

Orders parseOrders(const nlohmann::json& json) {
    Orders orders;
    
    if (json.contains("order")) {
        auto ordersJson = json["order"];
        
        if (ordersJson.is_array()) {
            for (const auto& orderJson : ordersJson) {
                orders.push_back(parseOrder(orderJson));
            }
        } else if (ordersJson.is_object()) {
            orders.push_back(parseOrder(ordersJson));
        }
    }
    
    return orders;
}

Trade parseTrade(const nlohmann::json& json) {
    Trade trade;
    
    trade.commission = json.value("commission", 0.0);
    trade.description = json.value("description", "");
    trade.price = json.value("price", 0.0);
    trade.quantity = json.value("quantity", 0.0);
    trade.symbol = json.value("symbol", "");
    trade.tradeType = json.value("trade_type", "");
    
    return trade;
}

Adjustment parseAdjustment(const nlohmann::json& json) {
    Adjustment adjustment;
    
    adjustment.description = json.value("description", "");
    adjustment.quantity = json.value("quantity", 0.0);
    
    return adjustment;
}

OptionEvent parseOptionEvent(const nlohmann::json& json) {
    OptionEvent optionEvent;
    
    optionEvent.optionType = json.value("option_type", "");
    optionEvent.description = json.value("description", "");
    optionEvent.quantity = json.value("quantity", 0.0);
    
    return optionEvent;
}

Journal parseJournal(const nlohmann::json& json) {
    Journal journal;
    
    journal.description = json.value("description", "");
    journal.quantity = json.value("quantity", 0.0);
    
    return journal;
}

HistoryEvent parseHistoryEvent(const nlohmann::json& json) {
    HistoryEvent event;
    
    event.amount = json.value("amount", 0.0);
    
    if (json.contains("date") && !json["date"].is_null()) {
        event.date = parseISODateTime(json["date"]);
    }
    
    event.type = json.value("type", "");
    
    if (event.type == "trade" && json.contains("trade")) {
        event.trade = parseTrade(json["trade"]);
    } else if (event.type == "dividend" && json.contains("adjustment")) {
        event.adjustment = parseAdjustment(json["adjustment"]);
    } else if (event.type == "option" && json.contains("option")) {
        event.option = parseOptionEvent(json["option"]);
    } else if (event.type == "journal" && json.contains("journal")) {
        event.journal = parseJournal(json["journal"]);
    }
    
    return event;
}

History parseHistory(const nlohmann::json& json) {
    History history;
    
    if (json.contains("event")) {
        auto eventsJson = json["event"];
        
        if (eventsJson.is_array()) {
            for (const auto& eventJson : eventsJson) {
                history.push_back(parseHistoryEvent(eventJson));
            }
        } else if (eventsJson.is_object()) {
            history.push_back(parseHistoryEvent(eventsJson));
        }
    }
    
    return history;
}

ClosedPosition parseClosedPosition(const nlohmann::json& json) {
    ClosedPosition position;
    
    if (json.contains("close_date") && !json["close_date"].is_null()) {
        position.closeDate = parseISODateTime(json["close_date"]);
    }
    
    position.cost = json.value("cost", 0.0);
    position.gainLoss = json.value("gain_loss", 0.0);
    position.gainLossPercent = json.value("gain_loss_percent", 0.0);
    
    if (json.contains("open_date") && !json["open_date"].is_null()) {
        position.openDate = parseISODateTime(json["open_date"]);
    }
    
    position.proceeds = json.value("proceeds", 0.0);
    position.quantity = json.value("quantity", 0.0);
    position.symbol = json.value("symbol", "");
    position.term = json.value("term", 0);
    
    return position;
}

GainLoss parseGainLoss(const nlohmann::json& json) {
    GainLoss gainLoss;
    
    if (json.contains("closed_position")) {
        auto positionsJson = json["closed_position"];
        
        if (positionsJson.is_array()) {
            for (const auto& positionJson : positionsJson) {
                gainLoss.push_back(parseClosedPosition(positionJson));
            }
        } else if (positionsJson.is_object()) {
            gainLoss.push_back(parseClosedPosition(positionsJson));
        }
    }
    
    return gainLoss;
}

} // namespace json
} // namespace tradier
