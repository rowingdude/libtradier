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

#include <vector>
#include <string>
#include <optional>
#include "tradier/common/types.hpp"

namespace tradier {

class TradierClient;

struct WatchlistItem {
    std::string symbol;
    std::string id;
};

struct Watchlist {
    std::string name;
    std::string id;
    std::string publicId;
    std::vector<WatchlistItem> items;
};

struct WatchlistSummary {
    std::string name;
    std::string id;
    std::string publicId;
};

class WatchlistService {
private:
    TradierClient& client_;
    
public:
    explicit WatchlistService(TradierClient& client) : client_(client) {}
    
    Result<std::vector<WatchlistSummary>> getWatchlists();
    Result<Watchlist> getWatchlist(const std::string& watchlistId);
    Result<Watchlist> createWatchlist(const std::string& name, const std::vector<std::string>& symbols = {});
    Result<Watchlist> updateWatchlist(const std::string& watchlistId, const std::string& name, const std::vector<std::string>& symbols = {});
    Result<std::vector<WatchlistSummary>> deleteWatchlist(const std::string& watchlistId);
    Result<Watchlist> addSymbols(const std::string& watchlistId, const std::vector<std::string>& symbols);
    Result<Watchlist> removeSymbol(const std::string& watchlistId, const std::string& symbol);
};

}