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
#include <unordered_map>
#include <atomic>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <queue>
#include "tradier/common/types.hpp"

namespace tradier {

class TradierClient;

enum class StreamEventType {
    TRADE,
    QUOTE,
    SUMMARY,
    TIMESALE,
    TRADEX,
    ACCOUNT_ORDER,
    ACCOUNT_POSITION
};

struct TradeEvent {
    std::string type = "trade";
    std::string symbol;
    std::string exchange;
    double price = 0.0;
    int size = 0;
    long cvol = 0;
    std::string date;
    double last = 0.0;
};

struct QuoteEvent {
    std::string type = "quote";
    std::string symbol;
    double bid = 0.0;
    int bidSize = 0;
    std::string bidExchange;
    std::string bidDate;
    double ask = 0.0;
    int askSize = 0;
    std::string askExchange;
    std::string askDate;
};

struct SummaryEvent {
    std::string type = "summary";
    std::string symbol;
    double open = 0.0;
    double high = 0.0;
    double low = 0.0;
    double prevClose = 0.0;
};

struct TimesaleEvent {
    std::string type = "timesale";
    std::string symbol;
    std::string exchange;
    double bid = 0.0;
    double ask = 0.0;
    double last = 0.0;
    int size = 0;
    std::string date;
    int seq = 0;
    std::string flag;
    bool cancel = false;
    bool correction = false;
    std::string session;
};

struct AccountOrderEvent {
    int orderId = 0;
    std::string event;
    std::string status;
    std::string account;
    std::string symbol;
    double quantity = 0.0;
    double price = 0.0;
    std::string side;
    std::string type;
    TimePoint timestamp;
};

struct AccountPositionEvent {
    std::string account;
    std::string symbol;
    double quantity = 0.0;
    double costBasis = 0.0;
    TimePoint timestamp;
};

using TradeEventHandler = std::function<void(const TradeEvent&)>;
using QuoteEventHandler = std::function<void(const QuoteEvent&)>;
using SummaryEventHandler = std::function<void(const SummaryEvent&)>;
using TimesaleEventHandler = std::function<void(const TimesaleEvent&)>;
using AccountOrderEventHandler = std::function<void(const AccountOrderEvent&)>;
using AccountPositionEventHandler = std::function<void(const AccountPositionEvent&)>;
using ErrorHandler = std::function<void(const std::string&)>;

struct StreamingConfig {
    bool autoReconnect = true;
    int reconnectDelay = 5000; // milliseconds
    int maxReconnectAttempts = 10;
    int heartbeatInterval = 30000; // milliseconds
    bool filterDuplicates = true;
    std::vector<std::string> validExchanges;
};

struct StreamSession {
    std::string url;
    std::string sessionId;
    TimePoint expiresAt;
    bool isActive = false;
};

struct StreamStatistics {
    std::atomic<long> messagesReceived{0};
    std::atomic<long> messagesProcessed{0};
    std::atomic<long> errors{0};
    std::atomic<long> reconnects{0};
    
private:
    mutable std::shared_mutex timeMutex_;
    TimePoint connectionStart;
    TimePoint lastMessage;

public:
    StreamStatistics() = default;
    
    StreamStatistics(const StreamStatistics& other) 
        : messagesReceived(other.messagesReceived.load()),
          messagesProcessed(other.messagesProcessed.load()),
          errors(other.errors.load()),
          reconnects(other.reconnects.load()) {
        std::shared_lock lock(other.timeMutex_);
        connectionStart = other.connectionStart;
        lastMessage = other.lastMessage;
    }
    
    StreamStatistics& operator=(const StreamStatistics& other) {
        if (this != &other) {
            messagesReceived.store(other.messagesReceived.load());
            messagesProcessed.store(other.messagesProcessed.load());
            errors.store(other.errors.load());
            reconnects.store(other.reconnects.load());
            
            std::shared_lock otherLock(other.timeMutex_);
            std::unique_lock thisLock(timeMutex_);
            connectionStart = other.connectionStart;
            lastMessage = other.lastMessage;
        }
        return *this;
    }
    
    void setConnectionStart(const TimePoint& time) {
        std::unique_lock lock(timeMutex_);
        connectionStart = time;
    }
    
    void setLastMessage(const TimePoint& time) {
        std::unique_lock lock(timeMutex_);
        lastMessage = time;
    }
    
    TimePoint getConnectionStart() const {
        std::shared_lock lock(timeMutex_);
        return connectionStart;
    }
    
    TimePoint getLastMessage() const {
        std::shared_lock lock(timeMutex_);
        return lastMessage;
    }
    
    struct Snapshot {
        long messagesReceived;
        long messagesProcessed;
        long errors;
        long reconnects;
        TimePoint connectionStart;
        TimePoint lastMessage;
    };
    
    Snapshot getSnapshot() const {
        std::shared_lock lock(timeMutex_);
        return {
            messagesReceived.load(),
            messagesProcessed.load(),
            errors.load(),
            reconnects.load(),
            connectionStart,
            lastMessage
        };
    }
    
    void reset() {
        messagesReceived.store(0);
        messagesProcessed.store(0);
        errors.store(0);
        reconnects.store(0);
        
        std::unique_lock lock(timeMutex_);
        connectionStart = TimePoint{};
        lastMessage = TimePoint{};
    }
};

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
    void renewSession(StreamSession& session);
    
    bool subscribeToTrades(
        const StreamSession& session,
        const std::vector<std::string>& symbols,
        TradeEventHandler handler
    );
    
    bool subscribeToQuotes(
        const StreamSession& session,
        const std::vector<std::string>& symbols,
        QuoteEventHandler handler
    );
    
    bool subscribeToSummary(
        const StreamSession& session,
        const std::vector<std::string>& symbols,
        SummaryEventHandler handler
    );
    
    bool subscribeToTimesales(
        const StreamSession& session,
        const std::vector<std::string>& symbols,
        TimesaleEventHandler handler
    );
    
    bool subscribeToOrderEvents(
        const StreamSession& session,
        AccountOrderEventHandler handler
    );
    
    bool subscribeToPositionEvents(
        const StreamSession& session,
        AccountPositionEventHandler handler
    );
    
    bool addSymbols(const std::vector<std::string>& symbols);
    bool removeSymbols(const std::vector<std::string>& symbols);
    std::vector<std::string> getSubscribedSymbols() const;
    
    void setConfig(const StreamingConfig& config);
    StreamingConfig getConfig() const;
    void setErrorHandler(ErrorHandler handler);
    
    void connect();
    void disconnect();
    bool isConnected() const;
    void reconnect();
    
    StreamStatistics::Snapshot getStatistics() const;
    void resetStatistics();
    std::string getConnectionStatus() const;
    
    void setSymbolFilter(const std::vector<std::string>& symbols);
    void setExchangeFilter(const std::vector<std::string>& exchanges);
    void clearFilters();
};

}