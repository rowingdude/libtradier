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

#include <string>
#include <string_view>
#include <functional>
#include <memory>
#include "tradier/common/types.hpp"
#include "tradier/common/config.hpp"

namespace tradier {

class WebSocketImpl;

using MessageCallback = std::function<void(const std::string&)>;

class WebSocketConnection {
private:
    std::unique_ptr<WebSocketImpl> impl_;
    
public:
    explicit WebSocketConnection(std::unique_ptr<WebSocketImpl> impl);
    ~WebSocketConnection();
    
    WebSocketConnection(const WebSocketConnection&) = delete;
    WebSocketConnection& operator=(const WebSocketConnection&) = delete;
    
    WebSocketConnection(WebSocketConnection&&) noexcept;
    WebSocketConnection& operator=(WebSocketConnection&&) noexcept;
    
    void connect();
    void disconnect();
    void send(const std::string& message);
    void setMessageHandler(MessageCallback callback);
    void setAuthToken(const std::string& token);
    bool isConnected() const; 
};

class WebSocketClient {
private:
    Config config_;
    
public:
    explicit WebSocketClient(const Config& config);
    ~WebSocketClient();
    
    WebSocketClient(const WebSocketClient&) = delete;
    WebSocketClient& operator=(const WebSocketClient&) = delete;
    
    WebSocketClient(WebSocketClient&&) noexcept;
    WebSocketClient& operator=(WebSocketClient&&) noexcept;
    
    WebSocketConnection connect(std::string_view endpoint, std::string_view auth_token);
};

} // namespace tradier