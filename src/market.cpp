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
        .andThen([symbol](const std::vector<Quote>& quotes) -> Quote {
            if (quotes.empty()) {
                throw ::tradier::ApiError(404, "No quote found for symbol: " + symbol);
            }
            return quotes.front();
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
            throw ::tradier::ApiError(response.status, "Failed to get option chain: " + response.body);
        }
        
        auto parsed = json::parseResponse<std::vector<OptionChain>>(response, json::parseOptionChains);
        if (!parsed) {
            throw std::runtime_error("Failed to parse option chain response");
        }
        
        return *parsed;
    }, "getOptionChain");
}

Result<std::vector<double>> MarketService::getOptionStrikes(const std::string& symbol, const std::string& expiration, bool includeAllRoots) {
    return tryExecute<std::vector<double>>([&]() -> std::vector<double> {
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
        
        if (!response.success()) {
            throw ::tradier::ApiError(response.status, "Failed to get option strikes: " + response.body);
        }
        
        auto parsed = json::parseResponse<std::vector<double>>(response, json::parseStrikes);
        if (!parsed) {
            throw std::runtime_error("Failed to parse option strikes response");
        }
        
        return *parsed;
    }, "getOptionStrikes");
}

Result<std::vector<Expiration>> MarketService::getOptionExpirations(const std::string& symbol, bool includeAllRoots, bool strikes, bool contractSize, bool expirationType) {
    return tryExecute<std::vector<Expiration>>([&]() -> std::vector<Expiration> {
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
        
        if (!response.success()) {
            throw ::tradier::ApiError(response.status, "Failed to get option expirations: " + response.body);
        }
        
        auto parsed = json::parseResponse<std::vector<Expiration>>(response, json::parseExpirations);
        if (!parsed) {
            throw std::runtime_error("Failed to parse option expirations response");
        }
        
        return *parsed;
    }, "getOptionExpirations");
}
Result<std::vector<OptionSymbol>> MarketService::lookupOptionSymbols(const std::string& underlying) {
    return tryExecute<std::vector<OptionSymbol>>([&]() -> std::vector<OptionSymbol> {
        if (underlying.empty()) {
            throw ValidationError("Underlying symbol cannot be empty");
        }
        
        QueryParams params;
        params["underlying"] = underlying;
        
        auto response = client_.get("/markets/options/lookup", params);
        
        if (!response.success()) {
            throw ::tradier::ApiError(response.status, "Failed to lookup option symbols: " + response.body);
        }
        
        auto parsed = json::parseResponse<std::vector<OptionSymbol>>(response, json::parseOptionSymbols);
        if (!parsed) {
            throw std::runtime_error("Failed to parse option symbols response");
        }
        
        return *parsed;
    }, "lookupOptionSymbols");
}

Result<std::vector<HistoricalData>> MarketService::getHistoricalData(const std::string& symbol, const std::string& interval, const std::string& start, const std::string& end, const std::string& sessionFilter) {
    return tryExecute<std::vector<HistoricalData>>([&]() -> std::vector<HistoricalData> {
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
        
        if (!response.success()) {
            throw ::tradier::ApiError(response.status, "Failed to get historical data: " + response.body);
        }
        
        auto parsed = json::parseResponse<std::vector<HistoricalData>>(response, json::parseHistoricalDataList);
        if (!parsed) {
            throw std::runtime_error("Failed to parse historical data response");
        }
        
        return *parsed;
    }, "getHistoricalData");
}

Result<std::vector<TimeSalesData>> MarketService::getTimeSales(const std::string& symbol, const std::string& interval, const std::string& start, const std::string& end, const std::string& sessionFilter) {
    return tryExecute<std::vector<TimeSalesData>>([&]() -> std::vector<TimeSalesData> {
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
        
        if (!response.success()) {
            throw ::tradier::ApiError(response.status, "Failed to get time sales data: " + response.body);
        }
        
        auto parsed = json::parseResponse<std::vector<TimeSalesData>>(response, json::parseTimeSalesList);
        if (!parsed) {
            throw std::runtime_error("Failed to parse time sales response");
        }
        
        return *parsed;
    }, "getTimeSales");
}

