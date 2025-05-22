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
#include "tradier/data.hpp"

namespace tradier {

class TradierClient;

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