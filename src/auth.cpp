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

#include "tradier/auth.hpp"
#include "tradier/client.hpp"
#include "tradier/common/errors.hpp"
#include <nlohmann/json.hpp>
#include <random>
#include <sstream>
#include <iomanip>
#include <curl/curl.h>

namespace tradier {

AuthService::AuthService(TradierClient& client) 
    : client_(client), 
      state_cache_duration_(std::chrono::minutes(10)) {}

std::pair<std::string, std::string> AuthService::getAuthorizationUrl(
    const std::string& client_id, 
    const std::string& redirect_uri,
    const std::string& scope) {
    
    std::string state = generateRandomState();
    saveStateWithTimestamp(state);
    std::stringstream urlStream;
    std::string baseUrl = client_.isSandboxMode() ? 
        "https://sandbox.tradier.com" : "https://api.tradier.com";
    
    urlStream << baseUrl << "/v1/oauth/authorize"
              << "?client_id=" << urlEncode(client_id)
              << "&scope=" << urlEncode(scope)
              << "&state=" << urlEncode(state);
    
    if (!redirect_uri.empty()) {
        urlStream << "&redirect_uri=" << urlEncode(redirect_uri);
    }
    
    return {urlStream.str(), state};
}

bool AuthService::validateState(const std::string& state) {
    cleanExpiredStates();
    
    auto it = state_cache_.find(state);
    if (it != state_cache_.end()) {
        state_cache_.erase(it);
        return true;
    }
    
    return false;
}

AuthToken AuthService::exchangeAuthCode(
    const std::string& auth_code,
    const std::string& client_id,
    const std::string& client_secret) {
    
    if (auth_code.empty()) {
        throw AuthenticationError("Authorization code cannot be empty");
    }
    
    if (client_id.empty() || client_secret.empty()) {
        throw AuthenticationError("Client ID and client secret are required");
    }
    
    FormParams params = {
        {"grant_type", "authorization_code"},
        {"code", auth_code}
    };
    
    Headers headers = {
        {"Authorization", generateBasicAuthHeader(client_id, client_secret)},
        {"Content-Type", "application/x-www-form-urlencoded"},
        {"Accept", "application/json"}
    };
    
    Response response = client_.post("/v1/oauth/accesstoken", params, headers);
    
    if (response.status != 200) {
        throw AuthenticationError("Failed to exchange authorization code: " + response.body);
    }
    
    auto json = nlohmann::json::parse(response.body);
    
    AuthToken token;
    token.accessToken = json["access_token"];
    
    if (json.contains("refresh_token")) {
        token.refreshToken = json["refresh_token"];
    }
    
    if (json.contains("expires_in")) {
        int expires_in = json["expires_in"];
        token.expiresAt = std::chrono::system_clock::now() + std::chrono::seconds(expires_in);
    }
    
    token.issuedAt = std::chrono::system_clock::now();
    
    client_.setAccessToken(token.accessToken);
    
    return token;
}

AuthToken AuthService::refreshToken(
    const std::string& refresh_token,
    const std::string& client_id,
    const std::string& client_secret) {
    
    if (refresh_token.empty()) {
        throw AuthenticationError("Refresh token cannot be empty");
    }
    
    if (client_id.empty() || client_secret.empty()) {
        throw AuthenticationError("Client ID and client secret are required");
    }
    
    FormParams params = {
        {"grant_type", "refresh_token"},
        {"refresh_token", refresh_token}
    };
    
    Headers headers = {
        {"Authorization", generateBasicAuthHeader(client_id, client_secret)},
        {"Content-Type", "application/x-www-form-urlencoded"},
        {"Accept", "application/json"}
    };
    
    Response response = client_.post("/v1/oauth/refreshtoken", params, headers);
    
    if (response.status != 200) {
        throw AuthenticationError("Failed to refresh token: " + response.body);
    }
    
    auto json = nlohmann::json::parse(response.body);
    
    AuthToken token;
    token.accessToken = json["access_token"];
    
    if (json.contains("refresh_token")) {
        token.refreshToken = json["refresh_token"];
    }
    
    if (json.contains("expires_in")) {
        int expires_in = json["expires_in"];
        token.expiresAt = std::chrono::system_clock::now() + std::chrono::seconds(expires_in);
    }
    
    token.issuedAt = std::chrono::system_clock::now();
    
    client_.setAccessToken(token.accessToken);
    
    return token;
}

bool AuthService::validateToken(const std::string& access_token) {
    if (access_token.empty()) {
        return false;
    }
    
    std::string current_token = client_.getAccessToken();
    
    try {
        client_.setAccessToken(access_token);
        Response response = client_.get("/v1/user/profile");
        client_.setAccessToken(current_token);
        return response.status == 200;
    } catch (const std::exception&) {
        client_.setAccessToken(current_token);
        return false;
    }
}

bool AuthService::isTokenExpired(const AuthToken& token) const {
    if (token.expiresAt.time_since_epoch().count() == 0) {
        return true;
    }
    
    auto now = std::chrono::system_clock::now();
    auto buffer = std::chrono::minutes(5);
    
    return (now + buffer) > token.expiresAt;
}

std::string AuthService::generateBasicAuthHeader(
    const std::string& client_id, 
    const std::string& client_secret) const {
    
    std::string credentials = client_id + ":" + client_secret;
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw ConnectionError("Failed to initialize CURL for encoding");
    }
    
    char* output = curl_easy_escape(curl, credentials.c_str(), static_cast<int>(credentials.length()));
    if (!output) {
        curl_easy_cleanup(curl);
        throw ConnectionError("Failed to encode credentials");
    }
    
    std::string encoded(output);
    curl_free(output);
    curl_easy_cleanup(curl);
    
    return "Basic " + encoded;
}

std::string AuthService::generateRandomState(size_t length) const {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, sizeof(alphanum) - 2);
    
    std::string state;
    state.reserve(length);
    
    for (size_t i = 0; i < length; ++i) {
        state += alphanum[distribution(generator)];
    }
    
    return state;
}

void AuthService::saveStateWithTimestamp(const std::string& state) {
    auto now = std::chrono::system_clock::now();
    state_cache_[state] = now;
}

void AuthService::cleanExpiredStates() {
    auto now = std::chrono::system_clock::now();
    
    auto it = state_cache_.begin();
    while (it != state_cache_.end()) {
        if ((now - it->second) > state_cache_duration_) {
            it = state_cache_.erase(it);
        } else {
            ++it;
        }
    }
}

std::string AuthService::urlEncode(const std::string& value) const {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw ConnectionError("Failed to initialize CURL for URL encoding");
    }
    
    char* encoded = curl_easy_escape(curl, value.c_str(), static_cast<int>(value.length()));
    if (!encoded) {
        curl_easy_cleanup(curl);
        throw ConnectionError("Failed to URL encode value");
    }
    
    std::string result(encoded);
    curl_free(encoded);
    curl_easy_cleanup(curl);
    
    return result;
}

} // namespace tradier