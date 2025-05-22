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

#include "tradier/json/streaming.hpp"
#include "tradier/streaming.hpp"
#include "tradier/common/utils.hpp"

namespace tradier {
namespace json {

MarketQuote parseMarketQuote(const nlohmann::json& json) {
    MarketQuote quote;
    
    quote.type = json.value("type", "");
    quote.symbol = json.value("symbol", "");
    quote.bid = json.value("bid", 0.0);
    quote.bidSize = json.value("bidsz", 0);
    quote.bidExchange = json.value("bidexch", "");
    
    if (json.contains("biddate") && !json["biddate"].is_null()) {
        std::string bidDateStr = json["biddate"];
        if (!bidDateStr.empty()) {
            auto timestamp = std::stoll(bidDateStr);
            quote.bidDate = std::chrono::system_clock::from_time_t(timestamp / 1000);
        }
    }
    
    quote.ask = json.value("ask", 0.0);
    quote.askSize = json.value("asksz", 0);
    quote.askExchange = json.value("askexch", "");
    
    if (json.contains("askdate") && !json["askdate"].is_null()) {
        std::string askDateStr = json["askdate"];
        if (!askDateStr.empty()) {
            auto timestamp = std::stoll(askDateStr);
            quote.askDate = std::chrono::system_clock::from_time_t(timestamp / 1000);
        }
    }
    
    return quote;
}

MarketTrade parseMarketTrade(const nlohmann::json& json) {
    MarketTrade trade;
    
    trade.type = json.value("type", "");
    trade.symbol = json.value("symbol", "");
    trade.exchange = json.value("exch", "");
    
    if (json.contains("price")) {
        if (json["price"].is_string()) {
            trade.price = std::stod(json["price"].get<std::string>());
        } else {
            trade.price = json.value("price", 0.0);
        }
    }
    
    if (json.contains("size")) {
        if (json["size"].is_string()) {
            trade.size = std::stoi(json["size"].get<std::string>());
        } else {
            trade.size = json.value("size", 0);
        }
    }
    
    if (json.contains("cvol")) {
        if (json["cvol"].is_string()) {
            trade.cumulativeVolume = std::stoll(json["cvol"].get<std::string>());
        } else {
            trade.cumulativeVolume = json.value("cvol", 0L);
        }
    }
    
    if (json.contains("date") && !json["date"].is_null()) {
        std::string dateStr = json["date"];
        if (!dateStr.empty()) {
            auto timestamp = std::stoll(dateStr);
            trade.date = std::chrono::system_clock::from_time_t(timestamp / 1000);
        }
    }
    
    if (json.contains("last")) {
        if (json["last"].is_string()) {
            trade.last = std::stod(json["last"].get<std::string>());
        } else {
            trade.last = json.value("last", 0.0);
        }
    }
    
    return trade;
}

MarketSummary parseMarketSummary(const nlohmann::json& json) {
    MarketSummary summary;
    
    summary.type = json.value("type", "");
    summary.symbol = json.value("symbol", "");
    
    if (json.contains("open")) {
        if (json["open"].is_string()) {
            summary.open = std::stod(json["open"].get<std::string>());
        } else {
            summary.open = json.value("open", 0.0);
        }
    }
    
    if (json.contains("high")) {
        if (json["high"].is_string()) {
            summary.high = std::stod(json["high"].get<std::string>());
        } else {
            summary.high = json.value("high", 0.0);
        }
    }
    
    if (json.contains("low")) {
        if (json["low"].is_string()) {
            summary.low = std::stod(json["low"].get<std::string>());
        } else {
            summary.low = json.value("low", 0.0);
        }
    }
    
    if (json.contains("prevClose")) {
        if (json["prevClose"].is_string()) {
            summary.previousClose = std::stod(json["prevClose"].get<std::string>());
        } else {
            summary.previousClose = json.value("prevClose", 0.0);
        }
    }
    
    return summary;
}

MarketTimesale parseMarketTimesale(const nlohmann::json& json) {
    MarketTimesale timesale;
    
    timesale.type = json.value("type", "");
    timesale.symbol = json.value("symbol", "");
    timesale.exchange = json.value("exch", "");
    timesale.bid = json.value("bid", 0.0);
    timesale.ask = json.value("ask", 0.0);
    timesale.last = json.value("last", 0.0);
    timesale.size = json.value("size", 0);
    timesale.sequence = json.value("seq", 0);
    timesale.flag = json.value("flag", "");
    timesale.cancel = json.value("cancel", false);
    timesale.correction = json.value("correction", false);
    timesale.session = json.value("session", "");
    
    if (json.contains("date") && !json["date"].is_null()) {
        std::string dateStr = json["date"];
        if (!dateStr.empty()) {
            auto timestamp = std::stoll(dateStr);
            timesale.date = std::chrono::system_clock::from_time_t(timestamp / 1000);
        }
    }
    
    return timesale;
}

MarketTradex parseMarketTradex(const nlohmann::json& json) {
    MarketTradex tradex;
    
    tradex.type = json.value("type", "");
    tradex.symbol = json.value("symbol", "");
    tradex.exchange = json.value("exch", "");
    
    if (json.contains("price")) {
        if (json["price"].is_string()) {
            tradex.price = std::stod(json["price"].get<std::string>());
        } else {
            tradex.price = json.value("price", 0.0);
        }
    }
    
    if (json.contains("size")) {
        if (json["size"].is_string()) {
            tradex.size = std::stoi(json["size"].get<std::string>());
        } else {
            tradex.size = json.value("size", 0);
        }
    }
    
    if (json.contains("cvol")) {
        if (json["cvol"].is_string()) {
            tradex.cumulativeVolume = std::stoll(json["cvol"].get<std::string>());
        } else {
            tradex.cumulativeVolume = json.value("cvol", 0L);
        }
    }
    
    if (json.contains("date") && !json["date"].is_null()) {
        std::string dateStr = json["date"];
        if (!dateStr.empty()) {
            auto timestamp = std::stoll(dateStr);
            tradex.date = std::chrono::system_clock::from_time_t(timestamp / 1000);
        }
    }
    
    if (json.contains("last")) {
        if (json["last"].is_string()) {
            tradex.last = std::stod(json["last"].get<std::string>());
        } else {
            tradex.last = json.value("last", 0.0);
        }
    }
    
    return tradex;
}

AccountOrderEvent parseAccountOrderEvent(const nlohmann::json& json) {
    AccountOrderEvent event;
    
    event.id = json.value("id", 0);
    event.event = json.value("event", "");
    event.status = json.value("status", "");
    event.type = json.value("type", "");
    event.price = json.value("price", 0.0);
    event.stopPrice = json.value("stop_price", 0.0);
    event.avgFillPrice = json.value("avg_fill_price", 0.0);
    event.executedQuantity = json.value("executed_quantity", 0.0);
    event.lastFillQuantity = json.value("last_fill_quantity", 0.0);
    event.remainingQuantity = json.value("remaining_quantity", 0.0);
    event.account = json.value("account", "");
    
    if (json.contains("transaction_date") && !json["transaction_date"].is_null()) {
        event.transactionDate = parseISODateTime(json["transaction_date"]);
    }
    
    if (json.contains("create_date") && !json["create_date"].is_null()) {
        event.createDate = parseISODateTime(json["create_date"]);
    }
    
    return event;
}

} // namespace json
} // namespace tradier