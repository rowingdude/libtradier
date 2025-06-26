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

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/url.hpp>

namespace tradier {

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;

class WebSocketImpl {
private:
    net::io_context ioc_;
    ssl::context ctx_;
    std::unique_ptr<websocket::stream<beast::ssl_stream<tcp::socket>>> ws_;
    std::string host_;
    std::string port_;
    std::string target_;
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
    
    void parseUrl(const std::string& url) {
        try {
            boost::urls::url_view u(url);
            host_ = u.host();
            port_ = u.port().empty() ? "443" : std::string(u.port());
            target_ = u.path().empty() ? "/" : std::string(u.encoded_path());
            if (!u.query().empty()) {
                target_ += "?";
                target_ += u.encoded_query();
            }
        } catch (const std::exception& e) {
            DEBUG_LOG(std::string("URL parsing error: ") + e.what());
            throw ConnectionError("Invalid WebSocket URL: " + url);
        }
    }
    
    void onConnect() {
        DEBUG_LOG("WebSocket connection opened");
        {
            std::lock_guard<std::mutex> lock(connectionMutex_);
            connected_.store(true, std::memory_order_release);
            connecting_.store(false, std::memory_order_release);
        }
        connectionCv_.notify_all();
        
        std::lock_guard<std::mutex> lock(pendingMutex_);
        while (!pendingMessages_.empty() && ws_) {
            try {
                ws_->write(net::buffer(pendingMessages_.front()));
                pendingMessages_.pop();
            } catch (const std::exception& e) {
                DEBUG_LOG(std::string("Failed to send pending message: ") + e.what());
                break;
            }
        }
    }
    
    void onClose() {
        DEBUG_LOG("WebSocket connection closed");
        std::lock_guard<std::mutex> lock(connectionMutex_);
        connected_.store(false, std::memory_order_release);
        connecting_.store(false, std::memory_order_release);
        connectionCv_.notify_all();
    }
    
    void onMessage(const std::string& msg) {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        if (messageCallback_) {
            try {
                messageCallback_(msg);
            } catch (const std::exception& e) {
                DEBUG_LOG(std::string("Message callback error: ") + e.what());
            }
        }
    }
    
    void runIoThread() {
        try {
            DEBUG_LOG("Starting WebSocket connection thread");
            
            // Create the WebSocket stream
            ws_ = std::make_unique<websocket::stream<beast::ssl_stream<tcp::socket>>>(ioc_, ctx_);
            DEBUG_LOG("Created WebSocket stream");
            
            // Look up the domain name
            DEBUG_LOG("Resolving hostname: " + host_ + ":" + port_);
            tcp::resolver resolver(ioc_);
            auto const results = resolver.resolve(host_, port_);
            DEBUG_LOG("DNS resolution successful");
            
            // Make the connection on the IP address we get from a lookup
            DEBUG_LOG("Establishing TCP connection");
            auto ep = net::connect(beast::get_lowest_layer(*ws_), results);
            DEBUG_LOG("TCP connection established to " + ep.address().to_string() + ":" + std::to_string(ep.port()));
            
            // Set SNI Hostname (many hosts require this to handshake successfully)
            DEBUG_LOG("Setting SNI hostname: " + host_);
            if (!SSL_set_tlsext_host_name(ws_->next_layer().native_handle(), host_.c_str())) {
                beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
                throw beast::system_error{ec};
            }
            
            // Update the host string for HTTP
            std::string host_port = host_ + ':' + std::to_string(ep.port());
            DEBUG_LOG("Performing SSL handshake");
            
            // Perform the SSL handshake
            ws_->next_layer().handshake(ssl::stream_base::client);
            DEBUG_LOG("SSL handshake successful");
            
            // Set a decorator to change the User-Agent of the handshake
            ws_->set_option(websocket::stream_base::decorator(
                [this](websocket::request_type& req) {
                    req.set(http::field::user_agent, "libtradier-websocket");
                    if (!authToken_.empty()) {
                        req.set(http::field::authorization, "Bearer " + authToken_);
                    }
                }
            ));
            
            // Perform the websocket handshake
            DEBUG_LOG("Performing WebSocket handshake to " + host_port + target_);
            ws_->handshake(host_port, target_);
            DEBUG_LOG("WebSocket handshake successful");
            
            onConnect();
            
            // Start message reading loop
            while (!shouldStop_.load(std::memory_order_acquire) && ws_->is_open()) {
                beast::flat_buffer buffer;
                
                // This will block until a message is received
                ws_->read(buffer);
                
                std::string message = beast::buffers_to_string(buffer.data());
                onMessage(message);
            }
            
        } catch (const std::exception& e) {
            DEBUG_LOG(std::string("WebSocket IO thread error: ") + e.what());
        }
        
        onClose();
    }
    
public:
    WebSocketImpl(const std::string& url, const std::string& authToken)
        : ctx_(ssl::context::tlsv12_client), authToken_(authToken) {
        parseUrl(url);
        
        try {
            ctx_.set_default_verify_paths();
            // Use less strict SSL verification to match Python websockets behavior
            ctx_.set_verify_mode(ssl::verify_none);
            ctx_.set_options(
                ssl::context::default_workarounds |
                ssl::context::no_sslv2 |
                ssl::context::no_sslv3 |
                ssl::context::single_dh_use
            );
            
            DEBUG_LOG("WebSocket client initialized with Boost.Beast");
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
            if (!ioThread_.joinable()) {
                ioThread_ = std::thread([this]() {
                    this->runIoThread();
                });
            }
            
            std::unique_lock<std::mutex> lock(connectionMutex_);
            connectionCv_.wait_for(lock, std::chrono::seconds(30), [this]() {
                return connected_.load(std::memory_order_acquire) || !connecting_.load(std::memory_order_acquire);
            });
            
            if (!connected_.load(std::memory_order_acquire)) {
                connecting_.store(false, std::memory_order_release);
                throw ConnectionError("Failed to establish WebSocket connection within timeout");
            }
            
        } catch (const std::exception& e) {
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
            if (ws_ && ws_->is_open()) {
                beast::error_code ec;
                ws_->close(websocket::close_code::normal, ec);
                
                std::unique_lock<std::mutex> lock(connectionMutex_);
                connectionCv_.wait_for(lock, std::chrono::seconds(5), [this]() {
                    return !connected_.load(std::memory_order_acquire);
                });
            }
            
            ioc_.stop();
            
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
            if (!connected_.load(std::memory_order_acquire)) {
                throw ConnectionError("WebSocket disconnected during send");
            }
            
            if (ws_) {
                ws_->write(net::buffer(message));
            }
            
        } catch (const std::exception& e) {
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