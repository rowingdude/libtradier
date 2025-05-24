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
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/url.hpp>
#include <thread>
#include <chrono>
#include <iostream>
#include <atomic>
#include <mutex>
#include <memory>
#include <queue>
#include <condition_variable>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;

namespace tradier {

class ConnectionManager {
private:
    std::atomic<bool> shouldStop_{false};
    std::atomic<bool> reconnecting_{false};
    std::mutex connectionMutex_;
    std::condition_variable connectionCv_;
    
public:
    ConnectionManager() = default;
    
    ~ConnectionManager() {
        stop();
    }
    
    void stop() {
        shouldStop_ = true;
        connectionCv_.notify_all();
    }
    
    bool shouldStop() const { return shouldStop_; }
    
    void setReconnecting(bool reconnecting) {
        reconnecting_ = reconnecting;
        if (!reconnecting) {
            connectionCv_.notify_all();
        }
    }
    
    bool isReconnecting() const { return reconnecting_; }
    
    void waitForReconnection(std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(connectionMutex_);
        connectionCv_.wait_for(lock, timeout, [this] { return !reconnecting_ || shouldStop_; });
    }
};

class MessageQueue {
private:
    std::queue<std::string> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> stopped_{false};
    size_t maxSize_;
    
public:
    explicit MessageQueue(size_t maxSize = 1000) : maxSize_(maxSize) {}
    
    void push(const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (stopped_ || queue_.size() >= maxSize_) {
            return;
        }
        queue_.push(message);
        cv_.notify_one();
    }
    
    bool pop(std::string& message, std::chrono::milliseconds timeout = std::chrono::milliseconds(100)) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        if (cv_.wait_for(lock, timeout, [this] { return !queue_.empty() || stopped_; })) {
            if (!queue_.empty()) {
                message = std::move(queue_.front());
                queue_.pop();
                return true;
            }
        }
        return false;
    }
    
    void stop() {
        std::lock_guard<std::mutex> lock(mutex_);
        stopped_ = true;
        cv_.notify_all();
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }
};

class WebSocketImpl {
private:
    std::string host_;
    std::string port_;
    std::string target_;
    std::string authToken_;
    bool useSSL_;
    
    net::io_context ioContext_;
    std::unique_ptr<tcp::resolver> resolver_;
    std::unique_ptr<websocket::stream<tcp::socket>> ws_;
    std::unique_ptr<websocket::stream<ssl::stream<tcp::socket>>> wss_;
    std::unique_ptr<ssl::context> sslContext_;
    
    std::atomic<bool> connected_{false};
    ConnectionManager connectionManager_;
    MessageQueue incomingMessages_;
    MessageQueue outgoingMessages_;
    
    MessageCallback messageCallback_;
    std::mutex callbackMutex_;
    
    std::thread ioThread_;
    std::thread messageProcessorThread_;
    std::thread messagePublisherThread_;
    
    beast::flat_buffer buffer_;
    std::mutex bufferMutex_;
    
public:
    WebSocketImpl(const std::string& url, const std::string& authToken)
        : authToken_(authToken), resolver_(std::make_unique<tcp::resolver>(ioContext_)) {
        
        parseUrl(url);
        
        if (useSSL_) {
            sslContext_ = std::make_unique<ssl::context>(ssl::context::tlsv12_client);
            sslContext_->set_default_verify_paths();
            sslContext_->set_verify_mode(ssl::verify_peer);
            
            wss_ = std::make_unique<websocket::stream<ssl::stream<tcp::socket>>>(ioContext_, *sslContext_);
            
            if (!SSL_set_tlsext_host_name(wss_->next_layer().native_handle(), host_.c_str())) {
                throw ConnectionError("Failed to set SNI hostname");
            }
        } else {
            ws_ = std::make_unique<websocket::stream<tcp::socket>>(ioContext_);
        }
    }
    
    ~WebSocketImpl() {
        disconnect();
    }
    
