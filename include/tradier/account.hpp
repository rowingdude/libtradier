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
#include "tradier/common/types.hpp"

namespace tradier {

class TradierClient;

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

struct AccountBalances {
    std::string accountNumber;
    std::string accountType;
    double totalEquity = 0.0;
    double totalCash = 0.0;
    double buyingPower = 0.0;
    double marketValue = 0.0;
    double dayChange = 0.0;
};

struct HistoryEvent {
    double amount = 0.0;
    TimePoint date;
    std::string type;
    std::string description;
    std::optional<std::string> symbol;
};

class AccountService {
private:
    TradierClient& client_;
    
public:
    explicit AccountService(TradierClient& client) : client_(client) {}
    
    Result<AccountProfile> getProfile();
    Result<Account> getAccount(const std::string& accountNumber);
    Result<AccountBalances> getBalances(const std::string& accountNumber);
    Result<std::vector<Position>> getPositions(const std::string& accountNumber);
    Result<std::vector<Order>> getOrders(const std::string& accountNumber);
    Result<Order> getOrder(const std::string& accountNumber, int orderId);
    Result<std::vector<HistoryEvent>> getHistory(
        const std::string& accountNumber,
        const std::string& startDate = "",
        const std::string& endDate = ""
    );
};

}