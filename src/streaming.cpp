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

#include "tradier/streaming.hpp"
#include "tradier/client.hpp"
#include "tradier/common/errors.hpp"
#include "tradier/common/json_utils.hpp"
#include "tradier/json/streaming.hpp"
#include "tradier/common/websocket_client.hpp"
#include <nlohmann/json.hpp>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <unordered_set>
#include <memory>
#include <vector>

namespace tradier {

class ThreadManager {
private:
    std::atomic<bool> shouldStop_{false};
    std::vector<std::thread> threads_;
    std::mutex threadsMutex_;

public:
    ThreadManager() = default;
    
    ~ThreadManager() {
        stop();
    }
    
    ThreadManager(const ThreadManager&) = delete;
    ThreadManager& operator=(const ThreadManager&) = delete;
    
    ThreadManager(ThreadManager&&) = default;
    ThreadManager& operator=(ThreadManager&&) = default;
    
    template<typename F>
    void addThread(F&& func) {
        std::lock_guard<std::mutex> lock(threadsMutex_);
        threads_.emplace_back(std::forward<F>(func));
    }
    
    void stop() {
        shouldStop_ = true;
        std::lock_guard<std::mutex> lock(threadsMutex_);
        for (auto& thread : threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        threads_.clear();
    }
    
    bool shouldStop() const { return shouldStop_; }
};

class StreamingService::Impl {
public:
    TradierClient& client;
    StreamingConfig config;
    std::unique_ptr<WebSocketConnection> connection;
    std::atomic<bool> connected{false};

    TradeEventHandler tradeHandler;
    QuoteEventHandler quoteHandler;
    SummaryEventHandler summaryHandler;
    TimesaleEventHandler timesaleHandler;
    AccountOrderEventHandler orderHandler;
    AccountPositionEventHandler positionHandler;
    ErrorHandler errorHandler;

    std::unordered_set<std::string> subscribedSymbols;
    std::unordered_set<std::string> symbolFilter;
    std::unordered_set<std::string> exchangeFilter;
    std::mutex subscriptionMutex;

    ThreadManager threadManager;
    std::mutex connectionMutex;
    std::condition_variable connectionCv;

    StreamStatistics stats;
    std::mutex statsMutex;

    StreamSession currentSession;
    
    explicit Impl(TradierClient& c) : client(c) {
        config.autoReconnect = true;
        config.reconnectDelay = 5000;
        config.maxReconnectAttempts = 10;
        config.heartbeatInterval = 30000;
        config.filterDuplicates = true;
    }
    
    ~Impl() {
        disconnect();
    }

private: 

    double parseNumericField(const nlohmann::json& json, const std::string& key, double defaultValue) {
        if (!json.contains(key) || json[key].is_null()) {
            return defaultValue;
        }
        
        if (json[key].is_number()) {
            return json[key].get<double>();
        } else if (json[key].is_string()) {
            try {
                std::string str = json[key].get<std::string>();
                if (str.empty()) return defaultValue;
                return std::stod(str);
            } catch (const std::exception&) {
                return defaultValue;
            }
        }
        
        return defaultValue;
    }

public:
    void handleMessage(const std::string& message) {
        std::lock_guard<std::mutex> lock(statsMutex);
        stats.messagesReceived++;
        stats.lastMessage = std::chrono::system_clock::now();
        
        try {
            auto json = nlohmann::json::parse(message);
            processEvent(json);
            stats.messagesProcessed++;
        } catch (const std::exception& e) {
            stats.errors++;
            if (errorHandler) {
                errorHandler("Message parsing error: " + std::string(e.what()));
            }
        }
    }
    
