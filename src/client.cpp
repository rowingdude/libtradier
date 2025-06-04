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


#include "tradier/client.hpp"
#include "tradier/account.hpp"
#include "tradier/trading.hpp"
#include "tradier/streaming.hpp"
#include "tradier/market.hpp"
#include "tradier/watchlist.hpp"
#include "tradier/common/http_client.hpp"
#include "tradier/common/errors.hpp"

namespace tradier {
TradierClient::TradierClient(const Config& config) 
    : config_(config), httpClient_(std::make_unique<HttpClient>(config)) {
    if (config_.accessToken.empty()) {
        throw AuthenticationError("Access token required");
    }
}

TradierClient::~TradierClient() = default;
TradierClient::TradierClient(TradierClient&&) noexcept = default;
TradierClient& TradierClient::operator=(TradierClient&&) noexcept = default;

Response TradierClient::get(const std::string& endpoint, const QueryParams& params) {
    return httpClient_->get(endpoint, params);
}

Response TradierClient::post(const std::string& endpoint, const FormParams& params) {
    return httpClient_->post(endpoint, params);
}

Response TradierClient::put(const std::string& endpoint, const FormParams& params) {
    return httpClient_->put(endpoint, params);
}

Response TradierClient::del(const std::string& endpoint, const QueryParams& params) {
    return httpClient_->del(endpoint, params);
}

AccountService TradierClient::accounts() {
    return AccountService(*this);
}

TradingService TradierClient::trading() {
    return TradingService(*this);
}

StreamingService TradierClient::streaming() {
    return StreamingService(*this);
}

MarketService TradierClient::market() {
    return MarketService(*this);
}

WatchlistService TradierClient::watchlists() {
    return WatchlistService(*this);
}

void TradierClient::setRateLimit(int maxRequestsPerWindow, std::chrono::milliseconds windowDuration) {
    httpClient_->setRateLimit(maxRequestsPerWindow, windowDuration);
}

void TradierClient::enableRateLimit(bool enabled) {
    httpClient_->enableRateLimit(enabled);
}

void TradierClient::setRetryPolicy(int maxRetries, std::chrono::milliseconds initialDelay, double backoffMultiplier) {
    httpClient_->setRetryPolicy(maxRetries, initialDelay, backoffMultiplier);
}

void TradierClient::enableRetries(bool enabled) {
    httpClient_->enableRetries(enabled);
}

}
