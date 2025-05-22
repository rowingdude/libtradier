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

#include <string>
#include <vector>
#include <chrono>
#include <nlohmann/json.hpp>
#include "tradier/common/utils.hpp"

namespace tradier {

struct Account;
struct AccountProfile;
struct MarginBalances;
struct CashBalances;
struct PDTBalances;
struct AccountBalances;
struct Position;
struct OrderLeg;
struct Order;
struct HistoryEvent;
struct Trade;
struct Adjustment;
struct OptionEvent;
struct Journal;
struct ClosedPosition;
using Positions = std::vector<Position>;
using Orders = std::vector<Order>;
using History = std::vector<HistoryEvent>;
using GainLoss = std::vector<ClosedPosition>;

namespace json {

Account parseAccount(const nlohmann::json& json);
AccountProfile parseAccountProfile(const nlohmann::json& json);
MarginBalances parseMarginBalances(const nlohmann::json& json);
CashBalances parseCashBalances(const nlohmann::json& json);
PDTBalances parsePDTBalances(const nlohmann::json& json);
AccountBalances parseAccountBalances(const nlohmann::json& json);
Position parsePosition(const nlohmann::json& json);
Positions parsePositions(const nlohmann::json& json);
OrderLeg parseOrderLeg(const nlohmann::json& json);
Order parseOrder(const nlohmann::json& json);
Order parseSingleOrder(const nlohmann::json& json);
Orders parseOrders(const nlohmann::json& json);
Trade parseTrade(const nlohmann::json& json);
Adjustment parseAdjustment(const nlohmann::json& json);
OptionEvent parseOptionEvent(const nlohmann::json& json);
Journal parseJournal(const nlohmann::json& json);
HistoryEvent parseHistoryEvent(const nlohmann::json& json);
History parseHistory(const nlohmann::json& json);
ClosedPosition parseClosedPosition(const nlohmann::json& json);
GainLoss parseGainLoss(const nlohmann::json& json);

} // namespace json
} // namespace tradier