Result<std::vector<Security>> MarketService::getETBList() {
    return tryExecute<std::vector<Security>>([&]() -> std::vector<Security> {
        auto response = client_.get("/markets/etb");
        
        if (!response.success()) {
            throw ::tradier::ApiError(response.status, "Failed to get ETB list: " + response.body);
        }
        
        auto parsed = json::parseResponse<std::vector<Security>>(response, json::parseSecurities);
        if (!parsed) {
            throw std::runtime_error("Failed to parse ETB list response");
        }
        
        return *parsed;
    }, "getETBList");
}

Result<MarketClock> MarketService::getClock(bool delayed) {
    return tryExecute<MarketClock>([&]() -> MarketClock {
        QueryParams params;
        if (delayed) {
            params["delayed"] = "true";
        }
        
        auto response = client_.get("/markets/clock", params);
        
        if (!response.success()) {
            throw ::tradier::ApiError(response.status, "Failed to get market clock: " + response.body);
        }
        
        auto parsed = json::parseResponse<MarketClock>(response, json::parseMarketClock);
        if (!parsed) {
            throw std::runtime_error("Failed to parse market clock response");
        }
        
        return *parsed;
    }, "getClock");
}

Result<MarketCalendar> MarketService::getCalendar(const std::string& month, const std::string& year) {
    return tryExecute<MarketCalendar>([&]() -> MarketCalendar {
        QueryParams params;
        if (!month.empty()) {
            params["month"] = month;
        }
        if (!year.empty()) {
            params["year"] = year;
        }
        
        auto response = client_.get("/markets/calendar", params);
        
        if (!response.success()) {
            throw ::tradier::ApiError(response.status, "Failed to get market calendar: " + response.body);
        }
        
        auto parsed = json::parseResponse<MarketCalendar>(response, json::parseMarketCalendar);
        if (!parsed) {
            throw std::runtime_error("Failed to parse market calendar response");
        }
        
        return *parsed;
    }, "getCalendar");
}

Result<std::vector<Security>> MarketService::searchSymbols(const std::string& query, bool indexes) {
    return tryExecute<std::vector<Security>>([&]() -> std::vector<Security> {
        if (query.empty()) {
            throw ValidationError("Search query cannot be empty");
        }
        
        QueryParams params;
        params["q"] = query;
        params["indexes"] = indexes ? "true" : "false";
        
        auto response = client_.get("/markets/search", params);
        
        if (!response.success()) {
            throw ::tradier::ApiError(response.status, "Failed to search symbols: " + response.body);
        }
        
        auto parsed = json::parseResponse<std::vector<Security>>(response, json::parseSecurities);
        if (!parsed) {
            throw std::runtime_error("Failed to parse symbol search response");
        }
        
        return *parsed;
    }, "searchSymbols");
}

Result<std::vector<Security>> MarketService::lookupSymbols(const std::string& query, const std::string& exchanges, const std::string& types) {
    return tryExecute<std::vector<Security>>([&]() -> std::vector<Security> {
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
        
        if (!response.success()) {
            throw ::tradier::ApiError(response.status, "Failed to lookup symbols: " + response.body);
        }
        
        auto parsed = json::parseResponse<std::vector<Security>>(response, json::parseSecurities);
        if (!parsed) {
            throw std::runtime_error("Failed to parse symbol lookup response");
        }
        
        return *parsed;
    }, "lookupSymbols");
}

