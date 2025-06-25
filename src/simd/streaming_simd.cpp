/*
 * libtradier - SIMD Implementation for Streaming Event Processing
 * 
 * HIGHEST IMPACT TARGET: 3-5x performance improvement expected
 * Target: processEvent() in streaming.cpp:252-337 (critical path)
 * 
 * This is the most latency-sensitive code in the entire library
 * Every microsecond improvement here translates to trading advantage
 * 
 * Author: libtradier team
 * Version: 1.0
 */

#include "tradier/simd/vectorized_ops.hpp"

#ifdef LIBTRADIER_SIMD_ENABLED

namespace tradier {
namespace simd {
namespace streaming {

//==============================================================================
// High-Performance Trade Event Processing Implementation
//==============================================================================

// Scalar implementation (fallback and baseline)
template<typename... Args>
auto process_trade_events_impl::scalar(Args&&... args) {
    // Traditional sequential implementation
    const auto* raw_events = std::get<0>(std::forward_as_tuple(args...));
    auto* event_output = std::get<1>(std::forward_as_tuple(args...));
    size_t count = std::get<2>(std::forward_as_tuple(args...));
    
    size_t processed = 0;
    
    for (size_t i = 0; i < count; ++i) {
        const auto& raw_event = raw_events[i];
        
        // Sequential field extraction (current bottleneck)
        if (auto price = parseNumericField(raw_event, "price")) {
            event_output->prices[processed] = *price;
        } else {
            continue; // Skip invalid events
        }
        
        if (auto volume = parseNumericField(raw_event, "volume")) {
            event_output->volumes[processed] = *volume;
        } else {
            event_output->volumes[processed] = 0.0;
        }
        
        if (auto size = parseNumericField(raw_event, "size")) {
            event_output->sizes[processed] = *size;
        } else {
            event_output->sizes[processed] = 0.0;
        }
        
        if (auto timestamp = parseNumericField(raw_event, "timestamp")) {
            event_output->timestamps[processed] = *timestamp;
        } else {
            event_output->timestamps[processed] = 0.0;
        }
        
        processed++;
    }
    
    return processed;
}

#ifdef LIBTRADIER_SIMD_AVX2
// AVX2 implementation - 4x parallel processing
template<typename... Args>
auto process_trade_events_impl::avx2(Args&&... args) {
    const auto* raw_events = std::get<0>(std::forward_as_tuple(args...));
    auto* event_output = std::get<1>(std::forward_as_tuple(args...));
    size_t count = std::get<2>(std::forward_as_tuple(args...));
    
    size_t processed = 0;
    const size_t simd_width = 4; // AVX2 processes 4 doubles at once
    
    // Process in SIMD-width chunks
    for (size_t i = 0; i + simd_width <= count; i += simd_width) {
        // Prepare SIMD vectors for parallel processing
        LIBTRADIER_SIMD_ALIGN double price_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double volume_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double size_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double timestamp_values[simd_width];
        
        bool valid_events[simd_width] = {false};
        
        // Extract fields for SIMD batch (this could be further optimized)
        for (size_t j = 0; j < simd_width; ++j) {
            const auto& raw_event = raw_events[i + j];
            
            if (auto price = parseNumericField(raw_event, "price")) {
                price_values[j] = *price;
                valid_events[j] = true;
            } else {
                price_values[j] = 0.0;
                valid_events[j] = false;
            }
            
            volume_values[j] = parseNumericField(raw_event, "volume").value_or(0.0);
            size_values[j] = parseNumericField(raw_event, "size").value_or(0.0);
            timestamp_values[j] = parseNumericField(raw_event, "timestamp").value_or(0.0);
        }
        
        // Load values into AVX2 registers
        __m256d price_vec = _mm256_load_pd(price_values);
        __m256d volume_vec = _mm256_load_pd(volume_values);
        __m256d size_vec = _mm256_load_pd(size_values);
        __m256d timestamp_vec = _mm256_load_pd(timestamp_values);
        
        // SIMD validation and processing
        __m256d zero = _mm256_setzero_pd();
        __m256d price_valid = _mm256_cmp_pd(price_vec, zero, _CMP_GT_OQ); // price > 0
        __m256d volume_valid = _mm256_cmp_pd(volume_vec, zero, _CMP_GE_OQ); // volume >= 0
        
        // Combine validation masks
        __m256d valid_mask = _mm256_and_pd(price_valid, volume_valid);
        
        // Apply validation mask to data
        price_vec = _mm256_and_pd(price_vec, valid_mask);
        volume_vec = _mm256_and_pd(volume_vec, valid_mask);
        size_vec = _mm256_and_pd(size_vec, valid_mask);
        timestamp_vec = _mm256_and_pd(timestamp_vec, valid_mask);
        
        // Store results (only valid events)
        for (size_t j = 0; j < simd_width; ++j) {
            if (valid_events[j]) {
                event_output->prices[processed] = price_values[j];
                event_output->volumes[processed] = volume_values[j];
                event_output->sizes[processed] = size_values[j];
                event_output->timestamps[processed] = timestamp_values[j];
                processed++;
            }
        }
    }
    
    // Handle remaining elements with scalar processing
    for (size_t i = (count / simd_width) * simd_width; i < count; ++i) {
        // Scalar fallback for remaining elements
        const auto& raw_event = raw_events[i];
        
        if (auto price = parseNumericField(raw_event, "price")) {
            event_output->prices[processed] = *price;
            event_output->volumes[processed] = parseNumericField(raw_event, "volume").value_or(0.0);
            event_output->sizes[processed] = parseNumericField(raw_event, "size").value_or(0.0);
            event_output->timestamps[processed] = parseNumericField(raw_event, "timestamp").value_or(0.0);
            processed++;
        }
    }
    
    return processed;
}
#endif // LIBTRADIER_SIMD_AVX2

#ifdef LIBTRADIER_SIMD_AVX512
// AVX-512 implementation - 8x parallel processing (highest performance)
template<typename... Args>
auto process_trade_events_impl::avx512(Args&&... args) {
    const auto* raw_events = std::get<0>(std::forward_as_tuple(args...));
    auto* event_output = std::get<1>(std::forward_as_tuple(args...));
    size_t count = std::get<2>(std::forward_as_tuple(args...));
    
    size_t processed = 0;
    const size_t simd_width = 8; // AVX-512 processes 8 doubles at once
    
    // Process in SIMD-width chunks
    for (size_t i = 0; i + simd_width <= count; i += simd_width) {
        // Prepare SIMD vectors for parallel processing
        LIBTRADIER_SIMD_ALIGN double price_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double volume_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double size_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double timestamp_values[simd_width];
        
        bool valid_events[simd_width] = {false};
        
        // Extract fields for SIMD batch
        LIBTRADIER_SIMD_UNROLL(8)
        for (size_t j = 0; j < simd_width; ++j) {
            const auto& raw_event = raw_events[i + j];
            
            if (auto price = parseNumericField(raw_event, "price")) {
                price_values[j] = *price;
                valid_events[j] = true;
            } else {
                price_values[j] = 0.0;
                valid_events[j] = false;
            }
            
            volume_values[j] = parseNumericField(raw_event, "volume").value_or(0.0);
            size_values[j] = parseNumericField(raw_event, "size").value_or(0.0);
            timestamp_values[j] = parseNumericField(raw_event, "timestamp").value_or(0.0);
        }
        
        // Load values into AVX-512 registers
        __m512d price_vec = _mm512_load_pd(price_values);
        __m512d volume_vec = _mm512_load_pd(volume_values);
        __m512d size_vec = _mm512_load_pd(size_values);
        __m512d timestamp_vec = _mm512_load_pd(timestamp_values);
        
        // SIMD validation with masks
        __mmask8 price_valid = _mm512_cmp_pd_mask(price_vec, _mm512_setzero_pd(), _CMP_GT_OQ);
        __mmask8 volume_valid = _mm512_cmp_pd_mask(volume_vec, _mm512_setzero_pd(), _CMP_GE_OQ);
        
        // Combine validation masks
        __mmask8 valid_mask = price_valid & volume_valid;
        
        // Store valid results using masked operations
        _mm512_mask_store_pd(&event_output->prices[processed], valid_mask, price_vec);
        _mm512_mask_store_pd(&event_output->volumes[processed], valid_mask, volume_vec);
        _mm512_mask_store_pd(&event_output->sizes[processed], valid_mask, size_vec);
        _mm512_mask_store_pd(&event_output->timestamps[processed], valid_mask, timestamp_vec);
        
        // Count valid events
        processed += __builtin_popcountll(valid_mask);
    }
    
    // Handle remaining elements with scalar processing
    for (size_t i = (count / simd_width) * simd_width; i < count; ++i) {
        const auto& raw_event = raw_events[i];
        
        if (auto price = parseNumericField(raw_event, "price")) {
            event_output->prices[processed] = *price;
            event_output->volumes[processed] = parseNumericField(raw_event, "volume").value_or(0.0);
            event_output->sizes[processed] = parseNumericField(raw_event, "size").value_or(0.0);
            event_output->timestamps[processed] = parseNumericField(raw_event, "timestamp").value_or(0.0);
            processed++;
        }
    }
    
    return processed;
}
#endif // LIBTRADIER_SIMD_AVX512

//==============================================================================
// Fast Event Processing for Ultra-Low Latency
//==============================================================================

// Single event processing optimized for minimum latency
template<typename... Args>
auto fast_event_processing_impl::scalar(Args&&... args) {
    // Baseline scalar implementation
    const auto& raw_event = std::get<0>(std::forward_as_tuple(args...));
    auto& processed_event = std::get<1>(std::forward_as_tuple(args...));
    uint32_t validation_flags = std::get<2>(std::forward_as_tuple(args...));
    
    // Sequential processing (current implementation)
    if (validation_flags & 0x1) { // Price validation
        if (auto price = parseNumericField(raw_event, "price")) {
            processed_event.price = *price;
        } else {
            return false;
        }
    }
    
    if (validation_flags & 0x2) { // Volume validation
        processed_event.volume = parseNumericField(raw_event, "volume").value_or(0.0);
    }
    
    if (validation_flags & 0x4) { // Size validation
        processed_event.size = parseNumericField(raw_event, "size").value_or(0.0);
    }
    
    if (validation_flags & 0x8) { // Timestamp validation
        processed_event.timestamp = parseNumericField(raw_event, "timestamp").value_or(0.0);
    }
    
    return true;
}

#ifdef LIBTRADIER_SIMD_AVX2
template<typename... Args>
auto fast_event_processing_impl::avx2(Args&&... args) {
    // For single event processing, AVX2 might not provide benefit
    // due to overhead. Consider using for batch processing instead.
    return fast_event_processing_impl::scalar(std::forward<Args>(args)...);
}
#endif

//==============================================================================
// Market Data Aggregation with SIMD
//==============================================================================

template<typename... Args>
auto aggregate_market_data_impl::scalar(Args&&... args) {
    // Traditional sequential aggregation
    const auto* events = std::get<0>(std::forward_as_tuple(args...));
    auto& aggregated_data = std::get<1>(std::forward_as_tuple(args...));
    double time_window = std::get<2>(std::forward_as_tuple(args...));
    size_t count = std::get<3>(std::forward_as_tuple(args...));
    
    double sum_price = 0.0;
    double sum_volume = 0.0;
    double min_price = std::numeric_limits<double>::max();
    double max_price = std::numeric_limits<double>::min();
    size_t valid_events = 0;
    
    for (size_t i = 0; i < count; ++i) {
        const auto& event = events[i];
        
        if (event.price > 0.0) {
            sum_price += event.price;
            sum_volume += event.volume;
            min_price = std::min(min_price, event.price);
            max_price = std::max(max_price, event.price);
            valid_events++;
        }
    }
    
    if (valid_events > 0) {
        aggregated_data.average_price = sum_price / valid_events;
        aggregated_data.total_volume = sum_volume;
        aggregated_data.min_price = min_price;
        aggregated_data.max_price = max_price;
        aggregated_data.event_count = valid_events;
    }
    
    return valid_events;
}

#ifdef LIBTRADIER_SIMD_AVX2
template<typename... Args>
auto aggregate_market_data_impl::avx2(Args&&... args) {
    const auto* events = std::get<0>(std::forward_as_tuple(args...));
    auto& aggregated_data = std::get<1>(std::forward_as_tuple(args...));
    double time_window = std::get<2>(std::forward_as_tuple(args...));
    size_t count = std::get<3>(std::forward_as_tuple(args...));
    
    const size_t simd_width = 4;
    
    // SIMD accumulators
    __m256d sum_price_vec = _mm256_setzero_pd();
    __m256d sum_volume_vec = _mm256_setzero_pd();
    __m256d min_price_vec = _mm256_set1_pd(std::numeric_limits<double>::max());
    __m256d max_price_vec = _mm256_set1_pd(std::numeric_limits<double>::min());
    
    size_t valid_events = 0;
    
    // Process in SIMD chunks
    for (size_t i = 0; i + simd_width <= count; i += simd_width) {
        // Load prices and volumes
        LIBTRADIER_SIMD_ALIGN double prices[simd_width];
        LIBTRADIER_SIMD_ALIGN double volumes[simd_width];
        
        for (size_t j = 0; j < simd_width; ++j) {
            prices[j] = events[i + j].price;
            volumes[j] = events[i + j].volume;
        }
        
        __m256d price_vec = _mm256_load_pd(prices);
        __m256d volume_vec = _mm256_load_pd(volumes);
        
        // Validate prices (> 0)
        __m256d zero = _mm256_setzero_pd();
        __m256d valid_mask = _mm256_cmp_pd(price_vec, zero, _CMP_GT_OQ);
        
        // Masked accumulation
        sum_price_vec = _mm256_add_pd(sum_price_vec, _mm256_and_pd(price_vec, valid_mask));
        sum_volume_vec = _mm256_add_pd(sum_volume_vec, _mm256_and_pd(volume_vec, valid_mask));
        
        // Min/Max with masking
        min_price_vec = _mm256_min_pd(min_price_vec, 
            _mm256_blendv_pd(min_price_vec, price_vec, valid_mask));
        max_price_vec = _mm256_max_pd(max_price_vec, 
            _mm256_blendv_pd(max_price_vec, price_vec, valid_mask));
        
        // Count valid events
        int mask_int = _mm256_movemask_pd(valid_mask);
        valid_events += __builtin_popcount(mask_int);
    }
    
    // Horizontal reduction of SIMD results
    LIBTRADIER_SIMD_ALIGN double sum_price_array[simd_width];
    LIBTRADIER_SIMD_ALIGN double sum_volume_array[simd_width];
    LIBTRADIER_SIMD_ALIGN double min_price_array[simd_width];
    LIBTRADIER_SIMD_ALIGN double max_price_array[simd_width];
    
    _mm256_store_pd(sum_price_array, sum_price_vec);
    _mm256_store_pd(sum_volume_array, sum_volume_vec);
    _mm256_store_pd(min_price_array, min_price_vec);
    _mm256_store_pd(max_price_array, max_price_vec);
    
    double total_price = 0.0, total_volume = 0.0;
    double final_min = std::numeric_limits<double>::max();
    double final_max = std::numeric_limits<double>::min();
    
    for (size_t i = 0; i < simd_width; ++i) {
        total_price += sum_price_array[i];
        total_volume += sum_volume_array[i];
        final_min = std::min(final_min, min_price_array[i]);
        final_max = std::max(final_max, max_price_array[i]);
    }
    
    // Handle remaining elements
    for (size_t i = (count / simd_width) * simd_width; i < count; ++i) {
        const auto& event = events[i];
        if (event.price > 0.0) {
            total_price += event.price;
            total_volume += event.volume;
            final_min = std::min(final_min, event.price);
            final_max = std::max(final_max, event.price);
            valid_events++;
        }
    }
    
    // Store results
    if (valid_events > 0) {
        aggregated_data.average_price = total_price / valid_events;
        aggregated_data.total_volume = total_volume;
        aggregated_data.min_price = final_min;
        aggregated_data.max_price = final_max;
        aggregated_data.event_count = valid_events;
    }
    
    return valid_events;
}
#endif // LIBTRADIER_SIMD_AVX2

} // namespace streaming
} // namespace simd
} // namespace tradier

#endif // LIBTRADIER_SIMD_ENABLED