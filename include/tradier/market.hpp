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

#include <vector>
#include <string>
#include <optional>
#include <chrono>
#include "tradier/common/types.hpp"

namespace tradier {

class TradierClient;

struct Greeks {
    double delta = 0.0;
    double gamma = 0.0;
    double theta = 0.0;
    double vega = 0.0;
    double rho = 0.0;
    double phi = 0.0;
    double bidIv = 0.0;
    double midIv = 0.0;
    double askIv = 0.0;
    double smvVol = 0.0;
    TimePoint updatedAt;
};

struct Quote {
    std::string symbol;
    std::string description;
    std::string exchange;
    std::string type;
    std::optional<double> last;
    std::optional<double> change;
    int volume = 0;
    std::optional<double> open;
    std::optional<double> high;
    std::optional<double> low;
    std::optional<double> close;
    double bid = 0.0;
    double ask = 0.0;
    std::optional<double> changePercentage;
    int averageVolume = 0;
    int lastVolume = 0;
    TimePoint tradeDate;
    TimePoint timestamp;
    std::optional<double> prevClose;
    double week52High = 0.0;
    double week52Low = 0.0;
    int bidSize = 0;
    std::string bidExchange;
    TimePoint bidDate;
    int askSize = 0;
    std::string askExchange;
    TimePoint askDate;
    std::string rootSymbols;
    
    // Option-specific fields
    std::optional<std::string> underlying;
    std::optional<double> strike;
    std::optional<int> openInterest;
    std::optional<int> contractSize;
    std::optional<std::string> expirationDate;
    std::optional<std::string> expirationType;
    std::optional<std::string> optionType;
    std::optional<std::string> rootSymbol;
    std::optional<Greeks> greeks;
};

struct OptionChain {
    std::string symbol;
    std::string description;
    std::string exchange;
    std::string type;
    std::optional<double> last;
    std::optional<double> change;
    int volume = 0;
    std::optional<double> open;
    std::optional<double> high;
    std::optional<double> low;
    std::optional<double> close;
    double bid = 0.0;
    double ask = 0.0;
    std::string underlying;
    double strike = 0.0;
    std::optional<double> changePercentage;
    int averageVolume = 0;
    int lastVolume = 0;
    TimePoint tradeDate;
    std::optional<double> prevClose;
    double week52High = 0.0;
    double week52Low = 0.0;
    int bidSize = 0;
    std::string bidExchange;
    TimePoint bidDate;
    int askSize = 0;
    std::string askExchange;
    TimePoint askDate;
    int openInterest = 0;
    int contractSize = 100;
    std::string expirationDate;
    std::string expirationType;
    std::string optionType;
    std::string rootSymbol;
    std::optional<Greeks> greeks;
};

struct Strike {
    double value;
};

struct Expiration {
    std::string date;
    int contractSize = 100;
    std::string expirationType;
    std::vector<double> strikes;
};

struct OptionSymbol {
    std::string rootSymbol;
    std::vector<std::string> options;
};

struct HistoricalData {
    std::string date;
    double open = 0.0;
    double high = 0.0;
    double low = 0.0;
    double close = 0.0;
    long volume = 0;
};

struct TimeSalesData {
    std::string time;
    long timestamp = 0;
    double price = 0.0;
    double open = 0.0;
    double high = 0.0;
    double low = 0.0;
    double close = 0.0;
    long volume = 0;
    double vwap = 0.0;
};

struct Security {
    std::string symbol;
    std::string exchange;
    std::string type;
    std::string description;
};

struct SessionTime {
    std::string start;
    std::string end;
};

struct MarketDay {
    std::string date;
    std::string status;
    std::string description;
    SessionTime premarket;
    SessionTime open;
    SessionTime postmarket;
};

struct MarketCalendar {
    int month;
    int year;
    std::vector<MarketDay> days;
};

struct MarketClock {
    std::string date;
    std::string description;
    std::string state;
    long timestamp = 0;
    std::string nextChange;
    std::string nextState;
};

class MarketService {
private:
    TradierClient& client_;
    
public:
    explicit MarketService(TradierClient& client) : client_(client) {}
    
    // Quotes
    Result<std::vector<Quote>> getQuotes(const std::vector<std::string>& symbols, bool greeks = false);
    Result<std::vector<Quote>> getQuotesPost(const std::vector<std::string>& symbols, bool greeks = false);
    Result<Quote> getQuote(const std::string& symbol, bool greeks = false);
    
    // Options
    Result<std::vector<OptionChain>> getOptionChain(const std::string& symbol, const std::string& expiration, bool greeks = false);
    Result<std::vector<double>> getOptionStrikes(const std::string& symbol, const std::string& expiration, bool includeAllRoots = false);
    Result<std::vector<Expiration>> getOptionExpirations(const std::string& symbol, bool includeAllRoots = false, bool strikes = false, bool contractSize = false, bool expirationType = false);
    Result<std::vector<OptionSymbol>> lookupOptionSymbols(const std::string& underlying);
    
    // Historical Data
    Result<std::vector<HistoricalData>> getHistoricalData(const std::string& symbol, const std::string& interval = "daily", const std::string& start = "", const std::string& end = "", const std::string& sessionFilter = "all");
    Result<std::vector<TimeSalesData>> getTimeSales(const std::string& symbol, const std::string& interval = "1min", const std::string& start = "", const std::string& end = "", const std::string& sessionFilter = "all");
    
    // Market Info
    Result<std::vector<Security>> getETBList();
    Result<MarketClock> getClock(bool delayed = false);
    Result<MarketCalendar> getCalendar(const std::string& month = "", const std::string& year = "");
    Result<std::vector<Security>> searchSymbols(const std::string& query, bool indexes = true);
    Result<std::vector<Security>> lookupSymbols(const std::string& query, const std::string& exchanges = "", const std::string& types = "");
};

}