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


#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <queue>
#include <thread>
#include <unordered_set>
#include <vector>

#include "tradier/client.hpp"
#include "tradier/common/debug.hpp"
#include "tradier/common/errors.hpp"
#include "tradier/common/json_utils.hpp"
#include "tradier/common/websocket_client.hpp"
#include "tradier/json/streaming.hpp"
#include "tradier/streaming.hpp"


namespace tradier {

class ThreadManager {
private:
    std::atomic<bool> shouldStop_{false};
    std::vector<std::thread> threads_;
    mutable std::mutex threadsMutex_;
    std::atomic<size_t> threadIdCounter_{0};

public:
    ThreadManager() = default;
    
    ~ThreadManager() {
        stop();
    }
    
    ThreadManager(const ThreadManager&) = delete;
    ThreadManager& operator=(const ThreadManager&) = delete;
    
    ThreadManager(ThreadManager&& other) noexcept {
        std::lock_guard<std::mutex> lock(other.threadsMutex_);
        shouldStop_.store(other.shouldStop_.load());
        threads_ = std::move(other.threads_);
        threadIdCounter_.store(other.threadIdCounter_.load());
        other.shouldStop_.store(true);
    }
    
    ThreadManager& operator=(ThreadManager&& other) noexcept {
        if (this != &other) {
            stop();
            
            std::lock_guard<std::mutex> otherLock(other.threadsMutex_);
            std::lock_guard<std::mutex> thisLock(threadsMutex_);
            
            shouldStop_.store(other.shouldStop_.load());
            threads_ = std::move(other.threads_);
            threadIdCounter_.store(other.threadIdCounter_.load());
            other.shouldStop_.store(true);
        }
        return *this;
    }
    
    template<typename F>
    bool addThread(F&& func) {
        if (shouldStop_.load()) {
            tradier::debug::Logger::getInstance().warn("ThreadManager: Attempt to add thread while stopping");
            return false;
        }
        
        try {
            std::lock_guard<std::mutex> lock(threadsMutex_);
            
            if (shouldStop_.load()) {
                return false;
            }
            
            size_t threadId = threadIdCounter_++;
            
            threads_.emplace_back([func = std::forward<F>(func), threadId, this]() {
                tradier::debug::Logger::getInstance().debug("ThreadManager: Thread " + std::to_string(threadId) + " started");
                
                try {
                    func();
                } catch (const std::exception& e) {
                    tradier::debug::Logger::getInstance().error(
                        "ThreadManager: Thread " + std::to_string(threadId) + " threw exception: " + e.what()
                    );
                } catch (...) {
                    tradier::debug::Logger::getInstance().error(
                        "ThreadManager: Thread " + std::to_string(threadId) + " threw unknown exception"
                    );
                }
                
                tradier::debug::Logger::getInstance().debug("ThreadManager: Thread " + std::to_string(threadId) + " finished");
            });
            
            tradier::debug::Logger::getInstance().info("ThreadManager: Added thread " + std::to_string(threadId) + 
                                                      " (total: " + std::to_string(threads_.size()) + ")");
            return true;
            
        } catch (const std::system_error& e) {
            tradier::debug::Logger::getInstance().error("ThreadManager: Failed to create thread: " + std::string(e.what()));
            return false;
        } catch (const std::exception& e) {
            tradier::debug::Logger::getInstance().error("ThreadManager: Unexpected error creating thread: " + std::string(e.what()));
            return false;
        }
    }
    
    void stop() {
        if (shouldStop_.exchange(true)) {
            return; 
        }
        
        tradier::debug::Logger::getInstance().info("ThreadManager: Initiating shutdown");
        
        std::lock_guard<std::mutex> lock(threadsMutex_);
        
        size_t joinedCount = 0;
        size_t errorCount = 0;
        
        for (size_t i = 0; i < threads_.size(); ++i) {
            auto& thread = threads_[i];
            if (thread.joinable()) {
                try {
                    thread.join();
                    joinedCount++;
                    tradier::debug::Logger::getInstance().trace("ThreadManager: Successfully joined thread " + std::to_string(i));
                } catch (const std::system_error& e) {
                    errorCount++;
                    tradier::debug::Logger::getInstance().error(
                        "ThreadManager: Failed to join thread " + std::to_string(i) + ": " + e.what()
                    );
                } catch (const std::exception& e) {
                    errorCount++;
                    tradier::debug::Logger::getInstance().error(
                        "ThreadManager: Unexpected error joining thread " + std::to_string(i) + ": " + e.what()
                    );
                }
            }
        }
        
        threads_.clear();
        
        tradier::debug::Logger::getInstance().info(
            "ThreadManager: Shutdown complete - joined: " + std::to_string(joinedCount) + 
            ", errors: " + std::to_string(errorCount)
        );
    }
    
    bool shouldStop() const { 
        return shouldStop_.load(); 
    }
    
    size_t threadCount() const {
        std::lock_guard<std::mutex> lock(threadsMutex_);
        return threads_.size();
    }
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
    mutable std::mutex subscriptionMutex;

