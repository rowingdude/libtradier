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

#include "tradier/watchlist.hpp"
#include "tradier/client.hpp"
#include "tradier/common/errors.hpp"
#include "tradier/common/json_utils.hpp"
#include <sstream>

namespace tradier {

Result<std::vector<WatchlistSummary>> WatchlistService::getWatchlists() {
    return tryExecute<std::vector<WatchlistSummary>>([&]() -> std::vector<WatchlistSummary> {
        auto response = client_.get("/watchlists");
        
        if (!response.success()) {
            throw ApiError(response.status, "Failed to get watchlists: " + response.body);
        }
        
        auto parsed = json::parseResponse<std::vector<WatchlistSummary>>(response, [](const auto& json) {
            std::vector<WatchlistSummary> watchlists;
            
            if (json.contains("watchlists") && json["watchlists"].contains("watchlist")) {
                const auto& watchlistsJson = json["watchlists"]["watchlist"];
                
                if (watchlistsJson.is_array()) {
                    for (const auto& item : watchlistsJson) {
                        WatchlistSummary summary;
                        summary.name = item.value("name", "");
                        summary.id = item.value("id", "");
                        summary.publicId = item.value("public_id", "");
                        watchlists.push_back(summary);
                    }
                } else if (watchlistsJson.is_object()) {
                    WatchlistSummary summary;
                    summary.name = watchlistsJson.value("name", "");
                    summary.id = watchlistsJson.value("id", "");
                    summary.publicId = watchlistsJson.value("public_id", "");
                    watchlists.push_back(summary);
                }
            }
            
            return watchlists;
        });
        
        if (!parsed) {
            throw std::runtime_error("Failed to parse watchlists response");
        }
        
        return *parsed;
    }, "getWatchlists");
}

Result<Watchlist> WatchlistService::getWatchlist(const std::string& watchlistId) {
    return tryExecute<Watchlist>([&]() -> Watchlist {
        if (watchlistId.empty()) {
            throw ValidationError("Watchlist ID cannot be empty");
        }
        
        auto response = client_.get("/watchlists/" + watchlistId);
        
        if (!response.success()) {
            throw ApiError(response.status, "Failed to get watchlist: " + response.body);
        }
        
        auto parsed = json::parseResponse<Watchlist>(response, [](const auto& json) {
            if (!json.contains("watchlist")) {
                throw ApiError(400, "Invalid watchlist response format");
            }
            
            const auto& watchlistJson = json["watchlist"];
            Watchlist watchlist;
            watchlist.name = watchlistJson.value("name", "");
            watchlist.id = watchlistJson.value("id", "");
            watchlist.publicId = watchlistJson.value("public_id", "");
            
            if (watchlistJson.contains("items") && watchlistJson["items"].contains("item")) {
                const auto& itemsJson = watchlistJson["items"]["item"];
                
                if (itemsJson.is_array()) {
                    for (const auto& item : itemsJson) {
                        WatchlistItem watchlistItem;
                        watchlistItem.symbol = item.value("symbol", "");
                        watchlistItem.id = item.value("id", "");
                        watchlist.items.push_back(watchlistItem);
                    }
                } else if (itemsJson.is_object()) {
                    WatchlistItem watchlistItem;
                    watchlistItem.symbol = itemsJson.value("symbol", "");
                    watchlistItem.id = itemsJson.value("id", "");
                    watchlist.items.push_back(watchlistItem);
                }
            }
            
            return watchlist;
        });
        
        if (!parsed) {
            throw std::runtime_error("Failed to parse watchlist response");
        }
        
        return *parsed;
    }, "getWatchlist");
}

Result<Watchlist> WatchlistService::createWatchlist(const std::string& name, const std::vector<std::string>& symbols) {
    return tryExecute<Watchlist>([&]() -> Watchlist {
        if (name.empty()) {
            throw ValidationError("Watchlist name cannot be empty");
        }
        
        FormParams params;
        params["name"] = name;
        
        if (!symbols.empty()) {
            std::ostringstream symbolsStr;
            for (size_t i = 0; i < symbols.size(); ++i) {
                if (i > 0) symbolsStr << ",";
                symbolsStr << symbols[i];
            }
            params["symbols"] = symbolsStr.str();
        }
        
        auto response = client_.post("/watchlists", params);
        
        if (!response.success()) {
            throw ApiError(response.status, "Failed to create watchlist: " + response.body);
        }
        
        auto parsed = json::parseResponse<Watchlist>(response, [](const auto& json) {
            if (!json.contains("watchlist")) {
                throw ApiError(400, "Invalid watchlist creation response format");
            }
            
            const auto& watchlistJson = json["watchlist"];
            Watchlist watchlist;
            watchlist.name = watchlistJson.value("name", "");
            watchlist.id = watchlistJson.value("id", "");
            watchlist.publicId = watchlistJson.value("public_id", "");
            
            if (watchlistJson.contains("items") && watchlistJson["items"].contains("item")) {
                const auto& itemsJson = watchlistJson["items"]["item"];
                
                if (itemsJson.is_array()) {
                    for (const auto& item : itemsJson) {
                        WatchlistItem watchlistItem;
                        watchlistItem.symbol = item.value("symbol", "");
                        watchlistItem.id = item.value("id", "");
                        watchlist.items.push_back(watchlistItem);
                    }
                } else if (itemsJson.is_object()) {
                    WatchlistItem watchlistItem;
                    watchlistItem.symbol = itemsJson.value("symbol", "");
                    watchlistItem.id = itemsJson.value("id", "");
                    watchlist.items.push_back(watchlistItem);
                }
            }
            
            return watchlist;
        });
        
        if (!parsed) {
            throw std::runtime_error("Failed to parse create watchlist response");
        }
        
        return *parsed;
    }, "createWatchlist");
}

Result<Watchlist> WatchlistService::updateWatchlist(const std::string& watchlistId, const std::string& name, const std::vector<std::string>& symbols) {
    return tryExecute<Watchlist>([&]() -> Watchlist {
        if (watchlistId.empty()) {
            throw ValidationError("Watchlist ID cannot be empty");
        }
        if (name.empty()) {
            throw ValidationError("Watchlist name cannot be empty");
        }
        
        FormParams params;
        params["name"] = name;
        
        if (!symbols.empty()) {
            std::ostringstream symbolsStr;
            for (size_t i = 0; i < symbols.size(); ++i) {
                if (i > 0) symbolsStr << ",";
                symbolsStr << symbols[i];
            }
            params["symbols"] = symbolsStr.str();
        }
        
        auto response = client_.put("/watchlists/" + watchlistId, params);
        
        if (!response.success()) {
            throw ApiError(response.status, "Failed to update watchlist: " + response.body);
        }
        
        auto parsed = json::parseResponse<Watchlist>(response, [](const auto& json) {
            if (!json.contains("watchlist")) {
                throw ApiError(400, "Invalid watchlist update response format");
            }
            
            const auto& watchlistJson = json["watchlist"];
            Watchlist watchlist;
            watchlist.name = watchlistJson.value("name", "");
            watchlist.id = watchlistJson.value("id", "");
            watchlist.publicId = watchlistJson.value("public_id", "");
            
            if (watchlistJson.contains("items") && watchlistJson["items"].contains("item")) {
                const auto& itemsJson = watchlistJson["items"]["item"];
                
                if (itemsJson.is_array()) {
                    for (const auto& item : itemsJson) {
                        WatchlistItem watchlistItem;
                        watchlistItem.symbol = item.value("symbol", "");
                        watchlistItem.id = item.value("id", "");
                        watchlist.items.push_back(watchlistItem);
                    }
                } else if (itemsJson.is_object()) {
                    WatchlistItem watchlistItem;
                    watchlistItem.symbol = itemsJson.value("symbol", "");
                    watchlistItem.id = itemsJson.value("id", "");
                    watchlist.items.push_back(watchlistItem);
                }
            }
            
            return watchlist;
        });
        
        if (!parsed) {
            throw std::runtime_error("Failed to parse update watchlist response");
        }
        
        return *parsed;
    }, "updateWatchlist");
}

Result<std::vector<WatchlistSummary>> WatchlistService::deleteWatchlist(const std::string& watchlistId) {
    return tryExecute<std::vector<WatchlistSummary>>([&]() -> std::vector<WatchlistSummary> {
        if (watchlistId.empty()) {
            throw ValidationError("Watchlist ID cannot be empty");
        }
        
        auto response = client_.del("/watchlists/" + watchlistId);
        
        if (!response.success()) {
            throw ApiError(response.status, "Failed to delete watchlist: " + response.body);
        }
        
        auto parsed = json::parseResponse<std::vector<WatchlistSummary>>(response, [](const auto& json) {
            std::vector<WatchlistSummary> watchlists;
            
            if (json.contains("watchlists") && json["watchlists"].contains("watchlist")) {
                const auto& watchlistsJson = json["watchlists"]["watchlist"];
                
                if (watchlistsJson.is_array()) {
                    for (const auto& item : watchlistsJson) {
                        WatchlistSummary summary;
                        summary.name = item.value("name", "");
                        summary.id = item.value("id", "");
                        summary.publicId = item.value("public_id", "");
                        watchlists.push_back(summary);
                    }
                } else if (watchlistsJson.is_object()) {
                    WatchlistSummary summary;
                    summary.name = watchlistsJson.value("name", "");
                    summary.id = watchlistsJson.value("id", "");
                    summary.publicId = watchlistsJson.value("public_id", "");
                    watchlists.push_back(summary);
                }
            }
            
            return watchlists;
        });
        
        if (!parsed) {
            throw std::runtime_error("Failed to parse delete watchlist response");
        }
        
        return *parsed;
    }, "deleteWatchlist");
}

Result<Watchlist> WatchlistService::addSymbols(const std::string& watchlistId, const std::vector<std::string>& symbols) {
    return tryExecute<Watchlist>([&]() -> Watchlist {
        if (watchlistId.empty()) {
            throw ValidationError("Watchlist ID cannot be empty");
        }
        if (symbols.empty()) {
            throw ValidationError("Symbols list cannot be empty");
        }
        
        FormParams params;
        std::ostringstream symbolsStr;
        for (size_t i = 0; i < symbols.size(); ++i) {
            if (i > 0) symbolsStr << ",";
            symbolsStr << symbols[i];
        }
        params["symbols"] = symbolsStr.str();
        
        auto response = client_.post("/watchlists/" + watchlistId + "/symbols", params);
        
        if (!response.success()) {
            throw ApiError(response.status, "Failed to add symbols to watchlist: " + response.body);
        }
        
        auto parsed = json::parseResponse<Watchlist>(response, [](const auto& json) {
            if (!json.contains("watchlist")) {
                throw ApiError(400, "Invalid add symbols response format");
            }
            
            const auto& watchlistJson = json["watchlist"];
            Watchlist watchlist;
            watchlist.name = watchlistJson.value("name", "");
            watchlist.id = watchlistJson.value("id", "");
            watchlist.publicId = watchlistJson.value("public_id", "");
            
            if (watchlistJson.contains("items") && watchlistJson["items"].contains("item")) {
                const auto& itemsJson = watchlistJson["items"]["item"];
                
                if (itemsJson.is_array()) {
                    for (const auto& item : itemsJson) {
                        WatchlistItem watchlistItem;
                        watchlistItem.symbol = item.value("symbol", "");
                        watchlistItem.id = item.value("id", "");
                        watchlist.items.push_back(watchlistItem);
                    }
                } else if (itemsJson.is_object()) {
                    WatchlistItem watchlistItem;
                    watchlistItem.symbol = itemsJson.value("symbol", "");
                    watchlistItem.id = itemsJson.value("id", "");
                    watchlist.items.push_back(watchlistItem);
                }
            }
            
            return watchlist;
        });
        
        if (!parsed) {
            throw std::runtime_error("Failed to parse add symbols response");
        }
        
        return *parsed;
    }, "addSymbols");
}

Result<Watchlist> WatchlistService::removeSymbol(const std::string& watchlistId, const std::string& symbol) {
    return tryExecute<Watchlist>([&]() -> Watchlist {
        if (watchlistId.empty()) {
            throw ValidationError("Watchlist ID cannot be empty");
        }
        if (symbol.empty()) {
            throw ValidationError("Symbol cannot be empty");
        }
        
        auto response = client_.del("/watchlists/" + watchlistId + "/symbols/" + symbol);
        
        if (!response.success()) {
            throw ApiError(response.status, "Failed to remove symbol from watchlist: " + response.body);
        }
        
        auto parsed = json::parseResponse<Watchlist>(response, [](const auto& json) {
            if (!json.contains("watchlist")) {
                throw ApiError(400, "Invalid remove symbol response format");
            }
            
            const auto& watchlistJson = json["watchlist"];
            Watchlist watchlist;
            watchlist.name = watchlistJson.value("name", "");
            watchlist.id = watchlistJson.value("id", "");
            watchlist.publicId = watchlistJson.value("public_id", "");
            
            if (watchlistJson.contains("items") && watchlistJson["items"].contains("item")) {
                const auto& itemsJson = watchlistJson["items"]["item"];
                
                if (itemsJson.is_array()) {
                    for (const auto& item : itemsJson) {
                        WatchlistItem watchlistItem;
                        watchlistItem.symbol = item.value("symbol", "");
                        watchlistItem.id = item.value("id", "");
                        watchlist.items.push_back(watchlistItem);
                    }
                } else if (itemsJson.is_object()) {
                    WatchlistItem watchlistItem;
                    watchlistItem.symbol = itemsJson.value("symbol", "");
                    watchlistItem.id = itemsJson.value("id", "");
                    watchlist.items.push_back(watchlistItem);
                }
            }
            
            return watchlist;
        });
        
        if (!parsed) {
            throw std::runtime_error("Failed to parse remove symbol response");
        }
        
        return *parsed;
    }, "removeSymbol");
}

}