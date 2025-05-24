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
#include "tradier/common/api_result.hpp"

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



struct CompanyProfile {
    std::string companyId;
    std::string contactEmail;
    std::string addressLine1;
    std::string city;
    std::string country;
    std::string fax;
    std::string homepage;
    std::string phone;
    std::string postalCode;
    std::string province;
    int totalEmployeeNumber = 0;
    std::string totalEmployeeNumberAsOfDate;
    std::string longDescription;
};

struct AssetClassification {
    std::string companyId;
    std::string financialHealthGrade;
    std::string financialHealthGradeAsOfDate;
    std::string growthGrade;
    std::string growthGradeAsOfDate;
    double growthScore = 0.0;
    int morningstarEconomySphereCode = 0;
    int morningstarIndustryCode = 0;
    int morningstarIndustryGroupCode = 0;
    int morningstarSectorCode = 0;
    int naics = 0;
    std::string profitabilityGrade;
    std::string profitabilityGradeAsOfDate;
    int sic = 0;
    double sizeScore = 0.0;
    int stockType = 0;
    std::string stockTypeAsOfDate;
    int styleBox = 0;
    std::string styleBoxAsOfDate;
    double styleScore = 0.0;
    double valueScore = 0.0;
};

struct CompanyFundamentals {
    std::string companyId;
    CompanyProfile profile;
    AssetClassification classification;
    std::string longDescription;
};

struct CorporateCalendarEvent {
    std::string companyId;
    std::string beginDateTime;
    std::string endDateTime;
    int eventType = 0;
    std::string estimatedDateForNextEvent;
    std::string event;
    int eventFiscalYear = 0;
    std::string eventStatus;
    std::string timeZone;
};

struct Dividend {
    std::string shareClassId;
    std::string dividendType;
    std::string exDate;
    double cashAmount = 0.0;
    std::string currencyId;
    std::string declarationDate;
    int frequency = 0;
    std::string payDate;
    std::string recordDate;
};

struct StockSplit {
    std::string shareClassId;
    std::string exDate;
    double adjustmentFactor = 0.0;
    double splitFrom = 0.0;
    double splitTo = 0.0;
    std::string splitType;
};

struct MergerAcquisition {
    std::string acquiredCompanyId;
    std::string parentCompanyId;
    double cashAmount = 0.0;
    std::string currencyId;
    std::string effectiveDate;
    std::string notes;
};

struct CorporateActions {
    std::vector<StockSplit> stockSplits;
    std::optional<MergerAcquisition> merger;
};

struct FinancialRatios {
    std::string companyId;
    std::string asOfDate;
    std::string fiscalYearEnd;
    std::string period;
    std::string reportType;
    
    double assetsTurnover = 0.0;
    double capExSalesRatio = 0.0;
    double cashConversionCycle = 0.0;
    double daysInInventory = 0.0;
    double daysInPayment = 0.0;
    double daysInSales = 0.0;
    double ebitdaMargin = 0.0;
    double ebitMargin = 0.0;
    double grossMargin = 0.0;
    double interestCoverage = 0.0;
    double netMargin = 0.0;
    double operationMargin = 0.0;
    double pretaxMargin = 0.0;
    double roa = 0.0;
    double roe = 0.0;
    double roic = 0.0;
    double taxRate = 0.0;
};

struct ValuationRatios {
    std::string shareClassId;
    std::string asOfDate;
    
    double peRatio = 0.0;
    double forwardPeRatio = 0.0;
    double pbRatio = 0.0;
    double psRatio = 0.0;
    double pegRatio = 0.0;
    double priceToEbitda = 0.0;
    double dividendYield = 0.0;
    double dividendRate = 0.0;
    double bookValuePerShare = 0.0;
    double earningYield = 0.0;
    double fcfYield = 0.0;
    double salesPerShare = 0.0;
};

struct FinancialStatement {
    std::string companyId;
    std::string asOfDate;
    std::string currencyId;
    std::string fiscalYearEnd;
    std::string period;
    std::string reportType;
    
    double totalRevenue = 0.0;
    double operatingRevenue = 0.0;
    double grossProfit = 0.0;
    double operatingIncome = 0.0;
    double netIncome = 0.0;
    double ebit = 0.0;
    double ebitda = 0.0;
    
    double totalAssets = 0.0;
    double currentAssets = 0.0;
    double totalLiabilities = 0.0;
    double currentLiabilities = 0.0;
    double stockholdersEquity = 0.0;
    
    double operatingCashFlow = 0.0;
    double freeCashFlow = 0.0;
    double capitalExpenditure = 0.0;
};

struct PriceStatistics {
    std::string shareClassId;
    std::string asOfDate;
    std::string period;
    
    double highPrice = 0.0;
    double lowPrice = 0.0;
    double averageVolume = 0.0;
    double totalVolume = 0.0;
    double movingAveragePrice = 0.0;
    double closePriceToMovingAverage = 0.0;
    double percentageBelowHighPrice = 0.0;
    double arithmeticMean = 0.0;
    double standardDeviation = 0.0;
    double best3MonthTotalReturn = 0.0;
    double worst3MonthTotalReturn = 0.0;
};

class MarketService {
private:
    TradierClient& client_;
    
public:
    explicit MarketService(TradierClient& client) : client_(client) {}

    Result<std::vector<Quote>> getQuotes(const std::vector<std::string>& symbols, bool greeks = false);
    Result<std::vector<Quote>> getQuotesPost(const std::vector<std::string>& symbols, bool greeks = false);
    Result<Quote> getQuote(const std::string& symbol, bool greeks = false);

    Result<std::vector<OptionChain>> getOptionChain(const std::string& symbol, const std::string& expiration, bool greeks = false);
    Result<std::vector<double>> getOptionStrikes(const std::string& symbol, const std::string& expiration, bool includeAllRoots = false);
    Result<std::vector<Expiration>> getOptionExpirations(const std::string& symbol, bool includeAllRoots = false, bool strikes = false, bool contractSize = false, bool expirationType = false);
    Result<std::vector<OptionSymbol>> lookupOptionSymbols(const std::string& underlying);

    Result<std::vector<HistoricalData>> getHistoricalData(const std::string& symbol, const std::string& interval = "daily", const std::string& start = "", const std::string& end = "", const std::string& sessionFilter = "all");
    Result<std::vector<TimeSalesData>> getTimeSales(const std::string& symbol, const std::string& interval = "1min", const std::string& start = "", const std::string& end = "", const std::string& sessionFilter = "all");

    Result<std::vector<Security>> getETBList();
    Result<MarketClock> getClock(bool delayed = false);
    Result<MarketCalendar> getCalendar(const std::string& month = "", const std::string& year = "");
    Result<std::vector<Security>> searchSymbols(const std::string& query, bool indexes = true);
    Result<std::vector<Security>> lookupSymbols(const std::string& query, const std::string& exchanges = "", const std::string& types = "");

    Result<CompanyFundamentals> getCompanyInfo(const std::string& symbol);
    Result<std::vector<CorporateCalendarEvent>> getCorporateCalendar(const std::string& symbol);
    Result<std::vector<Dividend>> getDividends(const std::string& symbol);
    Result<CorporateActions> getCorporateActions(const std::string& symbol);
    Result<std::vector<FinancialRatios>> getFinancialRatios(const std::string& symbol);
    Result<FinancialStatement> getFinancialStatements(const std::string& symbol);
    Result<PriceStatistics> getPriceStatistics(const std::string& symbol);
};

}