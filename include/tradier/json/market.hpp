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

#include <nlohmann/json.hpp>
#include "tradier/market.hpp"

namespace tradier {
namespace json {

Greeks parseGreeks(const nlohmann::json& json);
Quote parseQuote(const nlohmann::json& json);
std::vector<Quote> parseQuotes(const nlohmann::json& json);
OptionChain parseOptionChain(const nlohmann::json& json);
std::vector<OptionChain> parseOptionChains(const nlohmann::json& json);
std::vector<double> parseStrikes(const nlohmann::json& json);
Expiration parseExpiration(const nlohmann::json& json);
std::vector<Expiration> parseExpirations(const nlohmann::json& json);
OptionSymbol parseOptionSymbol(const nlohmann::json& json);
std::vector<OptionSymbol> parseOptionSymbols(const nlohmann::json& json);
HistoricalData parseHistoricalData(const nlohmann::json& json);
std::vector<HistoricalData> parseHistoricalDataList(const nlohmann::json& json);
TimeSalesData parseTimeSalesData(const nlohmann::json& json);
std::vector<TimeSalesData> parseTimeSalesList(const nlohmann::json& json);
Security parseSecurity(const nlohmann::json& json);
std::vector<Security> parseSecurities(const nlohmann::json& json);
SessionTime parseSessionTime(const nlohmann::json& json);
MarketDay parseMarketDay(const nlohmann::json& json);
MarketCalendar parseMarketCalendar(const nlohmann::json& json);
MarketClock parseMarketClock(const nlohmann::json& json);

CompanyFundamentals parseCompanyFundamentals(const nlohmann::json& json);
std::vector<CorporateCalendarEvent> parseCorporateCalendar(const nlohmann::json& json);
std::vector<Dividend> parseDividends(const nlohmann::json& json);
CorporateActions parseCorporateActions(const nlohmann::json& json);
std::vector<FinancialRatios> parseFinancialRatios(const nlohmann::json& json);
FinancialStatement parseFinancialStatements(const nlohmann::json& json);
PriceStatistics parsePriceStatistics(const nlohmann::json& json);

} // namespace json
} // namespace tradier