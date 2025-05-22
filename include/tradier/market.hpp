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
#include "tradier/common/types.hpp"
#include "tradier/data.hpp"

namespace tradier {

class TradierClient;

struct HistoricalData {
    std::string symbol;
    TimePoint date;
    double open = 0.0;
    double high = 0.0;
    double low = 0.0;
    double close = 0.0;
    long volume = 0;
};

struct OptionChain {
    std::string symbol;
    TimePoint expiration;
    double strike = 0.0;
    std::string type;
    double bid = 0.0;
    double ask = 0.0;
    int volume = 0;
    int openInterest = 0;
};

struct MarketClock {
    std::string state;
    TimePoint timestamp;
    TimePoint nextChange;
    std::string nextState;
    std::string description;
};

class MarketService {
private:
    TradierClient& client_;
    
public:
    explicit MarketService(TradierClient& client) : client_(client) {}
    
    Result<Quote> getQuote(const std::string& symbol);
    Result<std::vector<Quote>> getQuotes(const std::vector<std::string>& symbols);
    Result<std::vector<HistoricalData>> getHistory(
        const std::string& symbol,
        const std::string& interval = "daily",
        const std::string& start = "",
        const std::string& end = ""
    );
    Result<std::vector<OptionChain>> getOptionChain(const std::string& symbol);
    Result<MarketClock> getClock();
    Result<std::vector<std::string>> searchSymbols(const std::string& query);
};

}