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

#include <functional>
#include <memory>
#include <vector>
#include "tradier/common/types.hpp"
#include "tradier/data.hpp"

namespace tradier {

class TradierClient;

struct MarketEvent {
    std::string type;
    std::string symbol;
    double price = 0.0;
    int size = 0;
    TimePoint timestamp;
};

struct AccountEvent {
    int orderId = 0;
    std::string event;
    std::string status;
    std::string account;
    TimePoint timestamp;
};

using MarketEventHandler = std::function<void(const MarketEvent&)>;
using AccountEventHandler = std::function<void(const AccountEvent&)>;

class StreamingService {
private:
    TradierClient& client_;
    
    class Impl;
    std::unique_ptr<Impl> impl_;
    
public:
    explicit StreamingService(TradierClient& client);
    ~StreamingService();
    
    StreamingService(StreamingService&&) noexcept;
    StreamingService& operator=(StreamingService&&) noexcept;
    StreamingService(const StreamingService&) = delete;
    StreamingService& operator=(const StreamingService&) = delete;
    
    Result<StreamSession> createMarketSession();
    Result<StreamSession> createAccountSession();
    
    bool connectMarket(
        const StreamSession& session,
        const std::vector<std::string>& symbols,
        MarketEventHandler handler
    );
    
    bool connectAccount(
        const StreamSession& session,
        AccountEventHandler handler
    );
    
    void disconnect();
    bool isConnected() const;
};

}