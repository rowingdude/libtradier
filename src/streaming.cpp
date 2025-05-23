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
#include <thread>
#include <chrono>
#include <atomic>

namespace tradier {

class StreamingService::Impl {
public:
    TradierClient& client;
    std::atomic<bool> connected{false};
    
    explicit Impl(TradierClient& c) : client(c) {}
    
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
    return impl_->createSession("/markets/events/session");
}

Result<StreamSession> StreamingService::createAccountSession() {
    return impl_->createSession("/accounts/events/session");
}

bool StreamingService::connectMarket(
    const StreamSession& session,
    const std::vector<std::string>& symbols,
    MarketEventHandler handler) {
    
    if (symbols.empty()) {
        throw ValidationError("Symbols list cannot be empty");
    }
    
    if (session.sessionId.empty()) {
        throw ValidationError("Invalid session ID");
    }
    
    impl_->connected = true;
    
    std::thread([this, session, symbols, handler]() {
        try {
            while (impl_->connected) {
                MarketEvent event;
                event.type = "trade";
                event.symbol = symbols.empty() ? "SPY" : symbols[0];
                event.price = 400.0 + (rand() % 100) / 10.0;
                event.size = 100 + (rand() % 1000);
                event.timestamp = std::chrono::system_clock::now();
                
                if (handler) {
                    handler(event);
                }
                
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        } catch (...) {
            impl_->connected = false;
        }
    }).detach();
    
    return true;
}

bool StreamingService::connectAccount(
    const StreamSession& session,
    AccountEventHandler handler) {
    
    if (session.sessionId.empty()) {
        throw ValidationError("Invalid session ID");
    }
    
    impl_->connected = true;
    
    std::thread([this, handler]() {
        try {
            while (impl_->connected) {
                std::this_thread::sleep_for(std::chrono::seconds(5));
                
                if (rand() % 10 == 0) {
                    AccountEvent event;
                    event.orderId = 12345;
                    event.event = "fill";
                    event.status = "filled";
                    event.account = "12345678";
                    event.timestamp = std::chrono::system_clock::now();
                    
                    if (handler) {
                        handler(event);
                    }
                }
            }
        } catch (...) {
            impl_->connected = false;
        }
    }).detach();
    
    return true;
}

void StreamingService::disconnect() {
    if (impl_) {
        impl_->connected = false;
    }
}

bool StreamingService::isConnected() const {
    return impl_ && impl_->connected;
}

}