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
#include <chrono>
#include <map>
#include "tradier/common/types.hpp"

namespace tradier {

class TradierClient;

struct AuthToken {
    std::string accessToken;
    std::string refreshToken;
    std::chrono::system_clock::time_point expiresAt;
    std::chrono::system_clock::time_point issuedAt;
};

class AuthService {
private:
    TradierClient& client_;
    std::map<std::string, std::chrono::system_clock::time_point> state_cache_;
    std::chrono::seconds state_cache_duration_;
    
    void saveStateWithTimestamp(const std::string& state);
    void cleanExpiredStates();
    std::string generateRandomState(size_t length = 32) const;
    std::string urlEncode(const std::string& value) const;
    
public:
    explicit AuthService(TradierClient& client);
    
    std::pair<std::string, std::string> getAuthorizationUrl(
        const std::string& client_id, 
        const std::string& redirect_uri = "",
        const std::string& scope = "read,write,market,trade,stream");
    
    bool validateState(const std::string& state);
    
    AuthToken exchangeAuthCode(
        const std::string& auth_code,
        const std::string& client_id,
        const std::string& client_secret);
    
    AuthToken refreshToken(
        const std::string& refresh_token,
        const std::string& client_id,
        const std::string& client_secret);
    
    bool validateToken(const std::string& access_token);
    
    bool isTokenExpired(const AuthToken& token) const;
    
    std::string generateBasicAuthHeader(
        const std::string& client_id, 
        const std::string& client_secret) const;
};

} // namespace tradier