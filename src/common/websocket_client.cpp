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
#include "tradier/common/debug.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <queue>
#include <iostream>

#if WEBSOCKETPP_ENABLED
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#endif

namespace tradier {

#if WEBSOCKETPP_ENABLED

using WsClient = websocketpp::client<websocketpp::config::asio_tls_client>;
using MessagePtr = websocketpp::config::asio_client::message_type::ptr;
using ContextPtr = websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context>;

class WebSocketImpl {
private:
    WsClient client_;
    websocketpp::connection_hdl hdl_;
    std::string url_;
    std::string authToken_;
    std::atomic<bool> connected_{false};
    std::atomic<bool> connecting_{false};
    std::atomic<bool> shouldStop_{false};
    MessageCallback messageCallback_;
    std::thread ioThread_;
    std::mutex callbackMutex_;
    std::mutex connectionMutex_;
    std::condition_variable connectionCv_;
    std::queue<std::string> pendingMessages_;
    std::mutex pendingMutex_;
    
    static ContextPtr onTlsInit(websocketpp::connection_hdl) {
        ContextPtr ctx = websocketpp::lib::make_shared<websocketpp::lib::asio::ssl::context>(
            websocketpp::lib::asio::ssl::context::tlsv12);
        
        try {
            ctx->set_options(websocketpp::lib::asio::ssl::context::default_workarounds |
                           websocketpp::lib::asio::ssl::context::no_sslv2 |
                           websocketpp::lib::asio::ssl::context::no_sslv3 |
                           websocketpp::lib::asio::ssl::context::single_dh_use);
            
            ctx->set_verify_mode(websocketpp::lib::asio::ssl::verify_peer);
            ctx->set_default_verify_paths();
            
            DEBUG_LOG("TLS context initialized");
        } catch (std::exception& e) {
            DEBUG_LOG(std::string("TLS initialization error: ") + e.what());
        }
        
        return ctx;
    }
    
    void onOpen(websocketpp::connection_hdl hdl) {
        DEBUG_LOG("WebSocket connection opened");
        {
            std::lock_guard<std::mutex> lock(connectionMutex_);
            hdl_ = hdl;
            connected_.store(true, std::memory_order_release);
            connecting_.store(false, std::memory_order_release);
        }
        connectionCv_.notify_all();
        
        std::lock_guard<std::mutex> lock(pendingMutex_);
        while (!pendingMessages_.empty()) {
            try {
                client_.send(hdl_, pendingMessages_.front(), websocketpp::frame::opcode::text);
                pendingMessages_.pop();
            } catch (const websocketpp::exception& e) {
                DEBUG_LOG(std::string("Failed to send pending message: ") + e.what());
                break;
            }
        }
    }
    
    void onClose(websocketpp::connection_hdl) {
        DEBUG_LOG("WebSocket connection closed");
        std::lock_guard<std::mutex> lock(connectionMutex_);
        connected_.store(false, std::memory_order_release);
        connecting_.store(false, std::memory_order_release);
        connectionCv_.notify_all();
    }
    
    void onFail(websocketpp::connection_hdl) {
        DEBUG_LOG("WebSocket connection failed");
        std::lock_guard<std::mutex> lock(connectionMutex_);
        connected_.store(false, std::memory_order_release);
        connecting_.store(false, std::memory_order_release);
        connectionCv_.notify_all();
    }
    
    void onMessage(websocketpp::connection_hdl, MessagePtr msg) {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        if (messageCallback_) {
            try {
                messageCallback_(msg->get_payload());
            } catch (const std::exception& e) {
                DEBUG_LOG(std::string("Message callback error: ") + e.what());
            }
        }
    }
    
public:
    WebSocketImpl(const std::string& url, const std::string& authToken)
        : url_(url), authToken_(authToken) {
        
        try {
            client_.clear_access_channels(websocketpp::log::alevel::all);
            client_.set_error_channels(websocketpp::log::elevel::all);
            
            client_.init_asio();
            
            client_.set_open_handler(std::bind(&WebSocketImpl::onOpen, this, std::placeholders::_1));
            client_.set_close_handler(std::bind(&WebSocketImpl::onClose, this, std::placeholders::_1));
            client_.set_fail_handler(std::bind(&WebSocketImpl::onFail, this, std::placeholders::_1));
            client_.set_message_handler(std::bind(&WebSocketImpl::onMessage, this, 
                std::placeholders::_1, std::placeholders::_2));
            
            client_.set_tls_init_handler(std::bind(&WebSocketImpl::onTlsInit, std::placeholders::_1));
            
        } catch (const std::exception& e) {
            throw ConnectionError("Failed to initialize WebSocket client: " + std::string(e.what()));
        }
    }
    
    ~WebSocketImpl() {
        disconnect();
    }
    
