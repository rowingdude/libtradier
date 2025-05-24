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
#include "tradier/common/api_result.hpp"
#include "tradier/json/market.hpp"

#include <iostream>
#include <sstream>

namespace tradier {

Result<std::vector<Quote>> MarketService::getQuotes(const std::vector<std::string>& symbols, bool greeks) {
    return tryExecute<std::vector<Quote>>([&]() -> std::vector<Quote> {
        if (symbols.empty()) {
            throw ValidationError("Symbols list cannot be empty");
        }
        
        std::ostringstream symbolsStr;
        for (size_t i = 0; i < symbols.size(); ++i) {
            if (i > 0) symbolsStr << ",";
            symbolsStr << symbols[i];
        }
        
        QueryParams params;
        params["symbols"] = symbolsStr.str();
        params["greeks"] = greeks ? "true" : "false";
        
        auto response = client_.get("/markets/quotes", params);
        
        if (!response.success()) {
            throw ::tradier::ApiError(response.status, "Failed to get quotes: " + response.body);
        }
        
        auto parsed = json::parseResponse<std::vector<Quote>>(response, json::parseQuotes);
        if (!parsed) {
            throw std::runtime_error("Failed to parse quotes response");
        }
        
        return *parsed;
    }, "getQuotes");
}


Result<std::vector<Quote>> MarketService::getQuotesPost(const std::vector<std::string>& symbols, bool greeks) {
    return tryExecute<std::vector<Quote>>([&]() -> std::vector<Quote> {
        if (symbols.empty()) {
            throw ValidationError("Symbols list cannot be empty");
        }
        
        std::ostringstream symbolsStr;
        for (size_t i = 0; i < symbols.size(); ++i) {
            if (i > 0) symbolsStr << ",";
            symbolsStr << symbols[i];
        }
        
        FormParams params;
        params["symbols"] = symbolsStr.str();
        params["greeks"] = greeks ? "true" : "false";
        
        auto response = client_.post("/markets/quotes", params);
        
        if (!response.success()) {
            throw ::tradier::ApiError(response.status, "Failed to get quotes via POST: " + response.body);
        }
        
        auto parsed = json::parseResponse<std::vector<Quote>>(response, json::parseQuotes);
        if (!parsed) {
            throw std::runtime_error("Failed to parse quotes POST response");
        }
        
        return *parsed;
    }, "getQuotesPost");
}

Result<Quote> MarketService::getQuote(const std::string& symbol, bool greeks) {
    if (symbol.empty()) {
        return Result<Quote>::validationError("Symbol cannot be empty");
    }
    
    return getQuotes({symbol}, greeks)
        .andThen([symbol](const std::vector<Quote>& quotes) -> Result<Quote> {
            if (quotes.empty()) {
                return Result<Quote>::apiError(404, "No quote found for symbol: " + symbol);
            }
            return Result<Quote>(quotes.front());
        });
}

Result<std::vector<OptionChain>> MarketService::getOptionChain(const std::string& symbol, const std::string& expiration, bool greeks) {
    return tryExecute<std::vector<OptionChain>>([&]() -> std::vector<OptionChain> {
        if (symbol.empty()) {
            throw ValidationError("Symbol cannot be empty");
        }
        if (expiration.empty()) {
            throw ValidationError("Expiration date cannot be empty");
        }
        
        QueryParams params;
        params["symbol"] = symbol;
        params["expiration"] = expiration;
        params["greeks"] = greeks ? "true" : "false";
        
        auto response = client_.get("/markets/options/chains", params);
        
        if (!response.success()) {
            std::string errorMsg = "Failed to get option chain for " + symbol + " exp " + expiration;
            
            switch (response.status) {
                case 400:
                    errorMsg += " - Invalid symbol or expiration date";
                    break;
                case 401:
                    throw AuthenticationError("Invalid API credentials");
                case 404:
                    errorMsg += " - Symbol not found or no options available";
                    break;
                case 429:
                    errorMsg += " - Rate limit exceeded, please retry later";
                    break;
                default:
                    errorMsg += " - " + response.body;
                    break;
            }
            
            throw ::tradier::ApiError(response.status, errorMsg);
        }
        
        auto parsed = json::parseResponse<std::vector<OptionChain>>(response, json::parseOptionChains);
        if (!parsed) {
            throw std::runtime_error("Failed to parse option chain response for " + symbol);
        }
        
        return *parsed;
    }, "getOptionChain");
}

Result<std::vector<double>> MarketService::getOptionStrikes(const std::string& symbol, const std::string& expiration, bool includeAllRoots) {
    if (symbol.empty()) {
        throw ValidationError("Symbol cannot be empty");
    }
    if (expiration.empty()) {
        throw ValidationError("Expiration date cannot be empty");
    }
    
    QueryParams params;
    params["symbol"] = symbol;
    params["expiration"] = expiration;
    params["includeAllRoots"] = includeAllRoots ? "true" : "false";
    
    auto response = client_.get("/markets/options/strikes", params);
    return json::parseResponse<std::vector<double>>(response, json::parseStrikes);
}

Result<std::vector<Expiration>> MarketService::getOptionExpirations(const std::string& symbol, bool includeAllRoots, bool strikes, bool contractSize, bool expirationType) {
    if (symbol.empty()) {
        throw ValidationError("Symbol cannot be empty");
    }
    
    QueryParams params;
    params["symbol"] = symbol;
    params["includeAllRoots"] = includeAllRoots ? "true" : "false";
    params["strikes"] = strikes ? "true" : "false";
    params["contractSize"] = contractSize ? "true" : "false";
    params["expirationType"] = expirationType ? "true" : "false";
    
    auto response = client_.get("/markets/options/expirations", params);
    return json::parseResponse<std::vector<Expiration>>(response, json::parseExpirations);
}

Result<std::vector<OptionSymbol>> MarketService::lookupOptionSymbols(const std::string& underlying) {
    if (underlying.empty()) {
        throw ValidationError("Underlying symbol cannot be empty");
    }
    
    QueryParams params;
    params["underlying"] = underlying;
    
    auto response = client_.get("/markets/options/lookup", params);
    return json::parseResponse<std::vector<OptionSymbol>>(response, json::parseOptionSymbols);
}

Result<std::vector<HistoricalData>> MarketService::getHistoricalData(const std::string& symbol, const std::string& interval, const std::string& start, const std::string& end, const std::string& sessionFilter) {
    if (symbol.empty()) {
        throw ValidationError("Symbol cannot be empty");
    }
    
    QueryParams params;
    params["symbol"] = symbol;
    params["interval"] = interval;
    params["session_filter"] = sessionFilter;
    
    if (!start.empty()) {
        params["start"] = start;
    }
    if (!end.empty()) {
        params["end"] = end;
    }
    
    auto response = client_.get("/markets/history", params);
    return json::parseResponse<std::vector<HistoricalData>>(response, json::parseHistoricalDataList);
}

Result<std::vector<TimeSalesData>> MarketService::getTimeSales(const std::string& symbol, const std::string& interval, const std::string& start, const std::string& end, const std::string& sessionFilter) {
    if (symbol.empty()) {
        throw ValidationError("Symbol cannot be empty");
    }
    
    QueryParams params;
    params["symbol"] = symbol;
    params["interval"] = interval;
    params["session_filter"] = sessionFilter;
    
    if (!start.empty()) {
        params["start"] = start;
    }
    if (!end.empty()) {
        params["end"] = end;
    }
    
    auto response = client_.get("/markets/timesales", params);
    return json::parseResponse<std::vector<TimeSalesData>>(response, json::parseTimeSalesList);
}

Result<std::vector<Security>> MarketService::getETBList() {
    auto response = client_.get("/markets/etb");
    
    if (!response.success()) {
        std::cerr << "ETB HTTP failed: " << response.status << std::endl;
        return std::nullopt;
    }
    
    try {
        return json::parseResponse<std::vector<Security>>(response, json::parseSecurities);
    } catch (const std::exception& e) {
        std::cerr << "ETB parsing failed: " << e.what() << std::endl;
        return std::nullopt;
    }
}

Result<MarketClock> MarketService::getClock(bool delayed) {
    QueryParams params;
    if (delayed) {
        params["delayed"] = "true";
    }
    
    auto response = client_.get("/markets/clock", params);
    return json::parseResponse<MarketClock>(response, json::parseMarketClock);
}

Result<MarketCalendar> MarketService::getCalendar(const std::string& month, const std::string& year) {
    QueryParams params;
    if (!month.empty()) {
        params["month"] = month;
    }
    if (!year.empty()) {
        params["year"] = year;
    }
    
    auto response = client_.get("/markets/calendar", params);
    return json::parseResponse<MarketCalendar>(response, json::parseMarketCalendar);
}

Result<std::vector<Security>> MarketService::searchSymbols(const std::string& query, bool indexes) {
    if (query.empty()) {
        throw ValidationError("Search query cannot be empty");
    }
    
    QueryParams params;
    params["q"] = query;
    params["indexes"] = indexes ? "true" : "false";
    
    auto response = client_.get("/markets/search", params);
    return json::parseResponse<std::vector<Security>>(response, json::parseSecurities);
}

Result<std::vector<Security>> MarketService::lookupSymbols(const std::string& query, const std::string& exchanges, const std::string& types) {
    if (query.empty()) {
        throw ValidationError("Search query cannot be empty");
    }
    
    QueryParams params;
    params["q"] = query;
    
    if (!exchanges.empty()) {
        params["exchanges"] = exchanges;
    }
    if (!types.empty()) {
        params["types"] = types;
    }
    
    auto response = client_.get("/markets/lookup", params);
    return json::parseResponse<std::vector<Security>>(response, json::parseSecurities);
}


Result<CompanyFundamentals> MarketService::getCompanyInfo(const std::string& symbol) {
    if (symbol.empty()) {
        throw ValidationError("Symbol cannot be empty");
    }
    
    QueryParams params;
    params["symbols"] = symbol;
    
    auto response = client_.get("/beta/markets/fundamentals/company", params);
    return json::parseResponse<CompanyFundamentals>(response, json::parseCompanyFundamentals);
}

Result<std::vector<CorporateCalendarEvent>> MarketService::getCorporateCalendar(const std::string& symbol) {
    if (symbol.empty()) {
        throw ValidationError("Symbol cannot be empty");
    }
    
    QueryParams params;
    params["symbols"] = symbol;
    
    auto response = client_.get("/beta/markets/fundamentals/calendars", params);
    return json::parseResponse<std::vector<CorporateCalendarEvent>>(response, json::parseCorporateCalendar);
}

Result<std::vector<Dividend>> MarketService::getDividends(const std::string& symbol) {
    if (symbol.empty()) {
        throw ValidationError("Symbol cannot be empty");
    }
    
    QueryParams params;
    params["symbols"] = symbol;
    
    auto response = client_.get("/beta/markets/fundamentals/dividends", params);
    return json::parseResponse<std::vector<Dividend>>(response, json::parseDividends);
}

Result<CorporateActions> MarketService::getCorporateActions(const std::string& symbol) {
    if (symbol.empty()) {
        throw ValidationError("Symbol cannot be empty");
    }
    
    QueryParams params;
    params["symbols"] = symbol;
    
    auto response = client_.get("/beta/markets/fundamentals/corporate_actions", params);
    return json::parseResponse<CorporateActions>(response, json::parseCorporateActions);
}

Result<std::vector<FinancialRatios>> MarketService::getFinancialRatios(const std::string& symbol) {
    if (symbol.empty()) {
        throw ValidationError("Symbol cannot be empty");
    }
    
    QueryParams params;
    params["symbols"] = symbol;
    
    auto response = client_.get("/beta/markets/fundamentals/ratios", params);
    return json::parseResponse<std::vector<FinancialRatios>>(response, json::parseFinancialRatios);
}

Result<FinancialStatement> MarketService::getFinancialStatements(const std::string& symbol) {
    if (symbol.empty()) {
        throw ValidationError("Symbol cannot be empty");
    }
    
    QueryParams params;
    params["symbols"] = symbol;
    
    auto response = client_.get("/beta/markets/fundamentals/financials", params);
    return json::parseResponse<FinancialStatement>(response, json::parseFinancialStatements);
}

Result<PriceStatistics> MarketService::getPriceStatistics(const std::string& symbol) {
    if (symbol.empty()) {
        throw ValidationError("Symbol cannot be empty");
    }
    
    QueryParams params;
    params["symbols"] = symbol;
    
    auto response = client_.get("/beta/markets/fundamentals/statistics", params);
    return json::parseResponse<PriceStatistics>(response, json::parsePriceStatistics);
}

}