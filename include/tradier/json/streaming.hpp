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
#include "tradier/common/utils.hpp"

namespace tradier {

struct MarketQuote;
struct MarketTrade;
struct MarketSummary;
struct MarketTimesale;
struct MarketTradex;
struct AccountOrderEvent;

namespace json {

MarketQuote parseMarketQuote(const nlohmann::json& json);
MarketTrade parseMarketTrade(const nlohmann::json& json);
MarketSummary parseMarketSummary(const nlohmann::json& json);
MarketTimesale parseMarketTimesale(const nlohmann::json& json);
MarketTradex parseMarketTradex(const nlohmann::json& json);
AccountOrderEvent parseAccountOrderEvent(const nlohmann::json& json);

} // namespace json
} // namespace tradier