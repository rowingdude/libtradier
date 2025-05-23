#include "tradier/json/market.hpp"
#include "tradier/common/utils.hpp"
#include <chrono>

namespace tradier {
namespace json {

Greeks parseGreeks(const nlohmann::json& json) {
    Greeks greeks;
    greeks.delta = json.value("delta", 0.0);
    greeks.gamma = json.value("gamma", 0.0);
    greeks.theta = json.value("theta", 0.0);
    greeks.vega = json.value("vega", 0.0);
    greeks.rho = json.value("rho", 0.0);
    greeks.phi = json.value("phi", 0.0);
    greeks.bidIv = json.value("bid_iv", 0.0);
    greeks.midIv = json.value("mid_iv", 0.0);
    greeks.askIv = json.value("ask_iv", 0.0);
    greeks.smvVol = json.value("smv_vol", 0.0);
    
    if (json.contains("updated_at") && !json["updated_at"].is_null()) {
        greeks.updatedAt = utils::parseISODateTime(json["updated_at"]);
    }
    
    return greeks;
}

Quote parseQuote(const nlohmann::json& json) {
    Quote quote;
    quote.symbol = json.value("symbol", "");
    quote.description = json.value("description", "");
    quote.exchange = json.value("exch", "");
    quote.type = json.value("type", "");
    
    if (json.contains("last") && !json["last"].is_null()) {
        quote.last = json["last"];
    }
    if (json.contains("change") && !json["change"].is_null()) {
        quote.change = json["change"];
    }
    
    quote.volume = json.value("volume", 0);
    
    if (json.contains("open") && !json["open"].is_null()) {
        quote.open = json["open"];
    }
    if (json.contains("high") && !json["high"].is_null()) {
        quote.high = json["high"];
    }
    if (json.contains("low") && !json["low"].is_null()) {
        quote.low = json["low"];
    }
    if (json.contains("close") && !json["close"].is_null()) {
        quote.close = json["close"];
    }
    
    quote.bid = json.value("bid", 0.0);
    quote.ask = json.value("ask", 0.0);
    
    if (json.contains("change_percentage") && !json["change_percentage"].is_null()) {
        quote.changePercentage = json["change_percentage"];
    }
    
    quote.averageVolume = json.value("average_volume", 0);
    quote.lastVolume = json.value("last_volume", 0);
    
    if (json.contains("trade_date") && json["trade_date"].get<long>() > 0) {
        quote.tradeDate = std::chrono::system_clock::from_time_t(json["trade_date"].get<long>() / 1000);
    }
    
    if (json.contains("prevclose") && !json["prevclose"].is_null()) {
        quote.prevClose = json["prevclose"];
    }
    
    quote.week52High = json.value("week_52_high", 0.0);
    quote.week52Low = json.value("week_52_low", 0.0);
    quote.bidSize = json.value("bidsize", 0);
    quote.bidExchange = json.value("bidexch", "");
    
    if (json.contains("bid_date") && json["bid_date"].get<long>() > 0) {
        quote.bidDate = std::chrono::system_clock::from_time_t(json["bid_date"].get<long>() / 1000);
    }
    
    quote.askSize = json.value("asksize", 0);
    quote.askExchange = json.value("askexch", "");
    
    if (json.contains("ask_date") && json["ask_date"].get<long>() > 0) {
        quote.askDate = std::chrono::system_clock::from_time_t(json["ask_date"].get<long>() / 1000);
    }
    
    quote.rootSymbols = json.value("root_symbols", "");
    
    // Option-specific fields
    if (json.contains("underlying")) {
        quote.underlying = json.value("underlying", "");
    }
    if (json.contains("strike")) {
        quote.strike = json.value("strike", 0.0);
    }
    if (json.contains("open_interest")) {
        quote.openInterest = json.value("open_interest", 0);
    }
    if (json.contains("contract_size")) {
        quote.contractSize = json.value("contract_size", 100);
    }
    if (json.contains("expiration_date")) {
        quote.expirationDate = json.value("expiration_date", "");
    }
    if (json.contains("expiration_type")) {
        quote.expirationType = json.value("expiration_type", "");
    }
    if (json.contains("option_type")) {
        quote.optionType = json.value("option_type", "");
    }
    if (json.contains("root_symbol")) {
        quote.rootSymbol = json.value("root_symbol", "");
    }
    if (json.contains("greeks")) {
        quote.greeks = parseGreeks(json["greeks"]);
    }
    
    return quote;
}

std::vector<Quote> parseQuotes(const nlohmann::json& json) {
    std::vector<Quote> quotes;
    
    if (json.contains("quotes") && json["quotes"].contains("quote")) {
        const auto& quotesJson = json["quotes"]["quote"];
        
        if (quotesJson.is_array()) {
            for (const auto& item : quotesJson) {
                quotes.push_back(parseQuote(item));
            }
        } else if (quotesJson.is_object()) {
            quotes.push_back(parseQuote(quotesJson));
        }
    }
    
    return quotes;
}

OptionChain parseOptionChain(const nlohmann::json& json) {
    OptionChain option;
    option.symbol = json.value("symbol", "");
    option.description = json.value("description", "");
    option.exchange = json.value("exch", "");
    option.type = json.value("type", "");
    
    if (json.contains("last") && !json["last"].is_null()) {
        option.last = json["last"];
    }
    if (json.contains("change") && !json["change"].is_null()) {
        option.change = json["change"];
    }
    
    option.volume = json.value("volume", 0);
    
    if (json.contains("open") && !json["open"].is_null()) {
        option.open = json["open"];
    }
    if (json.contains("high") && !json["high"].is_null()) {
        option.high = json["high"];
    }
    if (json.contains("low") && !json["low"].is_null()) {
        option.low = json["low"];
    }
    if (json.contains("close") && !json["close"].is_null()) {
        option.close = json["close"];
    }
    
    option.bid = json.value("bid", 0.0);
    option.ask = json.value("ask", 0.0);
    option.underlying = json.value("underlying", "");
    option.strike = json.value("strike", 0.0);
    
    if (json.contains("change_percentage") && !json["change_percentage"].is_null()) {
        option.changePercentage = json["change_percentage"];
    }
    
    option.averageVolume = json.value("average_volume", 0);
    option.lastVolume = json.value("last_volume", 0);
    
    if (json.contains("trade_date") && json["trade_date"].get<long>() > 0) {
        option.tradeDate = std::chrono::system_clock::from_time_t(json["trade_date"].get<long>() / 1000);
    }
    
    if (json.contains("prevclose") && !json["prevclose"].is_null()) {
        option.prevClose = json["prevclose"];
    }
    
    option.week52High = json.value("week_52_high", 0.0);
    option.week52Low = json.value("week_52_low", 0.0);
    option.bidSize = json.value("bidsize", 0);
    option.bidExchange = json.value("bidexch", "");
    
    if (json.contains("bid_date") && json["bid_date"].get<long>() > 0) {
        option.bidDate = std::chrono::system_clock::from_time_t(json["bid_date"].get<long>() / 1000);
    }
    
    option.askSize = json.value("asksize", 0);
    option.askExchange = json.value("askexch", "");
    
    if (json.contains("ask_date") && json["ask_date"].get<long>() > 0) {
        option.askDate = std::chrono::system_clock::from_time_t(json["ask_date"].get<long>() / 1000);
    }
    
    option.openInterest = json.value("open_interest", 0);
    option.contractSize = json.value("contract_size", 100);
    option.expirationDate = json.value("expiration_date", "");
    option.expirationType = json.value("expiration_type", "");
    option.optionType = json.value("option_type", "");
    option.rootSymbol = json.value("root_symbol", "");
    
    if (json.contains("greeks")) {
        option.greeks = parseGreeks(json["greeks"]);
    }
    
    return option;
}

std::vector<OptionChain> parseOptionChains(const nlohmann::json& json) {
    std::vector<OptionChain> options;
    
    if (json.contains("options") && json["options"].contains("option")) {
        const auto& optionsJson = json["options"]["option"];
        
        if (optionsJson.is_array()) {
            for (const auto& item : optionsJson) {
                options.push_back(parseOptionChain(item));
            }
        } else if (optionsJson.is_object()) {
            options.push_back(parseOptionChain(optionsJson));
        }
    }
    
    return options;
}

std::vector<double> parseStrikes(const nlohmann::json& json) {
    std::vector<double> strikes;
    
    if (json.contains("strikes") && json["strikes"].contains("strike")) {
        const auto& strikesJson = json["strikes"]["strike"];
        
        if (strikesJson.is_array()) {
            for (const auto& item : strikesJson) {
                strikes.push_back(item.get<double>());
            }
        } else if (strikesJson.is_number()) {
            strikes.push_back(strikesJson.get<double>());
        }
    }
    
    return strikes;
}

Expiration parseExpiration(const nlohmann::json& json) {
    Expiration expiration;
    expiration.date = json.value("date", "");
    expiration.contractSize = json.value("contract_size", 100);
    expiration.expirationType = json.value("expiration_type", "");
    
    if (json.contains("strikes") && json["strikes"].contains("strike")) {
        const auto& strikesJson = json["strikes"]["strike"];
        
        if (strikesJson.is_array()) {
            for (const auto& item : strikesJson) {
                expiration.strikes.push_back(item.get<double>());
            }
        } else if (strikesJson.is_number()) {
            expiration.strikes.push_back(strikesJson.get<double>());
        }
    }
    
    return expiration;
}

std::vector<Expiration> parseExpirations(const nlohmann::json& json) {
    std::vector<Expiration> expirations;
    
    if (json.contains("expirations") && json["expirations"].contains("expiration")) {
        const auto& expirationsJson = json["expirations"]["expiration"];
        
        if (expirationsJson.is_array()) {
            for (const auto& item : expirationsJson) {
                expirations.push_back(parseExpiration(item));
            }
        } else if (expirationsJson.is_object()) {
            expirations.push_back(parseExpiration(expirationsJson));
        }
    }
    
    return expirations;
}

OptionSymbol parseOptionSymbol(const nlohmann::json& json) {
    OptionSymbol optionSymbol;
    optionSymbol.rootSymbol = json.value("rootSymbol", "");
    
    if (json.contains("options") && json["options"].is_array()) {
        for (const auto& item : json["options"]) {
            optionSymbol.options.push_back(item.get<std::string>());
        }
    }
    
    return optionSymbol;
}

std::vector<OptionSymbol> parseOptionSymbols(const nlohmann::json& json) {
    std::vector<OptionSymbol> optionSymbols;
    
    if (json.contains("symbols") && json["symbols"].is_array()) {
        for (const auto& item : json["symbols"]) {
            optionSymbols.push_back(parseOptionSymbol(item));
        }
    }
    
    return optionSymbols;
}

HistoricalData parseHistoricalData(const nlohmann::json& json) {
    HistoricalData data;
    data.date = json.value("date", "");
    data.open = json.value("open", 0.0);
    data.high = json.value("high", 0.0);
    data.low = json.value("low", 0.0);
    data.close = json.value("close", 0.0);
    data.volume = json.value("volume", 0L);
    return data;
}

std::vector<HistoricalData> parseHistoricalDataList(const nlohmann::json& json) {
    std::vector<HistoricalData> history;
    
    if (json.contains("history") && json["history"].contains("day")) {
        const auto& daysJson = json["history"]["day"];
        
        if (daysJson.is_array()) {
            for (const auto& item : daysJson) {
                history.push_back(parseHistoricalData(item));
            }
        } else if (daysJson.is_object()) {
            history.push_back(parseHistoricalData(daysJson));
        }
    }
    
    return history;
}

TimeSalesData parseTimeSalesData(const nlohmann::json& json) {
    TimeSalesData data;
    data.time = json.value("time", "");
    data.timestamp = json.value("timestamp", 0L);
    data.price = json.value("price", 0.0);
    data.open = json.value("open", 0.0);
    data.high = json.value("high", 0.0);
    data.low = json.value("low", 0.0);
    data.close = json.value("close", 0.0);
    data.volume = json.value("volume", 0L);
    data.vwap = json.value("vwap", 0.0);
    return data;
}

std::vector<TimeSalesData> parseTimeSalesList(const nlohmann::json& json) {
    std::vector<TimeSalesData> timeSales;
    
    if (json.contains("series") && json["series"].contains("data")) {
        const auto& dataJson = json["series"]["data"];
        
        if (dataJson.is_array()) {
            for (const auto& item : dataJson) {
                timeSales.push_back(parseTimeSalesData(item));
            }
        } else if (dataJson.is_object()) {
            timeSales.push_back(parseTimeSalesData(dataJson));
        }
    }
    
    return timeSales;
}

Security parseSecurity(const nlohmann::json& json) {
    Security security;
    security.symbol = json.value("symbol", "");
    security.exchange = json.value("exchange", "");
    security.type = json.value("type", "");
    security.description = json.value("description", "");
    return security;
}

std::vector<Security> parseSecurities(const nlohmann::json& json) {
    std::vector<Security> securities;
    
    if (json.contains("securities") && json["securities"].contains("security")) {
        const auto& securitiesJson = json["securities"]["security"];
        
        if (securitiesJson.is_array()) {
            for (const auto& item : securitiesJson) {
                securities.push_back(parseSecurity(item));
            }
        } else if (securitiesJson.is_object()) {
            securities.push_back(parseSecurity(securitiesJson));
        }
    }
    
    return securities;
}

SessionTime parseSessionTime(const nlohmann::json& json) {
    SessionTime session;
    session.start = json.value("start", "");
    session.end = json.value("end", "");
    return session;
}

MarketDay parseMarketDay(const nlohmann::json& json) {
    MarketDay day;
    day.date = json.value("date", "");
    day.status = json.value("status", "");
    day.description = json.value("description", "");
    
    if (json.contains("premarket")) {
        day.premarket = parseSessionTime(json["premarket"]);
    }
    if (json.contains("open")) {
        day.open = parseSessionTime(json["open"]);
    }
    if (json.contains("postmarket")) {
        day.postmarket = parseSessionTime(json["postmarket"]);
    }
    
    return day;
}

MarketCalendar parseMarketCalendar(const nlohmann::json& json) {
    MarketCalendar calendar;
    
    if (json.contains("calendar")) {
        const auto& cal = json["calendar"];
        calendar.month = cal.value("month", 0);
        calendar.year = cal.value("year", 0);
        
        if (cal.contains("days") && cal["days"].contains("day")) {
            const auto& daysJson = cal["days"]["day"];
            
            if (daysJson.is_array()) {
                for (const auto& item : daysJson) {
                    calendar.days.push_back(parseMarketDay(item));
                }
            } else if (daysJson.is_object()) {
                calendar.days.push_back(parseMarketDay(daysJson));
            }
        }
    }
    
    return calendar;
}

MarketClock parseMarketClock(const nlohmann::json& json) {
    MarketClock clock;
    
    if (json.contains("clock")) {
        const auto& clockJson = json["clock"];
        clock.date = clockJson.value("date", "");
        clock.description = clockJson.value("description", "");
        clock.state = clockJson.value("state", "");
        clock.timestamp = clockJson.value("timestamp", 0L);
        clock.nextChange = clockJson.value("next_change", "");
        clock.nextState = clockJson.value("next_state", "");
    }
    
    return clock;
}

} // namespace json
} // namespace tradier