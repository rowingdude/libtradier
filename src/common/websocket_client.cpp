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

namespace tradier {

class WebSocketImpl {
private:
    std::string url_;
    std::string authToken_;
    std::atomic<bool> running_{false};
    std::thread workerThread_;
    MessageCallback messageCallback_;
    
public:
    WebSocketImpl(const std::string& url, const std::string& authToken)
        : url_(url), authToken_(authToken) {}
    
    ~WebSocketImpl() {
        disconnect();
    }
    
    void connect() {
        if (running_) return;
        
        running_ = true;
        workerThread_ = std::thread([this]() {
            while (running_) {
                try {
                    if (messageCallback_) {
                        std::string mockMessage = R"({"type":"trade","symbol":"SPY","price":"450.25","size":"100","timestamp":")" + 
                            std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::system_clock::now().time_since_epoch()).count()) + R"("})";
                        messageCallback_(mockMessage);
                    }
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                } catch (...) {
                    running_ = false;
                    break;
                }
            }
        });
    }
    
    void disconnect() {
        running_ = false;
        if (workerThread_.joinable()) {
            workerThread_.join();
        }
    }
    
    void send(const std::string& /*message*/) {
        if (!running_) {
            throw ConnectionError("WebSocket not connected");
        }
        // Mock implementation - in real version would send message
    }
    
    void setMessageHandler(MessageCallback callback) {
        messageCallback_ = callback;
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
        impl_->setMessageHandler(callback);
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