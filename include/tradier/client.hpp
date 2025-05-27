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

#include <memory>
#include "tradier/common/types.hpp"
#include "tradier/common/config.hpp"

namespace tradier {

class HttpClient;
class AccountService;
class TradingService;
class StreamingService;
class MarketService;
class WatchlistService;

class TradierClient {
private:
    Config config_;
    std::unique_ptr<HttpClient> httpClient_;
    
public:
    explicit TradierClient(const Config& config);
    ~TradierClient();
    
    TradierClient(TradierClient&&) noexcept;
    TradierClient& operator=(TradierClient&&) noexcept;
    TradierClient(const TradierClient&) = delete;
    TradierClient& operator=(const TradierClient&) = delete;
    
    static constexpr const char* version() { return "0.1.0"; }
    
    const Config& config() const { return config_; }
    bool isAuthenticated() const { return !config_.accessToken.empty(); }
    
    Response get(const std::string& endpoint, const QueryParams& params = {});
    Response post(const std::string& endpoint, const FormParams& params = {});
    Response put(const std::string& endpoint, const FormParams& params = {});
    Response del(const std::string& endpoint, const QueryParams& params = {});
    
    WatchlistService watchlists();
    AccountService accounts();
    TradingService trading();
    StreamingService streaming();
    MarketService market();
};

}