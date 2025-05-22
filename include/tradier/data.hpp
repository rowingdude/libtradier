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
#include <optional>
#include "tradier/common/types.hpp"

namespace tradier {

struct Account {
    std::string number;
    std::string type;
    std::string status;
    std::string classification;
    bool dayTrader = false;
    int optionLevel = 0;
    TimePoint dateCreated;
    TimePoint lastUpdate;
};

struct AccountProfile {
    std::string id;
    std::string name;
    std::vector<Account> accounts;
};

struct Position {
    std::string symbol;
    double quantity = 0.0;
    double costBasis = 0.0;
    TimePoint acquired;
};

struct Order {
    int id = 0;
    std::string symbol;
    std::string type;
    std::string side;
    std::string status;
    double quantity = 0.0;
    double price = 0.0;
    double filled = 0.0;
    TimePoint created;
    std::optional<std::string> tag;
};

struct Quote {
    std::string symbol;
    double bid = 0.0;
    double ask = 0.0;
    double last = 0.0;
    int volume = 0;
    TimePoint timestamp;
};

struct StreamSession {
    std::string url;
    std::string sessionId;
};

}