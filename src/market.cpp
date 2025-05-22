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

#include "tradier/market.hpp"
#include "tradier/client.hpp"
#include "tradier/common/errors.hpp"
#include "tradier/common/json_utils.hpp"
#include "tradier/common/utils.hpp"

namespace tradier {

Result<Quote> MarketService::getQuote(const std::string& symbol) {
    if (symbol.empty()) {
        throw ValidationError("Symbol cannot be empty");
    }
    
    QueryParams params{{"symbols", symbol}};
    auto response = client_.get("/markets/quotes", params);
    
    return json::parseResponse<Quote>(response, [](const auto& json) {
        if (json.contains("quotes") && json["quotes"].contains("quote")) {
            return json::parseQuote(json["quotes"]["quote"]);
        }
        throw ApiError(400, "Invalid quote response format");
    });
}

Result<std::vector<Quote>> MarketService::getQuotes(const std::vector<std::string>& symbols) {
    if (symbols.empty()) {
        throw ValidationError("Symbols list cannot be empty");
    }
    
    std::string symbolsStr;
    for (size_t i = 0; i < symbols.size(); ++i) {
        if (i > 0) symbolsStr += ",";
        symbolsStr += symbols[i];
    }
    
    QueryParams params{{"symbols", symbolsStr}};
    auto response = client_.get("/markets/quotes", params);
    
    return json::parseResponse<std::vector<Quote>>(response, [](const auto& json) {
        if (json.contains("quotes")) {
            return json::parseQuotes(json["quotes"]);
        }
        return std::vector<Quote>{};
    });
}

Result<std::vector<HistoricalData>> MarketService::getHistory(
    const std::string& symbol,
    const std::string& interval,
    const std::string& start,
    const std::string& end) {
    
    if (symbol.empty()) {
        throw ValidationError("Symbol cannot be empty");
    }
    
    QueryParams params{{"symbol", symbol}};
    if (!interval.empty()) params["interval"] = interval;
    if (!start.empty()) params["start"] = start;
    if (!end.empty()) params["end"] = end;
    
    auto response = client_.get("/markets/history", params);
    
    return json::parseResponse<std::vector<HistoricalData>>(response, [symbol](const auto& json) {
        std::vector<HistoricalData> history;
        
        if (json.contains("history") && json["history"].contains("day")) {
            const auto& days = json["history"]["day"];
            
            if (days.is_array()) {
                for (const auto& day : days) {
                    HistoricalData data;
                    data.symbol = symbol;
                    data.date = json::parseDateTime(day, "date");
                    data.open = day.value("open", 0.0);
                    data.high = day.value("high", 0.0);
                    data.low = day.value("low", 0.0);
                    data.close = day.value("close", 0.0);
                    data.volume = day.value("volume", 0L);
                    history.push_back(data);
                }
            }
        }
        
        return history;
    });
}

Result<std::vector<OptionChain>> MarketService::getOptionChain(const std::string& symbol) {
    if (symbol.empty()) {
        throw ValidationError("Symbol cannot be empty");
    }
    
    QueryParams params{{"symbol", symbol}};
    auto response = client_.get("/markets/options/chains", params);
    
    return json::parseResponse<std::vector<OptionChain>>(response, [symbol](const auto& json) {
        std::vector<OptionChain> chain;
        
        if (json.contains("options") && json["options"].contains("option")) {
            const auto& options = json["options"]["option"];
            
            if (options.is_array()) {
                for (const auto& option : options) {
                    OptionChain opt;
                    opt.symbol = symbol;
                    opt.expiration = json::parseDateTime(option, "expiration_date");
                    opt.strike = option.value("strike", 0.0);
                    opt.type = option.value("option_type", "");
                    opt.bid = option.value("bid", 0.0);
                    opt.ask = option.value("ask", 0.0);
                    opt.volume = option.value("volume", 0);
                    opt.openInterest = option.value("open_interest", 0);
                    chain.push_back(opt);
                }
            }
        }
        
        return chain;
    });
}

Result<MarketClock> MarketService::getClock() {
    auto response = client_.get("/markets/clock");
    
    return json::parseResponse<MarketClock>(response, [](const auto& json) {
        if (!json.contains("clock")) {
            throw ApiError(400, "Invalid clock response format");
        }
        
        const auto& clock = json["clock"];
        MarketClock marketClock;
        marketClock.state = clock.value("state", "");
        marketClock.timestamp = json::parseDateTime(clock, "date");
        marketClock.nextChange = json::parseDateTime(clock, "next_change");
        marketClock.nextState = clock.value("next_state", "");
        marketClock.description = clock.value("description", "");
        
        return marketClock;
    });
}

Result<std::vector<std::string>> MarketService::searchSymbols(const std::string& query) {
    if (query.empty()) {
        throw ValidationError("Search query cannot be empty");
    }
    
    QueryParams params{{"q", query}};
    auto response = client_.get("/markets/search", params);
    
    return json::parseResponse<std::vector<std::string>>(response, [](const auto& json) {
        std::vector<std::string> symbols;
        
        if (json.contains("securities") && json["securities"].contains("security")) {
            const auto& securities = json["securities"]["security"];
            
            if (securities.is_array()) {
                for (const auto& security : securities) {
                    if (security.contains("symbol")) {
                        symbols.push_back(security["symbol"]);
                    }
                }
            } else if (securities.is_object() && securities.contains("symbol")) {
                symbols.push_back(securities["symbol"]);
            }
        }
        
        return symbols;
    });
}

}