    ThreadManager threadManager;
    std::mutex connectionMutex;
    std::condition_variable connectionCv;

    StreamStatistics stats;

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
        stats.messagesReceived++;
        stats.setLastMessage(std::chrono::system_clock::now());
        
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
        tradier::debug::Logger::getInstance().info("StreamingService: Starting heartbeat thread");
        
        bool success = threadManager.addThread([this]() {
            tradier::debug::logThreadInfo("Heartbeat thread started", 
                                        "interval=" + std::to_string(config.heartbeatInterval) + "ms");
            
            size_t heartbeatCount = 0;
            size_t errorCount = 0;
            auto lastSuccessfulHeartbeat = std::chrono::steady_clock::now();
            
            while (connected && !threadManager.shouldStop()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(config.heartbeatInterval));
                
                if (!connected || threadManager.shouldStop()) {
                    break;
                }
                
                if (connection) {
                    try {
                        PERF_TIMER("heartbeat_send");
                        
                        nlohmann::json heartbeat;
                        heartbeat["type"] = "heartbeat";
                        heartbeat["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch()).count();
                        
                        connection->send(heartbeat.dump());
                        
                        heartbeatCount++;
                        errorCount = 0; 
                        lastSuccessfulHeartbeat = std::chrono::steady_clock::now();
                        
                        tradier::debug::Logger::getInstance().trace(
                            "StreamingService: Heartbeat sent #" + std::to_string(heartbeatCount)
                        );
                        
                    } catch (const std::exception& e) {
                        errorCount++;
                        
                        tradier::debug::Logger::getInstance().error(
                            "StreamingService: Heartbeat error #" + std::to_string(errorCount) + ": " + e.what()
                        );
                        
                        if (errorHandler) {
                            errorHandler("Heartbeat error: " + std::string(e.what()));
                        }
                        
                        if (errorCount >= 3) {
                            tradier::debug::Logger::getInstance().error(
                                "StreamingService: Too many consecutive heartbeat failures, stopping heartbeat thread"
                            );
                            break;
                        }
                        
                        auto timeSinceSuccess = std::chrono::steady_clock::now() - lastSuccessfulHeartbeat;
                        if (timeSinceSuccess > std::chrono::minutes(5)) {
                            tradier::debug::Logger::getInstance().error(
                                "StreamingService: No successful heartbeat for 5 minutes, stopping heartbeat thread"
                            );
                            break;
                        }
                    }
                } else {
                    tradier::debug::Logger::getInstance().warn("StreamingService: No connection available for heartbeat");
                    break;
                }
            }
            
            tradier::debug::logThreadInfo("Heartbeat thread finished", 
                                        "sent=" + std::to_string(heartbeatCount) + 
                                        " errors=" + std::to_string(errorCount));
        });
        
        if (!success) {
            tradier::debug::Logger::getInstance().error("StreamingService: Failed to start heartbeat thread");
            if (errorHandler) {
                errorHandler("Failed to start heartbeat thread");
            }
        }
    }
    
    void disconnect() {
        connected = false;
        threadManager.stop();
        
        if (connection) {
            connection->disconnect();
            connection.reset();
        }
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
    return tryExecute<StreamSession>([&]() -> StreamSession {
        auto response = impl_->client.post("/markets/events/session");
        
        if (!response.success()) {
            throw ApiError(response.status, "Failed to create market session: " + response.body);
        }
        
        auto parsed = json::parseResponse<StreamSession>(response, json::parseStreamSession);
        if (!parsed) {
            throw std::runtime_error("Failed to parse market session response");
        }
        
        parsed->isActive = !parsed->url.empty() && !parsed->sessionId.empty();
        impl_->currentSession = *parsed;
        
        return *parsed;
    }, "createMarketSession");
}

Result<StreamSession> StreamingService::createAccountSession() {
    return tryExecute<StreamSession>([&]() -> StreamSession {
        auto response = impl_->client.post("/accounts/events/session");
        
        if (!response.success()) {
            throw ApiError(response.status, "Failed to create account session: " + response.body);
        }
        
        auto parsed = json::parseResponse<StreamSession>(response, json::parseStreamSession);
        if (!parsed) {
            throw std::runtime_error("Failed to parse account session response");
        }
        
        parsed->isActive = !parsed->url.empty() && !parsed->sessionId.empty();
        impl_->currentSession = *parsed;
        
        return *parsed;
    }, "createAccountSession");
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
    
    if (!session.isActive || symbols.empty() || !handler) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(impl_->connectionMutex);
    impl_->tradeHandler = handler;
    
    {
        std::lock_guard<std::mutex> subLock(impl_->subscriptionMutex);
        for (const auto& symbol : symbols) {
            impl_->subscribedSymbols.insert(symbol);
        }
    }
    
    if (!impl_->connected) {
        connect();
    }
    
    if (impl_->connection && impl_->connected) {
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
        impl_->stats.setConnectionStart(std::chrono::system_clock::now()); 
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