    void processEvent(const nlohmann::json& json) {
        if (!json.contains("type")) return;
        
        std::string type = json["type"];
        std::string symbol = json.value("symbol", "");

        if (!symbolFilter.empty() && symbolFilter.find(symbol) == symbolFilter.end()) {
            return;
        }

        if (!exchangeFilter.empty() && json.contains("exch")) {
            std::string exchange = json["exch"];
            if (exchangeFilter.find(exchange) == exchangeFilter.end()) {
                return;
            }
        }
        
        try {
            if (type == "trade" && tradeHandler) {
                TradeEvent event;
                event.type = type;
                event.symbol = symbol;
                event.exchange = json.value("exch", "");
                
                event.price = parseNumericField(json, "price", 0.0);
                event.size = static_cast<int>(parseNumericField(json, "size", 0.0));
                event.cvol = static_cast<long>(parseNumericField(json, "cvol", 0.0));
                event.last = parseNumericField(json, "last", 0.0);
                
                event.date = json.value("date", "");
                tradeHandler(event);
                
            } else if (type == "quote" && quoteHandler) {
                QuoteEvent event;
                event.type = type;
                event.symbol = symbol;
                
                event.bid = parseNumericField(json, "bid", 0.0);
                event.ask = parseNumericField(json, "ask", 0.0);
                event.bidSize = static_cast<int>(parseNumericField(json, "bidsz", 0.0));
                event.askSize = static_cast<int>(parseNumericField(json, "asksz", 0.0));
                
                event.bidExchange = json.value("bidexch", "");
                event.bidDate = json.value("biddate", "");
                event.askExchange = json.value("askexch", "");
                event.askDate = json.value("askdate", "");
                quoteHandler(event);
                
            } else if (type == "summary" && summaryHandler) {
                SummaryEvent event;
                event.type = type;
                event.symbol = symbol;
                
                event.open = parseNumericField(json, "open", 0.0);
                event.high = parseNumericField(json, "high", 0.0);
                event.low = parseNumericField(json, "low", 0.0);
                event.prevClose = parseNumericField(json, "prevClose", 0.0);
                
                summaryHandler(event);
                
            } else if (type == "timesale" && timesaleHandler) {
                TimesaleEvent event;
                event.type = type;
                event.symbol = symbol;
                event.exchange = json.value("exch", "");
                
                event.bid = parseNumericField(json, "bid", 0.0);
                event.ask = parseNumericField(json, "ask", 0.0);
                event.last = parseNumericField(json, "last", 0.0);
                event.size = static_cast<int>(parseNumericField(json, "size", 0.0));
                event.seq = static_cast<int>(parseNumericField(json, "seq", 0.0));
                
                event.date = json.value("date", "");
                event.flag = json.value("flag", "");
                event.cancel = json.value("cancel", false);
                event.correction = json.value("correction", false);
                event.session = json.value("session", "");
                timesaleHandler(event);
            }
            
        } catch (const std::exception& e) {
            if (errorHandler) {
                errorHandler("Event processing error: " + std::string(e.what()));
            }
        }
    }
    
    void startHeartbeat() {
        threadManager.addThread([this]() {
            while (connected && !threadManager.shouldStop()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(config.heartbeatInterval));
                
                if (connected && connection) {
                    try {
                        nlohmann::json heartbeat;
                        heartbeat["type"] = "heartbeat";
                        connection->send(heartbeat.dump());
                    } catch (const std::exception& e) {
                        if (errorHandler) {
                            errorHandler("Heartbeat error: " + std::string(e.what()));
                        }
                    }
                }
            }
        });
    }
    
    void disconnect() {
        connected = false;
        threadManager.stop();
        
        if (connection) {
            connection->disconnect();
            connection.reset();
        }
    }
    