Result<CompanyFundamentals> MarketService::getCompanyInfo(const std::string& symbol) {
    return tryExecute<CompanyFundamentals>([&]() -> CompanyFundamentals {
        if (symbol.empty()) {
            throw ValidationError("Symbol cannot be empty");
        }
        
        QueryParams params;
        params["symbols"] = symbol;
        
        auto response = client_.get("/beta/markets/fundamentals/company", params);
        
        if (!response.success()) {
            if (response.status == 302) {
                throw ::tradier::ApiError(response.status, "Company fundamentals endpoint redirected - feature unavailable");
            }
            throw ::tradier::ApiError(response.status, "Failed to get company info: " + response.body);
        }
        
        auto parsed = json::parseResponse<CompanyFundamentals>(response, json::parseCompanyFundamentals);
        if (!parsed) {
            throw std::runtime_error("Failed to parse company fundamentals response");
        }
        
        return *parsed;
    }, "getCompanyInfo");
}

Result<std::vector<CorporateCalendarEvent>> MarketService::getCorporateCalendar(const std::string& symbol) {
    return tryExecute<std::vector<CorporateCalendarEvent>>([&]() -> std::vector<CorporateCalendarEvent> {
        if (symbol.empty()) {
            throw ValidationError("Symbol cannot be empty");
        }
        
        QueryParams params;
        params["symbols"] = symbol;
        
        auto response = client_.get("/beta/markets/fundamentals/calendars", params);
        
        if (!response.success()) {
            throw ::tradier::ApiError(response.status, "Failed to get corporate calendar: " + response.body);
        }
        
        auto parsed = json::parseResponse<std::vector<CorporateCalendarEvent>>(response, json::parseCorporateCalendar);
        if (!parsed) {
            throw std::runtime_error("Failed to parse corporate calendar response");
        }
        
        return *parsed;
    }, "getCorporateCalendar");
}

Result<std::vector<Dividend>> MarketService::getDividends(const std::string& symbol) {
    return tryExecute<std::vector<Dividend>>([&]() -> std::vector<Dividend> {
        if (symbol.empty()) {
            throw ValidationError("Symbol cannot be empty");
        }
        
        QueryParams params;
        params["symbols"] = symbol;
        
        auto response = client_.get("/beta/markets/fundamentals/dividends", params);
        
        if (!response.success()) {
            throw ::tradier::ApiError(response.status, "Failed to get dividends: " + response.body);
        }
        
        auto parsed = json::parseResponse<std::vector<Dividend>>(response, json::parseDividends);
        if (!parsed) {
            throw std::runtime_error("Failed to parse dividends response");
        }
        
        return *parsed;
    }, "getDividends");
}

Result<CorporateActions> MarketService::getCorporateActions(const std::string& symbol) {
    return tryExecute<CorporateActions>([&]() -> CorporateActions {
        if (symbol.empty()) {
            throw ValidationError("Symbol cannot be empty");
        }
        
        QueryParams params;
        params["symbols"] = symbol;
        
        auto response = client_.get("/beta/markets/fundamentals/corporate_actions", params);
        
        if (!response.success()) {
            throw ::tradier::ApiError(response.status, "Failed to get corporate actions: " + response.body);
        }
        
        auto parsed = json::parseResponse<CorporateActions>(response, json::parseCorporateActions);
        if (!parsed) {
            throw std::runtime_error("Failed to parse corporate actions response");
        }
        
        return *parsed;
    }, "getCorporateActions");
}

Result<std::vector<FinancialRatios>> MarketService::getFinancialRatios(const std::string& symbol) {
    return tryExecute<std::vector<FinancialRatios>>([&]() -> std::vector<FinancialRatios> {
        if (symbol.empty()) {
            throw ValidationError("Symbol cannot be empty");
        }
        
        QueryParams params;
        params["symbols"] = symbol;
        
        auto response = client_.get("/beta/markets/fundamentals/ratios", params);
        
        if (!response.success()) {
            throw ::tradier::ApiError(response.status, "Failed to get financial ratios: " + response.body);
        }
        
        auto parsed = json::parseResponse<std::vector<FinancialRatios>>(response, json::parseFinancialRatios);
        if (!parsed) {
            throw std::runtime_error("Failed to parse financial ratios response");
        }
        
        return *parsed;
    }, "getFinancialRatios");
}

