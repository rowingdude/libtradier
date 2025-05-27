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

#include "tradier/common/websocket_client.hpp"
#include "tradier/common/errors.hpp"
#include <thread>
#include <chrono>
#include <iostream>
#include <atomic>
#include <mutex>
#include <memory>

namespace tradier {

class ThreadGuard {
private:
    std::thread thread_;
    std::atomic<bool>& shouldStop_;

public:
    ThreadGuard(std::thread&& t, std::atomic<bool>& stop) 
        : thread_(std::move(t)), shouldStop_(stop) {}
    
    ~ThreadGuard() {
        shouldStop_.store(true);
        if (thread_.joinable()) {
            thread_.join();
        }
    }
    
    ThreadGuard(const ThreadGuard&) = delete;
    ThreadGuard& operator=(const ThreadGuard&) = delete;
    
    ThreadGuard(ThreadGuard&& other) noexcept 
        : thread_(std::move(other.thread_)), shouldStop_(other.shouldStop_) {
    }
    
    ThreadGuard& operator=(ThreadGuard&& other) noexcept {
        if (this != &other) {
            shouldStop_.store(true);
            if (thread_.joinable()) {
                thread_.join();
            }
            thread_ = std::move(other.thread_);
        }
        return *this;
    }
};

class WebSocketImpl {
private:
    std::string url_;
    std::string authToken_;
    std::atomic<bool> running_{false};
    std::atomic<bool> shouldStop_{false};
    std::unique_ptr<ThreadGuard> workerThread_;
    MessageCallback messageCallback_;
    std::mutex callbackMutex_;
    
public:
    WebSocketImpl(const std::string& url, const std::string& authToken)
        : url_(url), authToken_(authToken) {}
    
    ~WebSocketImpl() {
        disconnect();
    }
    
    WebSocketImpl(const WebSocketImpl&) = delete;
    WebSocketImpl& operator=(const WebSocketImpl&) = delete;
    
    void connect() {
        if (running_) return;
        
        shouldStop_ = false;
        running_ = true;
        
        std::thread worker([this]() {
            while (running_ && !shouldStop_) {
                try {
                    std::lock_guard<std::mutex> lock(callbackMutex_);
                    if (messageCallback_) {
                        std::string mockMessage = R"({"type":"trade","symbol":"SPY","price":"450.25","size":"100","timestamp":")" + 
                            std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::system_clock::now().time_since_epoch()).count()) + R"("})";
                        messageCallback_(mockMessage);
                    }
                } catch (const std::exception&) {
                    running_ = false;
                    break;
                }
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        });
        
        workerThread_ = std::make_unique<ThreadGuard>(std::move(worker), shouldStop_);
    }
    
    void disconnect() {
        running_ = false;
        shouldStop_ = true;
        workerThread_.reset();
    }
    
    void send(const std::string& /*message*/) {
        if (!running_) {
            throw ConnectionError("WebSocket not connected");
        }
    }
    
    void setMessageHandler(MessageCallback callback) {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        messageCallback_ = std::move(callback);
    }
    
    void setAuthToken(const std::string& token) {
        authToken_ = token;
    }
    
    bool isConnected() const {
        return running_;
    }
};

WebSocketConnection::WebSocketConnection(std::unique_ptr<WebSocketImpl> impl)
    : impl_(std::move(impl)) {}

WebSocketConnection::~WebSocketConnection() {
    disconnect();
}

WebSocketConnection::WebSocketConnection(WebSocketConnection&& other) noexcept
    : impl_(std::move(other.impl_)) {}

WebSocketConnection& WebSocketConnection::operator=(WebSocketConnection&& other) noexcept {
    if (this != &other) {
        impl_ = std::move(other.impl_);
    }
    return *this;
}

void WebSocketConnection::connect() {
    if (impl_) {
        impl_->connect();
    }
}

void WebSocketConnection::disconnect() {
    if (impl_) {
        impl_->disconnect();
    }
}

void WebSocketConnection::send(const std::string& message) {
    if (impl_) {
        impl_->send(message);
    }
}

void WebSocketConnection::setMessageHandler(MessageCallback callback) {
    if (impl_) {
        impl_->setMessageHandler(std::move(callback));
    }
}

void WebSocketConnection::setAuthToken(const std::string& token) {
    if (impl_) {
        impl_->setAuthToken(token);
    }
}

bool WebSocketConnection::isConnected() const {
    return impl_ && impl_->isConnected();
}

WebSocketClient::WebSocketClient(const Config& config) : config_(config) {}

WebSocketClient::~WebSocketClient() = default;

WebSocketClient::WebSocketClient(WebSocketClient&&) noexcept = default;
WebSocketClient& WebSocketClient::operator=(WebSocketClient&&) noexcept = default;

WebSocketConnection WebSocketClient::connect(std::string_view endpoint, std::string_view authToken) {
    std::string url = config_.wsUrl();
    if (!endpoint.empty() && endpoint[0] != '/') {
        url += '/';
    }
    url += endpoint;
    
    auto impl = std::make_unique<WebSocketImpl>(url, std::string(authToken));
    return WebSocketConnection(std::move(impl));
}

}