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

#include "tradier/json/market.hpp"
#include "tradier/common/utils.hpp"
#include <chrono>

namespace tradier {
namespace json {

Greeks parseGreeks(const nlohmann::json& json) {
    Greeks greeks;
    if (!json.is_null() && json.is_object()) {
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
        
        if (json.contains("updated_at") && !json["updated_at"].is_null() && json["updated_at"].is_string()) {
            greeks.updatedAt = utils::parseISODateTime(json["updated_at"]);
        }
    }
    return greeks;
}

Quote parseQuote(const nlohmann::json& json) {
    Quote quote;
    if (!json.is_null() && json.is_object()) {
        quote.symbol = json.value("symbol", "");
        quote.description = json.value("description", "");
        quote.exchange = json.value("exch", "");
        quote.type = json.value("type", "");
        
        if (json.contains("last") && !json["last"].is_null() && json["last"].is_number()) {
            quote.last = json["last"];
        }
        if (json.contains("change") && !json["change"].is_null() && json["change"].is_number()) {
            quote.change = json["change"];
        }
        
        quote.volume = json.value("volume", 0);
        
        if (json.contains("open") && !json["open"].is_null() && json["open"].is_number()) {
            quote.open = json["open"];
        }
        if (json.contains("high") && !json["high"].is_null() && json["high"].is_number()) {
            quote.high = json["high"];
        }
        if (json.contains("low") && !json["low"].is_null() && json["low"].is_number()) {
            quote.low = json["low"];
        }
        if (json.contains("close") && !json["close"].is_null() && json["close"].is_number()) {
            quote.close = json["close"];
        }
        
        quote.bid = json.value("bid", 0.0);
        quote.ask = json.value("ask", 0.0);
        
        if (json.contains("change_percentage") && !json["change_percentage"].is_null() && json["change_percentage"].is_number()) {
            quote.changePercentage = json["change_percentage"];
        }
        
        quote.averageVolume = json.value("average_volume", 0);
        quote.lastVolume = json.value("last_volume", 0);
        
        if (json.contains("trade_date") && !json["trade_date"].is_null() && json["trade_date"].is_number() && json["trade_date"].get<long>() > 0) {
            quote.tradeDate = std::chrono::system_clock::from_time_t(json["trade_date"].get<long>() / 1000);
        }
        
        if (json.contains("prevclose") && !json["prevclose"].is_null() && json["prevclose"].is_number()) {
            quote.prevClose = json["prevclose"];
        }
        
        quote.week52High = json.value("week_52_high", 0.0);
        quote.week52Low = json.value("week_52_low", 0.0);
        quote.bidSize = json.value("bidsize", 0);
        quote.bidExchange = json.value("bidexch", "");
        
        if (json.contains("bid_date") && !json["bid_date"].is_null() && json["bid_date"].is_number() && json["bid_date"].get<long>() > 0) {
            quote.bidDate = std::chrono::system_clock::from_time_t(json["bid_date"].get<long>() / 1000);
        }
        
        quote.askSize = json.value("asksize", 0);
        quote.askExchange = json.value("askexch", "");
        
        if (json.contains("ask_date") && !json["ask_date"].is_null() && json["ask_date"].is_number() && json["ask_date"].get<long>() > 0) {
            quote.askDate = std::chrono::system_clock::from_time_t(json["ask_date"].get<long>() / 1000);
        }
        
        quote.rootSymbols = json.value("root_symbols", "");
        
        if (json.contains("underlying") && !json["underlying"].is_null()) {
            quote.underlying = json.value("underlying", "");
        }
        if (json.contains("strike") && !json["strike"].is_null()) {
            quote.strike = json.value("strike", 0.0);
        }
        if (json.contains("open_interest") && !json["open_interest"].is_null()) {
            quote.openInterest = json.value("open_interest", 0);
        }
        if (json.contains("contract_size") && !json["contract_size"].is_null()) {
            quote.contractSize = json.value("contract_size", 100);
        }
        if (json.contains("expiration_date") && !json["expiration_date"].is_null()) {
            quote.expirationDate = json.value("expiration_date", "");
        }
        if (json.contains("expiration_type") && !json["expiration_type"].is_null()) {
            quote.expirationType = json.value("expiration_type", "");
        }
        if (json.contains("option_type") && !json["option_type"].is_null()) {
            quote.optionType = json.value("option_type", "");
        }
        if (json.contains("root_symbol") && !json["root_symbol"].is_null()) {
            quote.rootSymbol = json.value("root_symbol", "");
        }
        if (json.contains("greeks") && !json["greeks"].is_null()) {
            quote.greeks = parseGreeks(json["greeks"]);
        }
    }
    return quote;
}

std::vector<Quote> parseQuotes(const nlohmann::json& json) {
    std::vector<Quote> quotes;
    if (!json.is_null() && json.is_object() && json.contains("quotes") && !json["quotes"].is_null() && json["quotes"].contains("quote") && !json["quotes"]["quote"].is_null()) {
        const auto& quotesJson = json["quotes"]["quote"];
        
        if (quotesJson.is_array()) {
            for (const auto& item : quotesJson) {
                if (!item.is_null()) {
                    quotes.push_back(parseQuote(item));
                }
            }
        } else if (quotesJson.is_object()) {
            quotes.push_back(parseQuote(quotesJson));
        }
    }
    return quotes;
}

OptionChain parseOptionChain(const nlohmann::json& json) {
    OptionChain option;
    if (!json.is_null() && json.is_object()) {
        option.symbol = json.value("symbol", "");
        option.description = json.value("description", "");
        option.exchange = json.value("exch", "");
        option.type = json.value("type", "");
        
        if (json.contains("last") && !json["last"].is_null() && json["last"].is_number()) {
            option.last = json["last"];
        }
        if (json.contains("change") && !json["change"].is_null() && json["change"].is_number()) {
            option.change = json["change"];
        }
        
        option.volume = json.value("volume", 0);
        
        if (json.contains("open") && !json["open"].is_null() && json["open"].is_number()) {
            option.open = json["open"];
        }
        if (json.contains("high") && !json["high"].is_null() && json["high"].is_number()) {
            option.high = json["high"];
        }
        if (json.contains("low") && !json["low"].is_null() && json["low"].is_number()) {
            option.low = json["low"];
        }
        if (json.contains("close") && !json["close"].is_null() && json["close"].is_number()) {
            option.close = json["close"];
        }
        
        option.bid = json.value("bid", 0.0);
        option.ask = json.value("ask", 0.0);
        option.underlying = json.value("underlying", "");
        option.strike = json.value("strike", 0.0);
        
        if (json.contains("change_percentage") && !json["change_percentage"].is_null() && json["change_percentage"].is_number()) {
            option.changePercentage = json["change_percentage"];
        }
        
        option.averageVolume = json.value("average_volume", 0);
        option.lastVolume = json.value("last_volume", 0);
        
        if (json.contains("trade_date") && !json["trade_date"].is_null() && json["trade_date"].is_number() && json["trade_date"].get<long>() > 0) {
            option.tradeDate = std::chrono::system_clock::from_time_t(json["trade_date"].get<long>() / 1000);
        }
        
        if (json.contains("prevclose") && !json["prevclose"].is_null() && json["prevclose"].is_number()) {
            option.prevClose = json["prevclose"];
        }
        
        option.week52High = json.value("week_52_high", 0.0);
        option.week52Low = json.value("week_52_low", 0.0);
        option.bidSize = json.value("bidsize", 0);
        option.bidExchange = json.value("bidexch", "");
        
        if (json.contains("bid_date") && !json["bid_date"].is_null() && json["bid_date"].is_number() && json["bid_date"].get<long>() > 0) {
            option.bidDate = std::chrono::system_clock::from_time_t(json["bid_date"].get<long>() / 1000);
        }
        
        option.askSize = json.value("asksize", 0);
        option.askExchange = json.value("askexch", "");
        
        if (json.contains("ask_date") && !json["ask_date"].is_null() && json["ask_date"].is_number() && json["ask_date"].get<long>() > 0) {
            option.askDate = std::chrono::system_clock::from_time_t(json["ask_date"].get<long>() / 1000);
        }
        
        option.openInterest = json.value("open_interest", 0);
        option.contractSize = json.value("contract_size", 100);
        option.expirationDate = json.value("expiration_date", "");
        option.expirationType = json.value("expiration_type", "");
        option.optionType = json.value("option_type", "");
        option.rootSymbol = json.value("root_symbol", "");
        
        if (json.contains("greeks") && !json["greeks"].is_null()) {
            option.greeks = parseGreeks(json["greeks"]);
        }
    }
    return option;
}

std::vector<OptionChain> parseOptionChains(const nlohmann::json& json) {
    std::vector<OptionChain> options;
    if (!json.is_null() && json.is_object() && json.contains("options") && !json["options"].is_null() && json["options"].contains("option") && !json["options"]["option"].is_null()) {
        const auto& optionsJson = json["options"]["option"];
        
        if (optionsJson.is_array()) {
            for (const auto& item : optionsJson) {
                if (!item.is_null()) {
                    options.push_back(parseOptionChain(item));
                }
            }
        } else if (optionsJson.is_object()) {
            options.push_back(parseOptionChain(optionsJson));
        }
    }
    return options;
}

std::vector<double> parseStrikes(const nlohmann::json& json) {
    std::vector<double> strikes;
    if (!json.is_null() && json.is_object() && json.contains("strikes") && !json["strikes"].is_null() && json["strikes"].contains("strike") && !json["strikes"]["strike"].is_null()) {
        const auto& strikesJson = json["strikes"]["strike"];
        
        if (strikesJson.is_array()) {
            for (const auto& item : strikesJson) {
                if (!item.is_null() && item.is_number()) {
                    strikes.push_back(item.get<double>());
                }
            }
        } else if (strikesJson.is_number()) {
            strikes.push_back(strikesJson.get<double>());
        }
    }
    return strikes;
}

Expiration parseExpiration(const nlohmann::json& json) {
    Expiration expiration;
    if (!json.is_null() && json.is_object()) {
        expiration.date = json.value("date", "");
        expiration.contractSize = json.value("contract_size", 100);
        expiration.expirationType = json.value("expiration_type", "");
        
        if (json.contains("strikes") && !json["strikes"].is_null() && json["strikes"].contains("strike") && !json["strikes"]["strike"].is_null()) {
            const auto& strikesJson = json["strikes"]["strike"];
            
            if (strikesJson.is_array()) {
                for (const auto& item : strikesJson) {
                    if (!item.is_null() && item.is_number()) {
                        expiration.strikes.push_back(item.get<double>());
                    }
                }
            } else if (strikesJson.is_number()) {
                expiration.strikes.push_back(strikesJson.get<double>());
            }
        }
    }
    return expiration;
}

std::vector<Expiration> parseExpirations(const nlohmann::json& json) {
    std::vector<Expiration> expirations;
    if (!json.is_null() && json.is_object() && json.contains("expirations") && !json["expirations"].is_null() && json["expirations"].contains("expiration") && !json["expirations"]["expiration"].is_null()) {
        const auto& expirationsJson = json["expirations"]["expiration"];
        
        if (expirationsJson.is_array()) {
            for (const auto& item : expirationsJson) {
                if (!item.is_null()) {
                    expirations.push_back(parseExpiration(item));
                }
            }
        } else if (expirationsJson.is_object()) {
            expirations.push_back(parseExpiration(expirationsJson));
        }
    }
    return expirations;
}

OptionSymbol parseOptionSymbol(const nlohmann::json& json) {
    OptionSymbol optionSymbol;
    if (!json.is_null() && json.is_object()) {
        optionSymbol.rootSymbol = json.value("rootSymbol", "");
        
        if (json.contains("options") && !json["options"].is_null() && json["options"].is_array()) {
            for (const auto& item : json["options"]) {
                if (!item.is_null() && item.is_string()) {
                    optionSymbol.options.push_back(item.get<std::string>());
                }
            }
        }
    }
    return optionSymbol;
}

std::vector<OptionSymbol> parseOptionSymbols(const nlohmann::json& json) {
    std::vector<OptionSymbol> optionSymbols;
    if (!json.is_null() && json.is_object() && json.contains("symbols") && !json["symbols"].is_null() && json["symbols"].is_array()) {
        for (const auto& item : json["symbols"]) {
            if (!item.is_null()) {
                optionSymbols.push_back(parseOptionSymbol(item));
            }
        }
    }
    return optionSymbols;
}

HistoricalData parseHistoricalData(const nlohmann::json& json) {
    HistoricalData data;
    if (!json.is_null() && json.is_object()) {
        data.date = json.value("date", "");
        data.open = json.value("open", 0.0);
        data.high = json.value("high", 0.0);
        data.low = json.value("low", 0.0);
        data.close = json.value("close", 0.0);
        data.volume = json.value("volume", 0L);
    }
    return data;
}

std::vector<HistoricalData> parseHistoricalDataList(const nlohmann::json& json) {
    std::vector<HistoricalData> history;
    if (!json.is_null() && json.is_object() && json.contains("history") && !json["history"].is_null() && json["history"].contains("day") && !json["history"]["day"].is_null()) {
        const auto& daysJson = json["history"]["day"];
        
        if (daysJson.is_array()) {
            for (const auto& item : daysJson) {
                if (!item.is_null()) {
                    history.push_back(parseHistoricalData(item));
                }
            }
        } else if (daysJson.is_object()) {
            history.push_back(parseHistoricalData(daysJson));
        }
    }
    return history;
}

TimeSalesData parseTimeSalesData(const nlohmann::json& json) {
    TimeSalesData data;
    if (!json.is_null() && json.is_object()) {
        data.time = json.value("time", "");
        data.timestamp = json.value("timestamp", 0L);
        data.price = json.value("price", 0.0);
        data.open = json.value("open", 0.0);
        data.high = json.value("high", 0.0);
        data.low = json.value("low", 0.0);
        data.close = json.value("close", 0.0);
        data.volume = json.value("volume", 0L);
        data.vwap = json.value("vwap", 0.0);
    }
    return data;
}

std::vector<TimeSalesData> parseTimeSalesList(const nlohmann::json& json) {
    std::vector<TimeSalesData> timeSales;
    if (!json.is_null() && json.is_object() && json.contains("series") && !json["series"].is_null() && json["series"].contains("data") && !json["series"]["data"].is_null()) {
        const auto& dataJson = json["series"]["data"];
        
        if (dataJson.is_array()) {
            for (const auto& item : dataJson) {
                if (!item.is_null()) {
                    timeSales.push_back(parseTimeSalesData(item));
                }
            }
        } else if (dataJson.is_object()) {
            timeSales.push_back(parseTimeSalesData(dataJson));
        }
    }
    return timeSales;
}

Security parseSecurity(const nlohmann::json& json) {
    Security security;
    if (!json.is_null() && json.is_object()) {
        security.symbol = json.contains("symbol") && !json["symbol"].is_null() && json["symbol"].is_string() ? json["symbol"].get<std::string>() : "";
        security.exchange = json.contains("exchange") && !json["exchange"].is_null() && json["exchange"].is_string() ? json["exchange"].get<std::string>() : "";
        security.type = json.contains("type") && !json["type"].is_null() && json["type"].is_string() ? json["type"].get<std::string>() : "";
        security.description = json.contains("description") && !json["description"].is_null() && json["description"].is_string() ? json["description"].get<std::string>() : "";
    }
    return security;
}

std::vector<Security> parseSecurities(const nlohmann::json& json) {
    std::vector<Security> securities;
    if (!json.is_null() && json.is_object() && json.contains("securities") && !json["securities"].is_null() && json["securities"].contains("security") && !json["securities"]["security"].is_null()) {
        const auto& securitiesJson = json["securities"]["security"];
        
        if (securitiesJson.is_array()) {
            for (const auto& item : securitiesJson) {
                if (!item.is_null()) {
                    securities.push_back(parseSecurity(item));
                }
            }
        } else if (securitiesJson.is_object()) {
            securities.push_back(parseSecurity(securitiesJson));
        }
    }
    return securities;
}

SessionTime parseSessionTime(const nlohmann::json& json) {
    SessionTime session;
    if (!json.is_null() && json.is_object()) {
        session.start = json.value("start", "");
        session.end = json.value("end", "");
    }
    return session;
}

MarketDay parseMarketDay(const nlohmann::json& json) {
    MarketDay day;
    if (!json.is_null() && json.is_object()) {
        day.date = json.value("date", "");
        day.status = json.value("status", "");
        day.description = json.value("description", "");
        
        if (json.contains("premarket") && !json["premarket"].is_null()) {
            day.premarket = parseSessionTime(json["premarket"]);
        }
        if (json.contains("open") && !json["open"].is_null()) {
            day.open = parseSessionTime(json["open"]);
        }
        if (json.contains("postmarket") && !json["postmarket"].is_null()) {
            day.postmarket = parseSessionTime(json["postmarket"]);
        }
    }
    return day;
}

MarketCalendar parseMarketCalendar(const nlohmann::json& json) {
    MarketCalendar calendar;
    if (!json.is_null() && json.is_object() && json.contains("calendar") && !json["calendar"].is_null()) {
        const auto& cal = json["calendar"];
        calendar.month = cal.value("month", 0);
        calendar.year = cal.value("year", 0);
        
        if (cal.contains("days") && !cal["days"].is_null() && cal["days"].contains("day") && !cal["days"]["day"].is_null()) {
            const auto& daysJson = cal["days"]["day"];
            
            if (daysJson.is_array()) {
                for (const auto& item : daysJson) {
                    if (!item.is_null()) {
                        calendar.days.push_back(parseMarketDay(item));
                    }
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
    if (!json.is_null() && json.is_object() && json.contains("clock") && !json["clock"].is_null()) {
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

CompanyFundamentals parseCompanyFundamentals(const nlohmann::json& json) {
    CompanyFundamentals fundamentals;
    if (!json.is_null() && json.is_array() && !json.empty()) {
        const auto& firstResult = json[0];
        if (firstResult.contains("results") && !firstResult["results"].is_null() && firstResult["results"].is_array() && !firstResult["results"].empty()) {
            const auto& result = firstResult["results"][0];
            if (result.contains("tables") && !result["tables"].is_null()) {
                const auto& tables = result["tables"];
                
                if (tables.contains("company_profile") && !tables["company_profile"].is_null()) {
                    const auto& profile = tables["company_profile"];
                    fundamentals.profile.companyId = profile.value("company_id", "");
                    fundamentals.profile.contactEmail = profile.value("contact_email", "");
                    fundamentals.profile.totalEmployeeNumber = profile.value("total_employee_number", 0);
                    fundamentals.profile.totalEmployeeNumberAsOfDate = profile.value("TotalEmployeeNumber.asOfDate", "");
                    
                    if (profile.contains("headquarter") && !profile["headquarter"].is_null()) {
                        const auto& hq = profile["headquarter"];
                        fundamentals.profile.addressLine1 = hq.value("address_line1", "");
                        fundamentals.profile.city = hq.value("city", "");
                        fundamentals.profile.country = hq.value("country", "");
                        fundamentals.profile.phone = hq.value("phone", "");
                        fundamentals.profile.homepage = hq.value("homepage", "");
                        fundamentals.profile.postalCode = hq.value("postal_code", "");
                        fundamentals.profile.province = hq.value("province", "");
                    }
                }
                
                if (tables.contains("asset_classification") && !tables["asset_classification"].is_null()) {
                    const auto& classification = tables["asset_classification"];
                    fundamentals.classification.companyId = classification.value("company_id", "");
                    fundamentals.classification.financialHealthGrade = classification.value("financial_health_grade", "");
                    fundamentals.classification.growthGrade = classification.value("growth_grade", "");
                    fundamentals.classification.growthScore = classification.value("growth_score", 0.0);
                    fundamentals.classification.profitabilityGrade = classification.value("profitability_grade", "");
                    fundamentals.classification.sizeScore = classification.value("size_score", 0.0);
                    fundamentals.classification.valueScore = classification.value("value_score", 0.0);
                }
                
                if (tables.contains("long_descriptions") && !tables["long_descriptions"].is_null()) {
                    fundamentals.longDescription = tables["long_descriptions"];
                }
            }
        }
    }
    return fundamentals;
}

std::vector<CorporateCalendarEvent> parseCorporateCalendar(const nlohmann::json& json) {
    std::vector<CorporateCalendarEvent> events;
    if (!json.is_null() && json.is_array() && !json.empty()) {
        const auto& firstResult = json[0];
        if (firstResult.contains("results") && !firstResult["results"].is_null() && firstResult["results"].is_array() && !firstResult["results"].empty()) {
            const auto& result = firstResult["results"][0];
            if (result.contains("tables") && !result["tables"].is_null() && result["tables"].contains("corporate_calendars") && !result["tables"]["corporate_calendars"].is_null()) {
                const auto& calendars = result["tables"]["corporate_calendars"];
                
                if (calendars.is_array()) {
                    for (const auto& item : calendars) {
                        if (!item.is_null()) {
                            CorporateCalendarEvent event;
                            event.companyId = item.value("company_id", "");
                            event.beginDateTime = item.value("begin_date_time", "");
                            event.endDateTime = item.value("end_date_time", "");
                            event.eventType = item.value("event_type", 0);
                            event.event = item.value("event", "");
                            event.eventFiscalYear = item.value("event_fiscal_year", 0);
                            event.eventStatus = item.value("event_status", "");
                            events.push_back(event);
                        }
                    }
                }
            }
        }
    }
    return events;
}

std::vector<Dividend> parseDividends(const nlohmann::json& json) {
    std::vector<Dividend> dividends;
    if (!json.is_null() && json.is_array() && !json.empty()) {
        const auto& firstResult = json[0];
        if (firstResult.contains("results") && !firstResult["results"].is_null() && firstResult["results"].is_array() && !firstResult["results"].empty()) {
            const auto& result = firstResult["results"][0];
            if (result.contains("tables") && !result["tables"].is_null() && result["tables"].contains("cash_dividends") && !result["tables"]["cash_dividends"].is_null()) {
                const auto& cashDividends = result["tables"]["cash_dividends"];
                
                if (cashDividends.is_array()) {
                    for (const auto& item : cashDividends) {
                        if (!item.is_null()) {
                            Dividend dividend;
                            dividend.shareClassId = item.value("share_class_id", "");
                            dividend.dividendType = item.value("dividend_type", "");
                            dividend.exDate = item.value("ex_date", "");
                            dividend.cashAmount = item.value("cash_amount", 0.0);
                            dividend.currencyId = item.value("currency_i_d", "");
                            dividend.declarationDate = item.value("declaration_date", "");
                            dividend.frequency = item.value("frequency", 0);
                            dividend.payDate = item.value("pay_date", "");
                            dividend.recordDate = item.value("record_date", "");
                            dividends.push_back(dividend);
                        }
                    }
                }
            }
        }
    }
    return dividends;
}

CorporateActions parseCorporateActions(const nlohmann::json& json) {
    CorporateActions actions;
    if (!json.is_null() && json.is_array() && !json.empty()) {
        for (const auto& resultItem : json[0]["results"]) {
            if (!resultItem.is_null() && resultItem.contains("tables") && !resultItem["tables"].is_null()) {
                const auto& tables = resultItem["tables"];
                
                if (tables.contains("stock_splits") && !tables["stock_splits"].is_null()) {
                    const auto& splits = tables["stock_splits"];
                    for (const auto& [date, splitData] : splits.items()) {
                        if (!splitData.is_null()) {
                            StockSplit split;
                            split.shareClassId = splitData.value("share_class_id", "");
                            split.exDate = splitData.value("ex_date", "");
                            split.adjustmentFactor = splitData.value("adjustment_factor", 0.0);
                            split.splitFrom = splitData.value("split_from", 0.0);
                            split.splitTo = splitData.value("split_to", 0.0);
                            split.splitType = splitData.value("split_type", "");
                            actions.stockSplits.push_back(split);
                        }
                    }
                }
                
                if (tables.contains("mergers_and_acquisitions") && !tables["mergers_and_acquisitions"].is_null()) {
                    const auto& merger = tables["mergers_and_acquisitions"];
                    MergerAcquisition ma;
                    ma.acquiredCompanyId = merger.value("acquired_company_id", "");
                    ma.parentCompanyId = merger.value("parent_company_id", "");
                    ma.cashAmount = merger.value("cash_amount", 0.0);
                    ma.currencyId = merger.value("currency_id", "");
                    ma.effectiveDate = merger.value("effective_date", "");
                    ma.notes = merger.value("notes", "");
                    actions.merger = ma;
                }
            }
        }
    }
    return actions;
}

std::vector<FinancialRatios> parseFinancialRatios(const nlohmann::json& json) {
    std::vector<FinancialRatios> ratios;
    if (!json.is_null() && json.is_array() && !json.empty()) {
        const auto& firstResult = json[0];
        if (firstResult.contains("results") && !firstResult["results"].is_null() && firstResult["results"].is_array() && !firstResult["results"].empty()) {
            const auto& result = firstResult["results"][0];
            if (result.contains("tables") && !result["tables"].is_null() && result["tables"].contains("operation_ratios_restate") && !result["tables"]["operation_ratios_restate"].is_null()) {
                const auto& operationRatios = result["tables"]["operation_ratios_restate"];
                
                if (operationRatios.is_array()) {
                    for (const auto& periodGroup : operationRatios) {
                        if (!periodGroup.is_null()) {
                            for (const auto& [periodKey, periodData] : periodGroup.items()) {
                                if (!periodData.is_null()) {
                                    FinancialRatios ratio;
                                    ratio.companyId = periodData.value("company_id", "");
                                    ratio.asOfDate = periodData.value("as_of_date", "");
                                    ratio.fiscalYearEnd = periodData.value("fiscal_year_end", "");
                                    ratio.period = periodData.value("period", "");
                                    ratio.reportType = periodData.value("report_type", "");
                                    ratio.assetsTurnover = periodData.value("assets_turnover", 0.0);
                                    ratio.ebitdaMargin = periodData.value("e_b_i_t_d_a_margin", 0.0);
                                    ratio.ebitMargin = periodData.value("e_b_i_t_margin", 0.0);
                                    ratio.grossMargin = periodData.value("gross_margin", 0.0);
                                    ratio.netMargin = periodData.value("net_margin", 0.0);
                                    ratio.operationMargin = periodData.value("operation_margin", 0.0);
                                    ratio.roa = periodData.value("r_o_a", 0.0);
                                    ratio.roe = periodData.value("r_o_e", 0.0);
                                    ratio.roic = periodData.value("r_o_i_c", 0.0);
                                    ratios.push_back(ratio);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return ratios;
}

FinancialStatement parseFinancialStatements(const nlohmann::json& json) {
    FinancialStatement statement;
    if (!json.is_null() && json.is_array() && !json.empty()) {
        const auto& firstResult = json[0];
        if (firstResult.contains("results") && !firstResult["results"].is_null() && firstResult["results"].is_array() && !firstResult["results"].empty()) {
            const auto& result = firstResult["results"][0];
            if (result.contains("tables") && !result["tables"].is_null() && result["tables"].contains("financial_statements_restate") && !result["tables"]["financial_statements_restate"].is_null()) {
                const auto& statements = result["tables"]["financial_statements_restate"];
                
                if (statements.contains("income_statement") && !statements["income_statement"].is_null() && statements["income_statement"].is_array() && !statements["income_statement"].empty()) {
                    const auto& incomeStatement = statements["income_statement"][0];
                    for (const auto& [periodKey, periodData] : incomeStatement.items()) {
                        if (!periodData.is_null()) {
                            statement.companyId = statements.value("company_id", "");
                            statement.asOfDate = statements.value("as_of_date", "");
                            statement.currencyId = periodData.value("currency_id", "");
                            statement.fiscalYearEnd = periodData.value("fiscal_year_end", "");
                            statement.period = periodData.value("period", "");
                            statement.reportType = periodData.value("report_type", "");
                            statement.totalRevenue = periodData.value("total_revenue", 0.0);
                            statement.operatingRevenue = periodData.value("operating_revenue", 0.0);
                            statement.grossProfit = periodData.value("gross_profit", 0.0);
                            statement.operatingIncome = periodData.value("operating_income", 0.0);
                            statement.netIncome = periodData.value("net_income", 0.0);
                            statement.ebit = periodData.value("e_b_i_t", 0.0);
                            statement.ebitda = periodData.value("e_b_i_t_d_a", 0.0);
                            break;
                        }
                    }
                }
            }
        }
    }
    return statement;
}

PriceStatistics parsePriceStatistics(const nlohmann::json& json) {
    PriceStatistics stats;
    if (!json.is_null() && json.is_array() && !json.empty()) {
        const auto& firstResult = json[0];
        if (firstResult.contains("results") && !firstResult["results"].is_null() && firstResult["results"].is_array() && !firstResult["results"].empty()) {
            const auto& result = firstResult["results"][0];
            if (result.contains("tables") && !result["tables"].is_null() && result["tables"].contains("price_statistics") && !result["tables"]["price_statistics"].is_null()) {
                const auto& priceStats = result["tables"]["price_statistics"];
                
                if (priceStats.contains("period_1y") && !priceStats["period_1y"].is_null()) {
                    const auto& yearlyStats = priceStats["period_1y"];
                    stats.shareClassId = yearlyStats.value("share_class_id", "");
                    stats.asOfDate = yearlyStats.value("as_of_date", "");
                    stats.period = yearlyStats.value("period", "");
                    stats.highPrice = yearlyStats.value("high_price", 0.0);
                    stats.lowPrice = yearlyStats.value("low_price", 0.0);
                    stats.averageVolume = yearlyStats.value("average_volume", 0.0);
                    stats.totalVolume = yearlyStats.value("total_volume", 0.0);
                    stats.movingAveragePrice = yearlyStats.value("moving_average_price", 0.0);
                    stats.closePriceToMovingAverage = yearlyStats.value("close_price_to_moving_average", 0.0);
                    stats.percentageBelowHighPrice = yearlyStats.value("percentage_below_high_price", 0.0);
                    stats.arithmeticMean = yearlyStats.value("arithmetic_mean", 0.0);
                    stats.standardDeviation = yearlyStats.value("standard_deviation", 0.0);
                    stats.best3MonthTotalReturn = yearlyStats.value("best3_month_total_return", 0.0);
                    stats.worst3MonthTotalReturn = yearlyStats.value("worst3_month_total_return", 0.0);
                }
            }
        }
    }
    return stats;
}

} // namespace json
} // namespace tradier