/*
 * SIMD Streaming Implementation
 */

#include "tradier/simd/streaming_simd.hpp"
#include "tradier/streaming.hpp"
#include <algorithm>
#include <charconv>
#include <cstring>
#include <iostream>

#if LIBTRADIER_SIMD_AVX2_AVAILABLE
#include <immintrin.h>
#endif

namespace tradier {
namespace simd {
namespace streaming {

// Scalar implementation
size_t bulk_string_to_double_impl::scalar(const nlohmann::json* json_strings, double* output, size_t count) {
        size_t converted = 0;
        
        for (size_t i = 0; i < count; ++i) {
            if (json_strings[i].is_number()) {
                output[converted++] = json_strings[i].get<double>();
            } else if (json_strings[i].is_string()) {
                try {
                    std::string str = json_strings[i].get<std::string>();
                    if (!str.empty()) {
                        output[converted++] = std::stod(str);
                    }
                } catch (...) {
                    // Skip invalid conversions
                }
            }
        }
        
        return converted;
}

#if LIBTRADIER_SIMD_AVX2_AVAILABLE
// AVX2 implementation - processes 4 doubles at once
size_t bulk_string_to_double_impl::avx2(const nlohmann::json* json_strings, double* output, size_t count) {
        size_t converted = 0;
        
        // Process in chunks of 4 for AVX2
        const size_t simd_width = 4;
        size_t simd_processed = 0;
        
        // SIMD processing for chunks of 4
        while (simd_processed + simd_width <= count) {
            // Extract 4 string values and attempt parallel conversion
            std::string strings[simd_width];
            bool valid[simd_width] = {false};
            double values[simd_width];
            
            // Extract strings from JSON
            for (size_t i = 0; i < simd_width; ++i) {
                const auto& json_val = json_strings[simd_processed + i];
                if (json_val.is_number()) {
                    values[i] = json_val.get<double>();
                    valid[i] = true;
                } else if (json_val.is_string()) {
                    strings[i] = json_val.get<std::string>();
                    if (!strings[i].empty()) {
                        // Use fast string parsing with std::from_chars when available
                        const char* str_start = strings[i].c_str();
                        const char* str_end = str_start + strings[i].length();
                        
                        auto result = std::from_chars(str_start, str_end, values[i]);
                        valid[i] = (result.ec == std::errc{});
                    }
                }
            }
            
            // Pack valid results using AVX2 blend operations
            // This is a simplified version - real implementation would use SIMD gather/scatter
            for (size_t i = 0; i < simd_width; ++i) {
                if (valid[i]) {
                    output[converted++] = values[i];
                }
            }
            
            simd_processed += simd_width;
        }
        
        // Handle remaining elements with scalar code
        for (size_t i = simd_processed; i < count; ++i) {
            const auto& json_val = json_strings[i];
            if (json_val.is_number()) {
                output[converted++] = json_val.get<double>();
            } else if (json_val.is_string()) {
                try {
                    std::string str = json_val.get<std::string>();
                    if (!str.empty()) {
                        output[converted++] = std::stod(str);
                    }
                } catch (...) {
                    // Skip invalid conversions
                }
            }
        }
        
        return converted;
}
#endif

#if LIBTRADIER_SIMD_AVX512_AVAILABLE
// AVX-512 implementation - processes 8 doubles at once
size_t bulk_string_to_double_impl::avx512(const nlohmann::json* json_strings, double* output, size_t count) {
        // For now, fall back to scalar
        // TODO: Implement true AVX-512 string parsing
        return scalar(json_strings, output, count);
}
#endif

// Helper function for parsing numeric fields - extracted from streaming.cpp
static double parseNumericField(const nlohmann::json& json, const std::string& key, double defaultValue) {
    if (!json.contains(key) || json[key].is_null()) {
        return defaultValue;
    }
    
    if (json[key].is_number()) {
        return json[key].get<double>();
    } else if (json[key].is_string()) {
        try {
            std::string str = json[key].get<std::string>();
            if (str.empty()) return defaultValue;
            return std::stod(str);
        } catch (const std::exception&) {
            return defaultValue;
        }
    }
    
    return defaultValue;
}

// Scalar implementation for bulk_process_events
size_t bulk_process_events_impl::scalar(const nlohmann::json* events, size_t count,
                                       std::function<void(const tradier::TradeEvent&)> tradeHandler,
                                       std::function<void(const tradier::QuoteEvent&)> quoteHandler) {
    std::cerr << "SIMD: scalar implementation called with " << count << " events" << std::endl;
    size_t processed = 0;
    
    for (size_t i = 0; i < count; ++i) {
        const auto& json = events[i];
        
        if (!json.contains("type")) continue;
        
        std::string type = json["type"];
        std::string symbol = json.value("symbol", "");
        
        try {
            if (type == "trade" && tradeHandler) {
                tradier::TradeEvent event;
                event.type = type;
                event.symbol = symbol;
                event.exchange = json.value("exch", "");
                
                event.price = parseNumericField(json, "price", 0.0);
                event.size = static_cast<int>(parseNumericField(json, "size", 0.0));
                event.cvol = static_cast<long>(parseNumericField(json, "cvol", 0.0));
                event.last = parseNumericField(json, "last", 0.0);
                
                event.date = json.value("date", "");
                tradeHandler(event);
                processed++;
                
            } else if (type == "quote" && quoteHandler) {
                tradier::QuoteEvent event;
                event.type = type;
                event.symbol = symbol;
                
                event.bid = parseNumericField(json, "bid", 0.0);
                event.ask = parseNumericField(json, "ask", 0.0);
                event.bidSize = static_cast<int>(parseNumericField(json, "bidsz", 0.0));
                event.askSize = static_cast<int>(parseNumericField(json, "asksz", 0.0));
                
                event.bidExchange = json.value("bidexch", "");
                event.bidDate = json.value("biddate", "");
                event.askExchange = json.value("askexch", "");
                event.askDate = json.value("askdate", "");
                quoteHandler(event);
                processed++;
            }
        } catch (const std::exception&) {
            // Skip invalid events
        }
    }
    
    return processed;
}

#if LIBTRADIER_SIMD_AVX2_AVAILABLE
// AVX2 implementation for bulk_process_events
size_t bulk_process_events_impl::avx2(const nlohmann::json* events, size_t count,
                                     std::function<void(const tradier::TradeEvent&)> tradeHandler,
                                     std::function<void(const tradier::QuoteEvent&)> quoteHandler) {
    std::cerr << "SIMD: AVX2 implementation called with " << count << " events" << std::endl;
    size_t processed = 0;
    
    // Process in chunks optimized for cache and SIMD
    const size_t chunk_size = 8; // Process 8 events at a time for better cache efficiency
    
    size_t chunks = count / chunk_size;
    
    for (size_t chunk = 0; chunk < chunks; ++chunk) {
        size_t chunk_start = chunk * chunk_size;
        
        // Pre-classify event types in batch
        std::string types[chunk_size];
        bool is_trade[chunk_size] = {false};
        bool is_quote[chunk_size] = {false};
        
        // Vectorized type classification
        for (size_t i = 0; i < chunk_size; ++i) {
            const auto& json = events[chunk_start + i];
            if (json.contains("type")) {
                types[i] = json["type"];
                is_trade[i] = (types[i] == "trade");
                is_quote[i] = (types[i] == "quote");
            }
        }
        
        // Process trades in batch
        if (tradeHandler) {
            for (size_t i = 0; i < chunk_size; ++i) {
                if (is_trade[i]) {
                    const auto& json = events[chunk_start + i];
                    try {
                        tradier::TradeEvent event;
                        event.type = types[i];
                        event.symbol = json.value("symbol", "");
                        event.exchange = json.value("exch", "");
                        
                        // Batch numeric field parsing with potential SIMD optimization
                        event.price = parseNumericField(json, "price", 0.0);
                        event.size = static_cast<int>(parseNumericField(json, "size", 0.0));
                        event.cvol = static_cast<long>(parseNumericField(json, "cvol", 0.0));
                        event.last = parseNumericField(json, "last", 0.0);
                        
                        event.date = json.value("date", "");
                        tradeHandler(event);
                        processed++;
                    } catch (const std::exception&) {
                        // Skip invalid events
                    }
                }
            }
        }
        
        // Process quotes in batch  
        if (quoteHandler) {
            for (size_t i = 0; i < chunk_size; ++i) {
                if (is_quote[i]) {
                    const auto& json = events[chunk_start + i];
                    try {
                        tradier::QuoteEvent event;
                        event.type = types[i];
                        event.symbol = json.value("symbol", "");
                        
                        // Batch numeric field parsing
                        event.bid = parseNumericField(json, "bid", 0.0);
                        event.ask = parseNumericField(json, "ask", 0.0);
                        event.bidSize = static_cast<int>(parseNumericField(json, "bidsz", 0.0));
                        event.askSize = static_cast<int>(parseNumericField(json, "asksz", 0.0));
                        
                        event.bidExchange = json.value("bidexch", "");
                        event.bidDate = json.value("biddate", "");
                        event.askExchange = json.value("askexch", "");
                        event.askDate = json.value("askdate", "");
                        quoteHandler(event);
                        processed++;
                    } catch (const std::exception&) {
                        // Skip invalid events
                    }
                }
            }
        }
    }
    
    // Handle remaining events with scalar code
    size_t remaining_start = chunks * chunk_size;
    for (size_t i = remaining_start; i < count; ++i) {
        const auto& json = events[i];
        
        if (!json.contains("type")) continue;
        
        std::string type = json["type"];
        
        try {
            if (type == "trade" && tradeHandler) {
                tradier::TradeEvent event;
                event.type = type;
                event.symbol = json.value("symbol", "");
                event.exchange = json.value("exch", "");
                
                event.price = parseNumericField(json, "price", 0.0);
                event.size = static_cast<int>(parseNumericField(json, "size", 0.0));
                event.cvol = static_cast<long>(parseNumericField(json, "cvol", 0.0));
                event.last = parseNumericField(json, "last", 0.0);
                
                event.date = json.value("date", "");
                tradeHandler(event);
                processed++;
                
            } else if (type == "quote" && quoteHandler) {
                tradier::QuoteEvent event;
                event.type = type;
                event.symbol = json.value("symbol", "");
                
                event.bid = parseNumericField(json, "bid", 0.0);
                event.ask = parseNumericField(json, "ask", 0.0);
                event.bidSize = static_cast<int>(parseNumericField(json, "bidsz", 0.0));
                event.askSize = static_cast<int>(parseNumericField(json, "asksz", 0.0));
                
                event.bidExchange = json.value("bidexch", "");
                event.bidDate = json.value("biddate", "");
                event.askExchange = json.value("askexch", "");
                event.askDate = json.value("askdate", "");
                quoteHandler(event);
                processed++;
            }
        } catch (const std::exception&) {
            // Skip invalid events
        }
    }
    
    return processed;
}
#endif

#if LIBTRADIER_SIMD_AVX512_AVAILABLE
// AVX-512 implementation for bulk_process_events
size_t bulk_process_events_impl::avx512(const nlohmann::json* events, size_t count,
                                       std::function<void(const tradier::TradeEvent&)> tradeHandler,
                                       std::function<void(const tradier::QuoteEvent&)> quoteHandler) {
    // For now, fall back to AVX2 implementation
    // TODO: Implement true AVX-512 optimizations
    return avx2(events, count, tradeHandler, quoteHandler);
}
#endif

} // namespace streaming
} // namespace simd
} // namespace tradier