Result<FinancialStatement> MarketService::getFinancialStatements(const std::string& symbol) {
    return tryExecute<FinancialStatement>([&]() -> FinancialStatement {
        if (symbol.empty()) {
            throw ValidationError("Symbol cannot be empty");
        }
        
        QueryParams params;
        params["symbols"] = symbol;
        
        auto response = client_.get("/beta/markets/fundamentals/financials", params);
        
        if (!response.success()) {
            throw ::tradier::ApiError(response.status, "Failed to get financial statements: " + response.body);
        }
        
        auto parsed = json::parseResponse<FinancialStatement>(response, json::parseFinancialStatements);
        if (!parsed) {
            throw std::runtime_error("Failed to parse financial statements response");
        }
        
        return *parsed;
    }, "getFinancialStatements");
}

Result<PriceStatistics> MarketService::getPriceStatistics(const std::string& symbol) {
    return tryExecute<PriceStatistics>([&]() -> PriceStatistics {
        if (symbol.empty()) {
            throw ValidationError("Symbol cannot be empty");
        }
        
        QueryParams params;
        params["symbols"] = symbol;
        
        auto response = client_.get("/beta/markets/fundamentals/statistics", params);
        
        if (!response.success()) {
            throw ::tradier::ApiError(response.status, "Failed to get price statistics: " + response.body);
        }
        
        auto parsed = json::parseResponse<PriceStatistics>(response, json::parsePriceStatistics);
        if (!parsed) {
            throw std::runtime_error("Failed to parse price statistics response");
        }
        
        return *parsed;
    }, "getPriceStatistics");
}

// Async method implementations
SimpleAsyncResult<std::vector<Quote>> MarketService::getQuotesAsync(const std::vector<std::string>& symbols, bool greeks) {
    return makeSimpleAsync<std::vector<Quote>>([this, symbols, greeks]() {
        return getQuotes(symbols, greeks);
    });
}

SimpleAsyncResult<std::vector<Quote>> MarketService::getQuotesPostAsync(const std::vector<std::string>& symbols, bool greeks) {
    return makeSimpleAsync<std::vector<Quote>>([this, symbols, greeks]() {
        return getQuotesPost(symbols, greeks);
    });
}

SimpleAsyncResult<Quote> MarketService::getQuoteAsync(const std::string& symbol, bool greeks) {
    return makeSimpleAsync<Quote>([this, symbol, greeks]() {
        return getQuote(symbol, greeks);
    });
}

SimpleAsyncResult<std::vector<OptionChain>> MarketService::getOptionChainAsync(const std::string& symbol, const std::string& expiration, bool greeks) {
    return makeSimpleAsync<std::vector<OptionChain>>([this, symbol, expiration, greeks]() {
        return getOptionChain(symbol, expiration, greeks);
    });
}

SimpleAsyncResult<std::vector<double>> MarketService::getOptionStrikesAsync(const std::string& symbol, const std::string& expiration, bool includeAllRoots) {
    return makeSimpleAsync<std::vector<double>>([this, symbol, expiration, includeAllRoots]() {
        return getOptionStrikes(symbol, expiration, includeAllRoots);
    });
}

SimpleAsyncResult<std::vector<Expiration>> MarketService::getOptionExpirationsAsync(const std::string& symbol, bool includeAllRoots, bool strikes, bool contractSize, bool expirationType) {
    return makeSimpleAsync<std::vector<Expiration>>([this, symbol, includeAllRoots, strikes, contractSize, expirationType]() {
        return getOptionExpirations(symbol, includeAllRoots, strikes, contractSize, expirationType);
    });
}

SimpleAsyncResult<std::vector<OptionSymbol>> MarketService::lookupOptionSymbolsAsync(const std::string& underlying) {
    return makeSimpleAsync<std::vector<OptionSymbol>>([this, underlying]() {
        return lookupOptionSymbols(underlying);
    });
}

AsyncResult<std::vector<HistoricalData>> MarketService::getHistoricalDataAsync(const std::string& symbol, const std::string& interval, const std::string& start, const std::string& end, const std::string& sessionFilter) {
    return makeAsync<std::vector<HistoricalData>>([this, symbol, interval, start, end, sessionFilter]() {
        return getHistoricalData(symbol, interval, start, end, sessionFilter);
    });
}