    WebSocketImpl(const WebSocketImpl&) = delete;
    WebSocketImpl& operator=(const WebSocketImpl&) = delete;
    
private:
    void parseUrl(const std::string& url) {
        try {
            auto result = boost::urls::parse_uri(url);
            if (!result) {
                throw ConnectionError("Invalid WebSocket URL: " + url);
            }
            
            auto parsed = *result;
            std::string scheme = parsed.scheme();
            
            if (scheme == "wss") {
                useSSL_ = true;
                port_ = parsed.port().empty() ? "443" : std::string(parsed.port());
            } else if (scheme == "ws") {
                useSSL_ = false;
                port_ = parsed.port().empty() ? "80" : std::string(parsed.port());
            } else {
                throw ConnectionError("Unsupported WebSocket scheme: " + scheme);
            }
            
            host_ = std::string(parsed.host());
            target_ = std::string(parsed.path());
            
            if (target_.empty()) {
                target_ = "/";
            }
            
            if (!parsed.query().empty()) {
                target_ += "?" + std::string(parsed.query());
            }
            
        } catch (const std::exception& e) {
            throw ConnectionError("URL parsing failed: " + std::string(e.what()));
        }
    }
    
    void setupWebSocketOptions() {
        websocket::stream_base::timeout opt{
            std::chrono::seconds(30),
            std::chrono::seconds(30),
            true
        };
        
        if (useSSL_) {
            wss_->set_option(opt);
            wss_->set_option(websocket::stream_base::decorator(
                [this](websocket::request_type& req) {
                    req.set(http::field::user_agent, "libtradier/0.1.0");
                    if (!authToken_.empty()) {
                        req.set(http::field::authorization, "Bearer " + authToken_);
                    }
                }));
        } else {
            ws_->set_option(opt);
            ws_->set_option(websocket::stream_base::decorator(
                [this](websocket::request_type& req) {
                    req.set(http::field::user_agent, "libtradier/0.1.0");
                    if (!authToken_.empty()) {
                        req.set(http::field::authorization, "Bearer " + authToken_);
                    }
                }));
        }
    }
    
    void startIOThread() {
        ioThread_ = std::thread([this]() {
            try {
                ioContext_.run();
            } catch (const std::exception& e) {
                handleError("IO thread error: " + std::string(e.what()));
            }
        });
    }
    
    void startMessageProcessor() {
        messageProcessorThread_ = std::thread([this]() {
            std::string message;
            while (!connectionManager_.shouldStop()) {
                if (incomingMessages_.pop(message, std::chrono::milliseconds(100))) {
                    try {
                        std::lock_guard<std::mutex> lock(callbackMutex_);
                        if (messageCallback_) {
                            messageCallback_(message);
                        }
                    } catch (const std::exception& e) {
                        handleError("Message processing error: " + std::string(e.what()));
                    }
                }
            }
        });
    }
    
    void startMessagePublisher() {
        messagePublisherThread_ = std::thread([this]() {
            std::string message;
            while (!connectionManager_.shouldStop()) {
                if (outgoingMessages_.pop(message, std::chrono::milliseconds(100))) {
                    try {
                        sendMessageInternal(message);
                    } catch (const std::exception& e) {
                        handleError("Message sending error: " + std::string(e.what()));
                    }
                }
            }
        });
    }
    
    void sendMessageInternal(const std::string& message) {
        if (!connected_) {
            throw ConnectionError("WebSocket not connected");
        }
        
        beast::error_code ec;
        
        if (useSSL_) {
            wss_->write(net::buffer(message), ec);
        } else {
            ws_->write(net::buffer(message), ec);
        }
        
        if (ec) {
            throw ConnectionError("WebSocket write failed: " + ec.message());
        }
    }
    
    void startReading() {
        if (useSSL_) {
            readMessageSSL();
        } else {
            readMessage();
        }
    }
    
    void readMessage() {
        ws_->async_read(buffer_, [this](beast::error_code ec, std::size_t bytes_transferred) {
            boost::ignore_unused(bytes_transferred);
            
            if (ec) {
                handleError("WebSocket read error: " + ec.message());
                return;
            }
            
            try {
                std::lock_guard<std::mutex> lock(bufferMutex_);
                std::string message = beast::buffers_to_string(buffer_.data());
                buffer_.consume(buffer_.size());
                
                incomingMessages_.push(message);
                
                if (connected_) {
                    readMessage();
                }
            } catch (const std::exception& e) {
                handleError("Message parsing error: " + std::string(e.what()));
            }
        });
    }
    