    Result<StreamSession> createSession(const std::string& endpoint) {
        auto response = client.post(endpoint);
        return json::parseResponse<StreamSession>(response, json::parseStreamSession);
    }
};

StreamingService::StreamingService(TradierClient& client) 
    : client_(client), impl_(std::make_unique<Impl>(client)) {}

StreamingService::~StreamingService() {
    disconnect();
}

StreamingService::StreamingService(StreamingService&& other) noexcept 
    : client_(other.client_), impl_(std::move(other.impl_)) {}

StreamingService& StreamingService::operator=(StreamingService&& other) noexcept {
    if (this != &other) {
        impl_ = std::move(other.impl_);
    }
    return *this;
}

Result<StreamSession> StreamingService::createMarketSession() {
    auto response = impl_->client.post("/markets/events/session");
    auto sessionResult = json::parseResponse<StreamSession>(response, json::parseStreamSession);
    
    if (sessionResult) {
        sessionResult->isActive = !sessionResult->url.empty() && !sessionResult->sessionId.empty();
        impl_->currentSession = *sessionResult;
    }
    
    return sessionResult;
}

Result<StreamSession> StreamingService::createAccountSession() {
    auto response = impl_->client.post("/accounts/events/session");
    auto sessionResult = json::parseResponse<StreamSession>(response, json::parseStreamSession);
    
    if (sessionResult) {
        sessionResult->isActive = !sessionResult->url.empty() && !sessionResult->sessionId.empty();
        impl_->currentSession = *sessionResult;
    }
    
    return sessionResult;
}

void StreamingService::renewSession(StreamSession& session) {
    if (session.url.find("markets") != std::string::npos) {
        auto newSession = createMarketSession();
        if (newSession) {
            session = *newSession;
        }
    } else {
        auto newSession = createAccountSession();
        if (newSession) {
            session = *newSession;
        }
    }
}

bool StreamingService::subscribeToTrades(
    const StreamSession& session,
    const std::vector<std::string>& symbols,
    TradeEventHandler handler) {
    
    if (!session.isActive || symbols.empty()) {
        return false;
    }
    
    impl_->tradeHandler = handler;
    
    {
        std::lock_guard<std::mutex> lock(impl_->subscriptionMutex);
        for (const auto& symbol : symbols) {
            impl_->subscribedSymbols.insert(symbol);
        }
    }
    
    if (!impl_->connected) {
        connect();
    }
    
    if (impl_->connection) {
        nlohmann::json subscription;
        subscription["type"] = "subscribe";
        subscription["to"] = "trade";
        subscription["symbols"] = symbols;
        
        try {
            impl_->connection->send(subscription.dump());
            return true;
        } catch (const std::exception& e) {
            if (impl_->errorHandler) {
                impl_->errorHandler("Subscription error: " + std::string(e.what()));
            }
            return false;
        }
    }
    
    return false;
}

bool StreamingService::subscribeToQuotes(
    const StreamSession& session,
    const std::vector<std::string>& symbols,
    QuoteEventHandler handler) {
    
    if (!session.isActive || symbols.empty()) {
        return false;
    }
    
    impl_->quoteHandler = handler;
    
    {
        std::lock_guard<std::mutex> lock(impl_->subscriptionMutex);
        for (const auto& symbol : symbols) {
            impl_->subscribedSymbols.insert(symbol);
        }
    }
    
    if (!impl_->connected) {
        connect();
    }
    
    if (impl_->connection) {
        nlohmann::json subscription;
        subscription["type"] = "subscribe";
        subscription["to"] = "quote";
        subscription["symbols"] = symbols;
        
        try {
            impl_->connection->send(subscription.dump());
            return true;
        } catch (const std::exception& e) {
            if (impl_->errorHandler) {
                impl_->errorHandler("Subscription error: " + std::string(e.what()));
            }
            return false;
        }
    }
    
    return false;
}

bool StreamingService::subscribeToSummary(
    const StreamSession& session,
    const std::vector<std::string>& symbols,
    SummaryEventHandler handler) {
    
    if (!session.isActive || symbols.empty()) {
        return false;
    }
    
    impl_->summaryHandler = handler;
    
    {
        std::lock_guard<std::mutex> lock(impl_->subscriptionMutex);
        for (const auto& symbol : symbols) {
            impl_->subscribedSymbols.insert(symbol);
        }
    }
    
    if (!impl_->connected) {
        connect();
    }
    
    if (impl_->connection) {
        nlohmann::json subscription;
        subscription["type"] = "subscribe";
        subscription["to"] = "summary";
        subscription["symbols"] = symbols;
        
        try {
            impl_->connection->send(subscription.dump());
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }
    
    return false;
}

bool StreamingService::subscribeToTimesales(
    const StreamSession& session,
    const std::vector<std::string>& symbols,
    TimesaleEventHandler handler) {
    
    if (!session.isActive || symbols.empty()) {
        return false;
    }
    
    impl_->timesaleHandler = handler;
    
    {
        std::lock_guard<std::mutex> lock(impl_->subscriptionMutex);
        for (const auto& symbol : symbols) {
            impl_->subscribedSymbols.insert(symbol);
        }
    }
    
    if (!impl_->connected) {
        connect();
    }
    
    if (impl_->connection) {
        nlohmann::json subscription;
        subscription["type"] = "subscribe";
        subscription["to"] = "timesale";
        subscription["symbols"] = symbols;
        
        try {
            impl_->connection->send(subscription.dump());
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }
    
    return false;
}

bool StreamingService::subscribeToOrderEvents(
    const StreamSession& session,
    AccountOrderEventHandler handler) {
    
    if (!session.isActive) {
        return false;
    }
    
    impl_->orderHandler = handler;
    
    if (!impl_->connected) {
        connect();
    }
    
    if (impl_->connection) {
        nlohmann::json subscription;
        subscription["type"] = "subscribe";
        subscription["to"] = "order";
        
        try {
            impl_->connection->send(subscription.dump());
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }
    
    return false;
}

bool StreamingService::subscribeToPositionEvents(
    const StreamSession& session,
    AccountPositionEventHandler handler) {
    
    if (!session.isActive) {
        return false;
    }
    
    impl_->positionHandler = handler;
    
    if (!impl_->connected) {
        connect();
    }
    
    if (impl_->connection) {
        nlohmann::json subscription;
        subscription["type"] = "subscribe";
        subscription["to"] = "position";
        
        try {
            impl_->connection->send(subscription.dump());
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }
    
    return false;
}

bool StreamingService::addSymbols(const std::vector<std::string>& symbols) {
    std::lock_guard<std::mutex> lock(impl_->subscriptionMutex);
    for (const auto& symbol : symbols) {
        impl_->subscribedSymbols.insert(symbol);
    }
    return true;
}

bool StreamingService::removeSymbols(const std::vector<std::string>& symbols) {
    std::lock_guard<std::mutex> lock(impl_->subscriptionMutex);
    for (const auto& symbol : symbols) {
        impl_->subscribedSymbols.erase(symbol);
    }
    return true;
}

std::vector<std::string> StreamingService::getSubscribedSymbols() const {
    std::lock_guard<std::mutex> lock(impl_->subscriptionMutex);
    return std::vector<std::string>(impl_->subscribedSymbols.begin(), impl_->subscribedSymbols.end());
}

void StreamingService::connect() {
    if (impl_->connected || !impl_->currentSession.isActive) {
        return;
    }
    
    try {
        WebSocketClient wsClient(client_.config());
        impl_->connection = std::make_unique<WebSocketConnection>(
            wsClient.connect(impl_->currentSession.url, client_.config().accessToken)
        );
        
        impl_->connection->setMessageHandler([this](const std::string& message) {
            impl_->handleMessage(message);
        });
        
        impl_->connection->setAuthToken(client_.config().accessToken);
        impl_->connection->connect();
        
        impl_->connected = true;
        impl_->stats.connectionStart = std::chrono::system_clock::now();
        impl_->startHeartbeat();
        
    } catch (const std::exception& e) {
        if (impl_->errorHandler) {
            impl_->errorHandler("Connection error: " + std::string(e.what()));
        }
        impl_->connected = false;
    }
}

void StreamingService::disconnect() {
    impl_->disconnect();
}

bool StreamingService::isConnected() const {
    return impl_->connected;
}

void StreamingService::reconnect() {
    disconnect();
    std::this_thread::sleep_for(std::chrono::milliseconds(impl_->config.reconnectDelay));
    connect();
}

void StreamingService::setConfig(const StreamingConfig& config) {
    impl_->config = config;
}

StreamingConfig StreamingService::getConfig() const {
    return impl_->config;
}

void StreamingService::setErrorHandler(ErrorHandler handler) {
    impl_->errorHandler = handler;
}

StreamStatistics::Snapshot StreamingService::getStatistics() const {
    return impl_->stats.getSnapshot();
}

void StreamingService::resetStatistics() {
    std::lock_guard<std::mutex> lock(impl_->statsMutex);
    impl_->stats.reset();
}

std::string StreamingService::getConnectionStatus() const {
    if (impl_->connected) {
        return "Connected";
    }
    return "Disconnected";
}

void StreamingService::setSymbolFilter(const std::vector<std::string>& symbols) {
    std::lock_guard<std::mutex> lock(impl_->subscriptionMutex);
    impl_->symbolFilter = std::unordered_set<std::string>(symbols.begin(), symbols.end());
}

void StreamingService::setExchangeFilter(const std::vector<std::string>& exchanges) {
    std::lock_guard<std::mutex> lock(impl_->subscriptionMutex);
    impl_->exchangeFilter = std::unordered_set<std::string>(exchanges.begin(), exchanges.end());
}

void StreamingService::clearFilters() {
    std::lock_guard<std::mutex> lock(impl_->subscriptionMutex);
    impl_->symbolFilter.clear();
    impl_->exchangeFilter.clear();
}

}