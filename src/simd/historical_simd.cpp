/*
 * libtradier - SIMD Implementation for Historical Data Processing
 * 
 * SECOND HIGHEST IMPACT TARGET: 2-4x performance improvement expected
 * Target: parseHistoricalDataList() and technical analysis calculations
 * 
 * Historical data processing is critical for:
 * - Backtesting and strategy analysis
 * - Real-time technical indicator calculations
 * - Risk management and portfolio analysis
 * 
 * Author: libtradier team
 * Version: 1.0
 */

#include "tradier/simd/vectorized_ops.hpp"

#ifdef LIBTRADIER_SIMD_ENABLED

namespace tradier {
namespace simd {
namespace historical_data {

//==============================================================================
// High-Performance OHLCV Data Processing Implementation
//==============================================================================

// Scalar implementation (fallback and baseline)
template<typename... Args>
auto bulk_ohlcv_processing_impl::scalar(Args&&... args) {
    const auto* raw_data = std::get<0>(std::forward_as_tuple(args...));
    auto& ohlcv_output = std::get<1>(std::forward_as_tuple(args...));
    size_t count = std::get<2>(std::forward_as_tuple(args...));
    
    size_t processed = 0;
    
    for (size_t i = 0; i < count; ++i) {
        const auto& data_point = raw_data[i];
        
        // Sequential OHLCV field extraction (current bottleneck)
        if (auto open = parseNumericField(data_point, "open")) {
            if (auto high = parseNumericField(data_point, "high")) {
                if (auto low = parseNumericField(data_point, "low")) {
                    if (auto close = parseNumericField(data_point, "close")) {
                        // All required fields present
                        ohlcv_output.open[processed] = *open;
                        ohlcv_output.high[processed] = *high;
                        ohlcv_output.low[processed] = *low;
                        ohlcv_output.close[processed] = *close;
                        ohlcv_output.volume[processed] = parseNumericField(data_point, "volume").value_or(0.0);
                        
                        // Basic validation
                        if (*high >= *low && *high >= *open && *high >= *close && 
                            *low <= *open && *low <= *close) {
                            processed++;
                        }
                    }
                }
            }
        }
    }
    
    return processed;
}

#ifdef LIBTRADIER_SIMD_AVX2
// AVX2 implementation - 4x parallel processing
template<typename... Args>
auto bulk_ohlcv_processing_impl::avx2(Args&&... args) {
    const auto* raw_data = std::get<0>(std::forward_as_tuple(args...));
    auto& ohlcv_output = std::get<1>(std::forward_as_tuple(args...));
    size_t count = std::get<2>(std::forward_as_tuple(args...));
    
    size_t processed = 0;
    const size_t simd_width = 4; // AVX2 processes 4 doubles at once
    
    // Process in SIMD-width chunks
    for (size_t i = 0; i + simd_width <= count; i += simd_width) {
        // Prepare SIMD vectors for parallel processing
        LIBTRADIER_SIMD_ALIGN double open_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double high_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double low_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double close_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double volume_values[simd_width];
        
        bool valid_ohlcv[simd_width] = {false};
        
        // Extract OHLCV fields for SIMD batch
        for (size_t j = 0; j < simd_width; ++j) {
            const auto& data_point = raw_data[i + j];
            
            auto open_opt = parseNumericField(data_point, "open");
            auto high_opt = parseNumericField(data_point, "high");
            auto low_opt = parseNumericField(data_point, "low");
            auto close_opt = parseNumericField(data_point, "close");
            
            if (open_opt && high_opt && low_opt && close_opt) {
                open_values[j] = *open_opt;
                high_values[j] = *high_opt;
                low_values[j] = *low_opt;
                close_values[j] = *close_opt;
                volume_values[j] = parseNumericField(data_point, "volume").value_or(0.0);
                valid_ohlcv[j] = true;
            } else {
                // Set to zero for invalid data points
                open_values[j] = 0.0;
                high_values[j] = 0.0;
                low_values[j] = 0.0;
                close_values[j] = 0.0;
                volume_values[j] = 0.0;
                valid_ohlcv[j] = false;
            }
        }
        
        // Load values into AVX2 registers
        __m256d open_vec = _mm256_load_pd(open_values);
        __m256d high_vec = _mm256_load_pd(high_values);
        __m256d low_vec = _mm256_load_pd(low_values);
        __m256d close_vec = _mm256_load_pd(close_values);
        __m256d volume_vec = _mm256_load_pd(volume_values);
        
        // SIMD validation: high >= low, high >= open, high >= close, low <= open, low <= close
        __m256d high_ge_low = _mm256_cmp_pd(high_vec, low_vec, _CMP_GE_OQ);
        __m256d high_ge_open = _mm256_cmp_pd(high_vec, open_vec, _CMP_GE_OQ);
        __m256d high_ge_close = _mm256_cmp_pd(high_vec, close_vec, _CMP_GE_OQ);
        __m256d low_le_open = _mm256_cmp_pd(low_vec, open_vec, _CMP_LE_OQ);
        __m256d low_le_close = _mm256_cmp_pd(low_vec, close_vec, _CMP_LE_OQ);
        
        // Combine all validation masks
        __m256d valid_mask = _mm256_and_pd(high_ge_low, high_ge_open);
        valid_mask = _mm256_and_pd(valid_mask, high_ge_close);
        valid_mask = _mm256_and_pd(valid_mask, low_le_open);
        valid_mask = _mm256_and_pd(valid_mask, low_le_close);
        
        // Store results (only valid OHLCV data)
        for (size_t j = 0; j < simd_width; ++j) {
            if (valid_ohlcv[j]) {
                // Check SIMD validation result
                double validation_result;
                _mm256_store_pd(&validation_result, valid_mask);
                
                if (validation_result != 0.0) { // Valid according to SIMD checks
                    ohlcv_output.open[processed] = open_values[j];
                    ohlcv_output.high[processed] = high_values[j];
                    ohlcv_output.low[processed] = low_values[j];
                    ohlcv_output.close[processed] = close_values[j];
                    ohlcv_output.volume[processed] = volume_values[j];
                    processed++;
                }
            }
        }
    }
    
    // Handle remaining elements with scalar processing
    for (size_t i = (count / simd_width) * simd_width; i < count; ++i) {
        const auto& data_point = raw_data[i];
        
        if (auto open = parseNumericField(data_point, "open")) {
            if (auto high = parseNumericField(data_point, "high")) {
                if (auto low = parseNumericField(data_point, "low")) {
                    if (auto close = parseNumericField(data_point, "close")) {
                        if (*high >= *low && *high >= *open && *high >= *close && 
                            *low <= *open && *low <= *close) {
                            ohlcv_output.open[processed] = *open;
                            ohlcv_output.high[processed] = *high;
                            ohlcv_output.low[processed] = *low;
                            ohlcv_output.close[processed] = *close;
                            ohlcv_output.volume[processed] = parseNumericField(data_point, "volume").value_or(0.0);
                            processed++;
                        }
                    }
                }
            }
        }
    }
    
    return processed;
}
#endif // LIBTRADIER_SIMD_AVX2

#ifdef LIBTRADIER_SIMD_AVX512
// AVX-512 implementation - 8x parallel processing (highest performance)
template<typename... Args>
auto bulk_ohlcv_processing_impl::avx512(Args&&... args) {
    const auto* raw_data = std::get<0>(std::forward_as_tuple(args...));
    auto& ohlcv_output = std::get<1>(std::forward_as_tuple(args...));
    size_t count = std::get<2>(std::forward_as_tuple(args...));
    
    size_t processed = 0;
    const size_t simd_width = 8; // AVX-512 processes 8 doubles at once
    
    // Process in SIMD-width chunks
    for (size_t i = 0; i + simd_width <= count; i += simd_width) {
        // Prepare SIMD vectors for parallel processing
        LIBTRADIER_SIMD_ALIGN double open_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double high_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double low_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double close_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double volume_values[simd_width];
        
        bool valid_ohlcv[simd_width] = {false};
        
        // Extract OHLCV fields for SIMD batch
        LIBTRADIER_SIMD_UNROLL(8)
        for (size_t j = 0; j < simd_width; ++j) {
            const auto& data_point = raw_data[i + j];
            
            auto open_opt = parseNumericField(data_point, "open");
            auto high_opt = parseNumericField(data_point, "high");
            auto low_opt = parseNumericField(data_point, "low");
            auto close_opt = parseNumericField(data_point, "close");
            
            if (open_opt && high_opt && low_opt && close_opt) {
                open_values[j] = *open_opt;
                high_values[j] = *high_opt;
                low_values[j] = *low_opt;
                close_values[j] = *close_opt;
                volume_values[j] = parseNumericField(data_point, "volume").value_or(0.0);
                valid_ohlcv[j] = true;
            } else {
                open_values[j] = 0.0;
                high_values[j] = 0.0;
                low_values[j] = 0.0;
                close_values[j] = 0.0;
                volume_values[j] = 0.0;
                valid_ohlcv[j] = false;
            }
        }
        
        // Load values into AVX-512 registers
        __m512d open_vec = _mm512_load_pd(open_values);
        __m512d high_vec = _mm512_load_pd(high_values);
        __m512d low_vec = _mm512_load_pd(low_values);
        __m512d close_vec = _mm512_load_pd(close_values);
        __m512d volume_vec = _mm512_load_pd(volume_values);
        
        // SIMD validation with masks
        __mmask8 high_ge_low = _mm512_cmp_pd_mask(high_vec, low_vec, _CMP_GE_OQ);
        __mmask8 high_ge_open = _mm512_cmp_pd_mask(high_vec, open_vec, _CMP_GE_OQ);
        __mmask8 high_ge_close = _mm512_cmp_pd_mask(high_vec, close_vec, _CMP_GE_OQ);
        __mmask8 low_le_open = _mm512_cmp_pd_mask(low_vec, open_vec, _CMP_LE_OQ);
        __mmask8 low_le_close = _mm512_cmp_pd_mask(low_vec, close_vec, _CMP_LE_OQ);
        
        // Combine all validation masks
        __mmask8 valid_mask = high_ge_low & high_ge_open & high_ge_close & low_le_open & low_le_close;
        
        // Store valid results using masked operations
        for (size_t j = 0; j < simd_width; ++j) {
            if (valid_ohlcv[j] && (valid_mask & (1 << j))) {
                ohlcv_output.open[processed] = open_values[j];
                ohlcv_output.high[processed] = high_values[j];
                ohlcv_output.low[processed] = low_values[j];
                ohlcv_output.close[processed] = close_values[j];
                ohlcv_output.volume[processed] = volume_values[j];
                processed++;
            }
        }
    }
    
    // Handle remaining elements with scalar processing
    for (size_t i = (count / simd_width) * simd_width; i < count; ++i) {
        const auto& data_point = raw_data[i];
        
        if (auto open = parseNumericField(data_point, "open")) {
            if (auto high = parseNumericField(data_point, "high")) {
                if (auto low = parseNumericField(data_point, "low")) {
                    if (auto close = parseNumericField(data_point, "close")) {
                        if (*high >= *low && *high >= *open && *high >= *close && 
                            *low <= *open && *low <= *close) {
                            ohlcv_output.open[processed] = *open;
                            ohlcv_output.high[processed] = *high;
                            ohlcv_output.low[processed] = *low;
                            ohlcv_output.close[processed] = *close;
                            ohlcv_output.volume[processed] = parseNumericField(data_point, "volume").value_or(0.0);
                            processed++;
                        }
                    }
                }
            }
        }
    }
    
    return processed;
}
#endif // LIBTRADIER_SIMD_AVX512

//==============================================================================
// Vectorized Technical Indicator Calculations
//==============================================================================

// Moving Average (Simple Moving Average)
template<typename... Args>
auto calculate_moving_average_impl::scalar(Args&&... args) {
    const auto* prices = std::get<0>(std::forward_as_tuple(args...));
    size_t window_size = std::get<1>(std::forward_as_tuple(args...));
    auto* output = std::get<2>(std::forward_as_tuple(args...));
    size_t count = std::get<3>(std::forward_as_tuple(args...));
    
    if (count < window_size) return 0;
    
    size_t calculated = 0;
    
    // Calculate moving averages
    for (size_t i = window_size - 1; i < count; ++i) {
        double sum = 0.0;
        for (size_t j = i - window_size + 1; j <= i; ++j) {
            sum += prices[j];
        }
        output[calculated++] = sum / window_size;
    }
    
    return calculated;
}

#ifdef LIBTRADIER_SIMD_AVX2
template<typename... Args>
auto calculate_moving_average_impl::avx2(Args&&... args) {
    const auto* prices = std::get<0>(std::forward_as_tuple(args...));
    size_t window_size = std::get<1>(std::forward_as_tuple(args...));
    auto* output = std::get<2>(std::forward_as_tuple(args...));
    size_t count = std::get<3>(std::forward_as_tuple(args...));
    
    if (count < window_size) return 0;
    
    const size_t simd_width = 4;
    size_t calculated = 0;
    
    // Use SIMD for window summation when possible
    for (size_t i = window_size - 1; i < count; ++i) {
        double sum = 0.0;
        
        // SIMD acceleration for window summation
        size_t window_start = i - window_size + 1;
        size_t simd_count = window_size / simd_width;
        
        __m256d sum_vec = _mm256_setzero_pd();
        
        // Process window in SIMD chunks
        for (size_t chunk = 0; chunk < simd_count; ++chunk) {
            size_t idx = window_start + chunk * simd_width;
            __m256d price_vec = _mm256_loadu_pd(&prices[idx]);
            sum_vec = _mm256_add_pd(sum_vec, price_vec);
        }
        
        // Horizontal sum of SIMD vector
        LIBTRADIER_SIMD_ALIGN double sum_array[simd_width];
        _mm256_store_pd(sum_array, sum_vec);
        for (size_t j = 0; j < simd_width; ++j) {
            sum += sum_array[j];
        }
        
        // Handle remaining elements
        for (size_t j = window_start + simd_count * simd_width; j <= i; ++j) {
            sum += prices[j];
        }
        
        output[calculated++] = sum / window_size;
    }
    
    return calculated;
}
#endif // LIBTRADIER_SIMD_AVX2

// RSI (Relative Strength Index) calculation
template<typename... Args>
auto calculate_rsi_impl::scalar(Args&&... args) {
    const auto* prices = std::get<0>(std::forward_as_tuple(args...));
    size_t period = std::get<1>(std::forward_as_tuple(args...));
    auto* output = std::get<2>(std::forward_as_tuple(args...));
    size_t count = std::get<3>(std::forward_as_tuple(args...));
    
    if (count < period + 1) return 0;
    
    size_t calculated = 0;
    
    // Calculate price changes
    std::vector<double> gains(count - 1);
    std::vector<double> losses(count - 1);
    
    for (size_t i = 1; i < count; ++i) {
        double change = prices[i] - prices[i - 1];
        if (change > 0) {
            gains[i - 1] = change;
            losses[i - 1] = 0.0;
        } else {
            gains[i - 1] = 0.0;
            losses[i - 1] = -change;
        }
    }
    
    // Calculate RSI for each period
    for (size_t i = period; i < count; ++i) {
        double avg_gain = 0.0, avg_loss = 0.0;
        
        // Average gains and losses over period
        for (size_t j = i - period; j < i; ++j) {
            avg_gain += gains[j];
            avg_loss += losses[j];
        }
        avg_gain /= period;
        avg_loss /= period;
        
        // Calculate RSI
        if (avg_loss == 0.0) {
            output[calculated++] = 100.0;
        } else {
            double rs = avg_gain / avg_loss;
            output[calculated++] = 100.0 - (100.0 / (1.0 + rs));
        }
    }
    
    return calculated;
}

#ifdef LIBTRADIER_SIMD_AVX2
template<typename... Args>
auto calculate_rsi_impl::avx2(Args&&... args) {
    const auto* prices = std::get<0>(std::forward_as_tuple(args...));
    size_t period = std::get<1>(std::forward_as_tuple(args...));
    auto* output = std::get<2>(std::forward_as_tuple(args...));
    size_t count = std::get<3>(std::forward_as_tuple(args...));
    
    if (count < period + 1) return 0;
    
    const size_t simd_width = 4;
    size_t calculated = 0;
    
    // Calculate price changes with SIMD
    std::vector<double> gains(count - 1);
    std::vector<double> losses(count - 1);
    
    // SIMD price change calculation
    for (size_t i = 1; i + simd_width <= count; i += simd_width) {
        __m256d curr_prices = _mm256_loadu_pd(&prices[i]);
        __m256d prev_prices = _mm256_loadu_pd(&prices[i - 1]);
        __m256d changes = _mm256_sub_pd(curr_prices, prev_prices);
        
        // Separate gains and losses
        __m256d zero = _mm256_setzero_pd();
        __m256d gains_vec = _mm256_max_pd(changes, zero);
        __m256d losses_vec = _mm256_max_pd(_mm256_sub_pd(zero, changes), zero);
        
        _mm256_storeu_pd(&gains[i - 1], gains_vec);
        _mm256_storeu_pd(&losses[i - 1], losses_vec);
    }
    
    // Handle remaining elements
    for (size_t i = ((count - 1) / simd_width) * simd_width + 1; i < count; ++i) {
        double change = prices[i] - prices[i - 1];
        if (change > 0) {
            gains[i - 1] = change;
            losses[i - 1] = 0.0;
        } else {
            gains[i - 1] = 0.0;
            losses[i - 1] = -change;
        }
    }
    
    // Calculate RSI (similar to scalar but with SIMD averages)
    for (size_t i = period; i < count; ++i) {
        // Use SIMD for averaging when period is large enough
        double avg_gain = 0.0, avg_loss = 0.0;
        
        size_t period_start = i - period;
        size_t simd_periods = period / simd_width;
        
        __m256d gain_sum = _mm256_setzero_pd();
        __m256d loss_sum = _mm256_setzero_pd();
        
        for (size_t chunk = 0; chunk < simd_periods; ++chunk) {
            size_t idx = period_start + chunk * simd_width;
            __m256d gain_vec = _mm256_loadu_pd(&gains[idx]);
            __m256d loss_vec = _mm256_loadu_pd(&losses[idx]);
            
            gain_sum = _mm256_add_pd(gain_sum, gain_vec);
            loss_sum = _mm256_add_pd(loss_sum, loss_vec);
        }
        
        // Horizontal sum
        LIBTRADIER_SIMD_ALIGN double gain_array[simd_width];
        LIBTRADIER_SIMD_ALIGN double loss_array[simd_width];
        _mm256_store_pd(gain_array, gain_sum);
        _mm256_store_pd(loss_array, loss_sum);
        
        for (size_t j = 0; j < simd_width; ++j) {
            avg_gain += gain_array[j];
            avg_loss += loss_array[j];
        }
        
        // Handle remaining elements
        for (size_t j = period_start + simd_periods * simd_width; j < i; ++j) {
            avg_gain += gains[j];
            avg_loss += losses[j];
        }
        
        avg_gain /= period;
        avg_loss /= period;
        
        // Calculate RSI
        if (avg_loss == 0.0) {
            output[calculated++] = 100.0;
        } else {
            double rs = avg_gain / avg_loss;
            output[calculated++] = 100.0 - (100.0 / (1.0 + rs));
        }
    }
    
    return calculated;
}
#endif // LIBTRADIER_SIMD_AVX2

// Volatility calculation (rolling standard deviation)
template<typename... Args>
auto calculate_volatility_impl::scalar(Args&&... args) {
    const auto* returns = std::get<0>(std::forward_as_tuple(args...));
    size_t window = std::get<1>(std::forward_as_tuple(args...));
    auto* output = std::get<2>(std::forward_as_tuple(args...));
    size_t count = std::get<3>(std::forward_as_tuple(args...));
    
    if (count < window) return 0;
    
    size_t calculated = 0;
    
    for (size_t i = window - 1; i < count; ++i) {
        // Calculate mean
        double mean = 0.0;
        for (size_t j = i - window + 1; j <= i; ++j) {
            mean += returns[j];
        }
        mean /= window;
        
        // Calculate variance
        double variance = 0.0;
        for (size_t j = i - window + 1; j <= i; ++j) {
            double diff = returns[j] - mean;
            variance += diff * diff;
        }
        variance /= (window - 1); // Sample variance
        
        output[calculated++] = std::sqrt(variance);
    }
    
    return calculated;
}

#ifdef LIBTRADIER_SIMD_AVX2
template<typename... Args>
auto calculate_volatility_impl::avx2(Args&&... args) {
    const auto* returns = std::get<0>(std::forward_as_tuple(args...));
    size_t window = std::get<1>(std::forward_as_tuple(args...));
    auto* output = std::get<2>(std::forward_as_tuple(args...));
    size_t count = std::get<3>(std::forward_as_tuple(args...));
    
    if (count < window) return 0;
    
    const size_t simd_width = 4;
    size_t calculated = 0;
    
    for (size_t i = window - 1; i < count; ++i) {
        size_t window_start = i - window + 1;
        
        // SIMD mean calculation
        __m256d mean_sum = _mm256_setzero_pd();
        size_t simd_count = window / simd_width;
        
        for (size_t chunk = 0; chunk < simd_count; ++chunk) {
            size_t idx = window_start + chunk * simd_width;
            __m256d return_vec = _mm256_loadu_pd(&returns[idx]);
            mean_sum = _mm256_add_pd(mean_sum, return_vec);
        }
        
        // Horizontal sum and handle remaining elements
        LIBTRADIER_SIMD_ALIGN double mean_array[simd_width];
        _mm256_store_pd(mean_array, mean_sum);
        
        double mean = 0.0;
        for (size_t j = 0; j < simd_width; ++j) {
            mean += mean_array[j];
        }
        
        for (size_t j = window_start + simd_count * simd_width; j <= i; ++j) {
            mean += returns[j];
        }
        mean /= window;
        
        // SIMD variance calculation
        __m256d mean_vec = _mm256_set1_pd(mean);
        __m256d variance_sum = _mm256_setzero_pd();
        
        for (size_t chunk = 0; chunk < simd_count; ++chunk) {
            size_t idx = window_start + chunk * simd_width;
            __m256d return_vec = _mm256_loadu_pd(&returns[idx]);
            __m256d diff = _mm256_sub_pd(return_vec, mean_vec);
            __m256d diff_sq = _mm256_mul_pd(diff, diff);
            variance_sum = _mm256_add_pd(variance_sum, diff_sq);
        }
        
        // Horizontal sum and handle remaining elements
        LIBTRADIER_SIMD_ALIGN double variance_array[simd_width];
        _mm256_store_pd(variance_array, variance_sum);
        
        double variance = 0.0;
        for (size_t j = 0; j < simd_width; ++j) {
            variance += variance_array[j];
        }
        
        for (size_t j = window_start + simd_count * simd_width; j <= i; ++j) {
            double diff = returns[j] - mean;
            variance += diff * diff;
        }
        variance /= (window - 1);
        
        output[calculated++] = std::sqrt(variance);
    }
    
    return calculated;
}
#endif // LIBTRADIER_SIMD_AVX2

} // namespace historical_data
} // namespace simd
} // namespace tradier

#endif // LIBTRADIER_SIMD_ENABLED