    void readMessageSSL() {
        wss_->async_read(buffer_, [this](beast::error_code ec, std::size_t bytes_transferred) {
            boost::ignore_unused(bytes_transferred);
            
            if (ec) {
                handleError("WebSocket SSL read error: " + ec.message());
                return;
            }
            
            try {
                std::lock_guard<std::mutex> lock(bufferMutex_);
                std::string message = beast::buffers_to_string(buffer_.data());
                buffer_.consume(buffer_.size());
                
                incomingMessages_.push(message);
                
                if (connected_) {
                    readMessageSSL();
                }
            } catch (const std::exception& e) {
                handleError("SSL message parsing error: " + std::string(e.what()));
            }
        });
    }
    
    void handleError(const std::string& error) {
        connected_ = false;
        
        std::lock_guard<std::mutex> lock(callbackMutex_);
        if (messageCallback_) {
            messageCallback_("{\"type\":\"error\",\"message\":\"" + error + "\"}");
        }
    }
    
public:
    void connect() {
        if (connected_) {
            return;
        }
        
        try {
            auto const results = resolver_->resolve(host_, port_);
            
            if (useSSL_) {
                connectSSL(results);
            } else {
                connectPlain(results);
            }
            
            setupWebSocketOptions();
            
            beast::error_code ec;
            
            if (useSSL_) {
                wss_->handshake(host_, target_, ec);
            } else {
                ws_->handshake(host_, target_, ec);
            }
            
            if (ec) {
                throw ConnectionError("WebSocket handshake failed: " + ec.message());
            }
            
            connected_ = true;
            
            startIOThread();
            startMessageProcessor();
            startMessagePublisher();
            startReading();
            
        } catch (const std::exception& e) {
            throw ConnectionError("WebSocket connection failed: " + std::string(e.what()));
        }
    }
    
private:
    void connectPlain(const tcp::resolver::results_type& results) {
        auto ep = net::connect(ws_->next_layer(), results);
        
        std::string host_port = host_ + ':' + std::to_string(ep.port());
        ws_->set_option(websocket::stream_base::decorator(
            [host_port, this](websocket::request_type& req) {
                req.set(http::field::host, host_port);
                req.set(http::field::user_agent, "libtradier/0.1.0");
                if (!authToken_.empty()) {
                    req.set(http::field::authorization, "Bearer " + authToken_);
                }
            }));
    }
    
    void connectSSL(const tcp::resolver::results_type& results) {
        auto ep = net::connect(wss_->next_layer().next_layer(), results);
        
        wss_->next_layer().handshake(ssl::stream_base::client);
        
        std::string host_port = host_ + ':' + std::to_string(ep.port());
        wss_->set_option(websocket::stream_base::decorator(
            [host_port, this](websocket::request_type& req) {
                req.set(http::field::host, host_port);
                req.set(http::field::user_agent, "libtradier/0.1.0");
                if (!authToken_.empty()) {
                    req.set(http::field::authorization, "Bearer " + authToken_);
                }
            }));
    }
    
public:
    void disconnect() {
        if (!connected_) {
            return;
        }
        
        connected_ = false;
        connectionManager_.stop();
        incomingMessages_.stop();
        outgoingMessages_.stop();
        
        try {
            beast::error_code ec;
            
            if (useSSL_ && wss_) {
                wss_->close(websocket::close_code::normal, ec);
            } else if (ws_) {
                ws_->close(websocket::close_code::normal, ec);
            }
        } catch (const std::exception&) {
            // Ignore close errors
        }
        
        ioContext_.stop();
        
        if (ioThread_.joinable()) {
            ioThread_.join();
        }
        if (messageProcessorThread_.joinable()) {
            messageProcessorThread_.join();
        }
        if (messagePublisherThread_.joinable()) {
            messagePublisherThread_.join();
        }
    }
    
    void send(const std::string& message) {
        if (!connected_) {
            throw ConnectionError("WebSocket not connected");
        }
        
        outgoingMessages_.push(message);
    }
    
    void setMessageHandler(MessageCallback callback) {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        messageCallback_ = std::move(callback);
    }
    
    void setAuthToken(const std::string& token) {
        authToken_ = token;
    }
    
    bool isConnected() const {
        return connected_;
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