    void connect() {
        if (connected_.load(std::memory_order_acquire) || connecting_.load(std::memory_order_acquire)) {
            return;
        }
        
        bool expected = false;
        if (!connecting_.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
            return;
        }
        
        shouldStop_.store(false, std::memory_order_release);
        
        try {
            websocketpp::lib::error_code ec;
            WsClient::connection_ptr con = client_.get_connection(url_, ec);
            
            if (ec) {
                connecting_.store(false, std::memory_order_release);
                throw ConnectionError("Failed to create connection: " + ec.message());
            }
            
            if (!authToken_.empty()) {
                con->append_header("Authorization", "Bearer " + authToken_);
            }
            
            client_.connect(con);
            
            if (!ioThread_.joinable()) {
                ioThread_ = std::thread([this]() {
                    try {
                        client_.run();
                    } catch (const std::exception& e) {
                        DEBUG_LOG(std::string("WebSocket IO thread error: ") + e.what());
                        connected_.store(false, std::memory_order_release);
                        connecting_.store(false, std::memory_order_release);
                    }
                });
            }
            
            std::unique_lock<std::mutex> lock(connectionMutex_);
            connectionCv_.wait_for(lock, std::chrono::seconds(10), [this]() {
                return connected_.load(std::memory_order_acquire) || !connecting_.load(std::memory_order_acquire);
            });
            
            if (!connected_.load(std::memory_order_acquire)) {
                connecting_.store(false, std::memory_order_release);
                throw ConnectionError("Failed to establish WebSocket connection within timeout");
            }
            
        } catch (const websocketpp::exception& e) {
            connecting_.store(false, std::memory_order_release);
            throw ConnectionError("WebSocket connection error: " + std::string(e.what()));
        } catch (...) {
            connecting_.store(false, std::memory_order_release);
            throw;
        }
    }
    
    void disconnect() {
        if (!connected_.load(std::memory_order_acquire) && !connecting_.load(std::memory_order_acquire)) {
            return;
        }
        
        shouldStop_.store(true, std::memory_order_release);
        
        try {
            if (connected_.load(std::memory_order_acquire)) {
                websocketpp::lib::error_code ec;
                client_.close(hdl_, websocketpp::close::status::going_away, "Client disconnecting", ec);
                
                std::unique_lock<std::mutex> lock(connectionMutex_);
                connectionCv_.wait_for(lock, std::chrono::seconds(5), [this]() {
                    return !connected_.load(std::memory_order_acquire);
                });
            }
            
            client_.stop();
            
            if (ioThread_.joinable()) {
                ioThread_.join();
            }
            
        } catch (const std::exception& e) {
            DEBUG_LOG(std::string("Error during disconnect: ") + e.what());
        }
        
        connected_.store(false, std::memory_order_release);
        connecting_.store(false, std::memory_order_release);
        
        {
            std::lock_guard<std::mutex> lock(pendingMutex_);
            while (!pendingMessages_.empty()) {
                pendingMessages_.pop();
            }
        }
    }
    
    void send(const std::string& message) {
        if (!connected_.load(std::memory_order_acquire)) {
            if (connecting_.load(std::memory_order_acquire)) {
                std::lock_guard<std::mutex> lock(pendingMutex_);
                pendingMessages_.push(message);
                return;
            }
            throw ConnectionError("WebSocket not connected");
        }
        
        try {
            websocketpp::lib::error_code ec;
            

            if (!connected_.load(std::memory_order_acquire)) {
                throw ConnectionError("WebSocket disconnected during send");
            }
            
            client_.send(hdl_, message, websocketpp::frame::opcode::text, ec);
            
            if (ec) {
                throw ConnectionError("Failed to send message: " + ec.message());
            }
        } catch (const websocketpp::exception& e) {
            throw ConnectionError("WebSocket send error: " + std::string(e.what()));
        }
    }
    
    void setMessageHandler(MessageCallback callback) {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        messageCallback_ = std::move(callback);
    }
    
    void setAuthToken(const std::string& token) {
        if (connected_.load(std::memory_order_acquire) || connecting_.load(std::memory_order_acquire)) {
            throw ValidationError("Cannot change auth token while connected");
        }
        authToken_ = token;
    }
    
    bool isConnected() const {
        return connected_.load(std::memory_order_acquire);
    }
};

#else

class WebSocketImpl {
private:
    std::string url_;
    std::string authToken_;
    MessageCallback messageCallback_;

public:
    WebSocketImpl(const std::string& url, const std::string& authToken)
        : url_(url), authToken_(authToken) {
        DEBUG_LOG("WebSocket functionality disabled (websocketpp not available)");
    }
    
    ~WebSocketImpl() = default;
    
    void connect() {
        throw ConnectionError("WebSocket functionality is not available (websocketpp issues)");
    }
    
    void disconnect() {
    }
    
    void send(const std::string&) {
        throw ConnectionError("WebSocket functionality is not available (websocketpp issues)");
    }
    
    void setMessageHandler(MessageCallback callback) {
        messageCallback_ = std::move(callback);
    }
    
    void setAuthToken(const std::string& token) {
        authToken_ = token;
    }
    
    bool isConnected() const {
        return false;
    }
};

#endif


WebSocketConnection::WebSocketConnection(std::unique_ptr<WebSocketImpl> impl)
    : impl_(std::move(impl)) {}

WebSocketConnection::~WebSocketConnection() {
    disconnect();
}

WebSocketConnection::WebSocketConnection(WebSocketConnection&& other) noexcept
    : impl_(std::move(other.impl_)) {}

WebSocketConnection& WebSocketConnection::operator=(WebSocketConnection&& other) noexcept {
    if (this != &other) {
        disconnect();
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
    } else {
        throw ConnectionError("WebSocket connection not initialized");
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