AsyncResult<std::vector<TimeSalesData>> MarketService::getTimeSalesAsync(const std::string& symbol, const std::string& interval, const std::string& start, const std::string& end, const std::string& sessionFilter) {
    return makeAsync<std::vector<TimeSalesData>>([this, symbol, interval, start, end, sessionFilter]() {
        return getTimeSales(symbol, interval, start, end, sessionFilter);
    });
}

AsyncResult<std::vector<Security>> MarketService::getETBListAsync() {
    return makeAsync<std::vector<Security>>([this]() {
        return getETBList();
    });
}

AsyncResult<MarketClock> MarketService::getClockAsync(bool delayed) {
    return makeAsync<MarketClock>([this, delayed]() {
        return getClock(delayed);
    });
}

AsyncResult<MarketCalendar> MarketService::getCalendarAsync(const std::string& month, const std::string& year) {
    return makeAsync<MarketCalendar>([this, month, year]() {
        return getCalendar(month, year);
    });
}

AsyncResult<std::vector<Security>> MarketService::searchSymbolsAsync(const std::string& query, bool indexes) {
    return makeAsync<std::vector<Security>>([this, query, indexes]() {
        return searchSymbols(query, indexes);
    });
}

AsyncResult<std::vector<Security>> MarketService::lookupSymbolsAsync(const std::string& query, const std::string& exchanges, const std::string& types) {
    return makeAsync<std::vector<Security>>([this, query, exchanges, types]() {
        return lookupSymbols(query, exchanges, types);
    });
}

AsyncResult<CompanyFundamentals> MarketService::getCompanyInfoAsync(const std::string& symbol) {
    return makeAsync<CompanyFundamentals>([this, symbol]() {
        return getCompanyInfo(symbol);
    });
}

AsyncResult<std::vector<CorporateCalendarEvent>> MarketService::getCorporateCalendarAsync(const std::string& symbol) {
    return makeAsync<std::vector<CorporateCalendarEvent>>([this, symbol]() {
        return getCorporateCalendar(symbol);
    });
}

AsyncResult<std::vector<Dividend>> MarketService::getDividendsAsync(const std::string& symbol) {
    return makeAsync<std::vector<Dividend>>([this, symbol]() {
        return getDividends(symbol);
    });
}

AsyncResult<CorporateActions> MarketService::getCorporateActionsAsync(const std::string& symbol) {
    return makeAsync<CorporateActions>([this, symbol]() {
        return getCorporateActions(symbol);
    });
}

AsyncResult<std::vector<FinancialRatios>> MarketService::getFinancialRatiosAsync(const std::string& symbol) {
    return makeAsync<std::vector<FinancialRatios>>([this, symbol]() {
        return getFinancialRatios(symbol);
    });
}

AsyncResult<FinancialStatement> MarketService::getFinancialStatementsAsync(const std::string& symbol) {
    return makeAsync<FinancialStatement>([this, symbol]() {
        return getFinancialStatements(symbol);
    });
}

AsyncResult<PriceStatistics> MarketService::getPriceStatisticsAsync(const std::string& symbol) {
    return makeAsync<PriceStatistics>([this, symbol]() {
        return getPriceStatistics(symbol);
    });
}

// Callback-based async methods
void MarketService::getQuotesAsync(const std::vector<std::string>& symbols, SimpleAsyncCallback<std::vector<Quote>> callback, bool greeks) {
    executeSimpleAsync<std::vector<Quote>>([this, symbols, greeks]() {
        return getQuotes(symbols, greeks);
    }, std::move(callback));
}

void MarketService::getQuoteAsync(const std::string& symbol, SimpleAsyncCallback<Quote> callback, bool greeks) {
    executeSimpleAsync<Quote>([this, symbol, greeks]() {
        return getQuote(symbol, greeks);
    }, std::move(callback));
}

}