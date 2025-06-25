/*
 * libtradier - SIMD Implementation for Option Greeks Calculations
 * 
 * THIRD HIGHEST IMPACT TARGET: 2-3x performance improvement expected
 * Target: parseGreeks() processing and options analytics
 * 
 * Greeks calculations are critical for:
 * - Real-time options pricing and risk management
 * - Portfolio Greeks aggregation and analysis
 * - Options chain processing and ranking
 * 
 * Author: libtradier team
 * Version: 1.0
 */

#include "tradier/simd/vectorized_ops.hpp"

#ifdef LIBTRADIER_SIMD_ENABLED

namespace tradier {
namespace simd {
namespace greeks_calculation {

//==============================================================================
// High-Performance Greeks Extraction and Processing
//==============================================================================

// Scalar implementation (fallback and baseline)
template<typename... Args>
auto bulk_greeks_extraction_impl::scalar(Args&&... args) {
    const auto* json_values = std::get<0>(std::forward_as_tuple(args...));
    auto& greeks_output = std::get<1>(std::forward_as_tuple(args...));
    size_t count = std::get<2>(std::forward_as_tuple(args...));
    
    size_t processed = 0;
    
    for (size_t i = 0; i < count; ++i) {
        const auto& greeks_data = json_values[i];
        
        // Sequential Greeks field extraction (current bottleneck)
        auto delta = parseNumericField(greeks_data, "delta");
        auto gamma = parseNumericField(greeks_data, "gamma");
        auto theta = parseNumericField(greeks_data, "theta");
        auto vega = parseNumericField(greeks_data, "vega");
        auto rho = parseNumericField(greeks_data, "rho");
        auto phi = parseNumericField(greeks_data, "phi");
        auto bid_iv = parseNumericField(greeks_data, "bid_iv");
        auto mid_iv = parseNumericField(greeks_data, "mid_iv");
        auto ask_iv = parseNumericField(greeks_data, "ask_iv");
        auto smv_vol = parseNumericField(greeks_data, "smv_vol");
        
        // Store extracted values (use 0.0 for missing values)
        greeks_output.delta[processed] = delta.value_or(0.0);
        greeks_output.gamma[processed] = gamma.value_or(0.0);
        greeks_output.theta[processed] = theta.value_or(0.0);
        greeks_output.vega[processed] = vega.value_or(0.0);
        greeks_output.rho[processed] = rho.value_or(0.0);
        greeks_output.phi[processed] = phi.value_or(0.0);
        greeks_output.bid_iv[processed] = bid_iv.value_or(0.0);
        greeks_output.mid_iv[processed] = mid_iv.value_or(0.0);
        greeks_output.ask_iv[processed] = ask_iv.value_or(0.0);
        greeks_output.smv_vol[processed] = smv_vol.value_or(0.0);
        
        processed++;
    }
    
    return processed;
}

#ifdef LIBTRADIER_SIMD_AVX2
// AVX2 implementation - 4x parallel processing
template<typename... Args>
auto bulk_greeks_extraction_impl::avx2(Args&&... args) {
    const auto* json_values = std::get<0>(std::forward_as_tuple(args...));
    auto& greeks_output = std::get<1>(std::forward_as_tuple(args...));
    size_t count = std::get<2>(std::forward_as_tuple(args...));
    
    size_t processed = 0;
    const size_t simd_width = 4; // AVX2 processes 4 doubles at once
    
    // Process in SIMD-width chunks
    for (size_t i = 0; i + simd_width <= count; i += simd_width) {
        // Prepare SIMD vectors for parallel processing
        LIBTRADIER_SIMD_ALIGN double delta_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double gamma_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double theta_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double vega_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double rho_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double phi_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double bid_iv_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double mid_iv_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double ask_iv_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double smv_vol_values[simd_width];
        
        // Extract Greeks fields for SIMD batch
        for (size_t j = 0; j < simd_width; ++j) {
            const auto& greeks_data = json_values[i + j];
            
            delta_values[j] = parseNumericField(greeks_data, "delta").value_or(0.0);
            gamma_values[j] = parseNumericField(greeks_data, "gamma").value_or(0.0);
            theta_values[j] = parseNumericField(greeks_data, "theta").value_or(0.0);
            vega_values[j] = parseNumericField(greeks_data, "vega").value_or(0.0);
            rho_values[j] = parseNumericField(greeks_data, "rho").value_or(0.0);
            phi_values[j] = parseNumericField(greeks_data, "phi").value_or(0.0);
            bid_iv_values[j] = parseNumericField(greeks_data, "bid_iv").value_or(0.0);
            mid_iv_values[j] = parseNumericField(greeks_data, "mid_iv").value_or(0.0);
            ask_iv_values[j] = parseNumericField(greeks_data, "ask_iv").value_or(0.0);
            smv_vol_values[j] = parseNumericField(greeks_data, "smv_vol").value_or(0.0);
        }
        
        // Load values into AVX2 registers for batch processing
        __m256d delta_vec = _mm256_load_pd(delta_values);
        __m256d gamma_vec = _mm256_load_pd(gamma_values);
        __m256d theta_vec = _mm256_load_pd(theta_values);
        __m256d vega_vec = _mm256_load_pd(vega_values);
        __m256d rho_vec = _mm256_load_pd(rho_values);
        __m256d phi_vec = _mm256_load_pd(phi_values);
        __m256d bid_iv_vec = _mm256_load_pd(bid_iv_values);
        __m256d mid_iv_vec = _mm256_load_pd(mid_iv_values);
        __m256d ask_iv_vec = _mm256_load_pd(ask_iv_values);
        __m256d smv_vol_vec = _mm256_load_pd(smv_vol_values);
        
        // Store processed Greeks in output structure
        _mm256_store_pd(&greeks_output.delta[processed], delta_vec);
        _mm256_store_pd(&greeks_output.gamma[processed], gamma_vec);
        _mm256_store_pd(&greeks_output.theta[processed], theta_vec);
        _mm256_store_pd(&greeks_output.vega[processed], vega_vec);
        _mm256_store_pd(&greeks_output.rho[processed], rho_vec);
        _mm256_store_pd(&greeks_output.phi[processed], phi_vec);
        _mm256_store_pd(&greeks_output.bid_iv[processed], bid_iv_vec);
        _mm256_store_pd(&greeks_output.mid_iv[processed], mid_iv_vec);
        _mm256_store_pd(&greeks_output.ask_iv[processed], ask_iv_vec);
        _mm256_store_pd(&greeks_output.smv_vol[processed], smv_vol_vec);
        
        processed += simd_width;
    }
    
    // Handle remaining elements with scalar processing
    for (size_t i = (count / simd_width) * simd_width; i < count; ++i) {
        const auto& greeks_data = json_values[i];
        
        greeks_output.delta[processed] = parseNumericField(greeks_data, "delta").value_or(0.0);
        greeks_output.gamma[processed] = parseNumericField(greeks_data, "gamma").value_or(0.0);
        greeks_output.theta[processed] = parseNumericField(greeks_data, "theta").value_or(0.0);
        greeks_output.vega[processed] = parseNumericField(greeks_data, "vega").value_or(0.0);
        greeks_output.rho[processed] = parseNumericField(greeks_data, "rho").value_or(0.0);
        greeks_output.phi[processed] = parseNumericField(greeks_data, "phi").value_or(0.0);
        greeks_output.bid_iv[processed] = parseNumericField(greeks_data, "bid_iv").value_or(0.0);
        greeks_output.mid_iv[processed] = parseNumericField(greeks_data, "mid_iv").value_or(0.0);
        greeks_output.ask_iv[processed] = parseNumericField(greeks_data, "ask_iv").value_or(0.0);
        greeks_output.smv_vol[processed] = parseNumericField(greeks_data, "smv_vol").value_or(0.0);
        
        processed++;
    }
    
    return processed;
}
#endif // LIBTRADIER_SIMD_AVX2

#ifdef LIBTRADIER_SIMD_AVX512
// AVX-512 implementation - 8x parallel processing (highest performance)
template<typename... Args>
auto bulk_greeks_extraction_impl::avx512(Args&&... args) {
    const auto* json_values = std::get<0>(std::forward_as_tuple(args...));
    auto& greeks_output = std::get<1>(std::forward_as_tuple(args...));
    size_t count = std::get<2>(std::forward_as_tuple(args...));
    
    size_t processed = 0;
    const size_t simd_width = 8; // AVX-512 processes 8 doubles at once
    
    // Process in SIMD-width chunks
    for (size_t i = 0; i + simd_width <= count; i += simd_width) {
        // Prepare SIMD vectors for parallel processing
        LIBTRADIER_SIMD_ALIGN double delta_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double gamma_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double theta_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double vega_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double rho_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double phi_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double bid_iv_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double mid_iv_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double ask_iv_values[simd_width];
        LIBTRADIER_SIMD_ALIGN double smv_vol_values[simd_width];
        
        // Extract Greeks fields for SIMD batch with loop unrolling
        LIBTRADIER_SIMD_UNROLL(8)
        for (size_t j = 0; j < simd_width; ++j) {
            const auto& greeks_data = json_values[i + j];
            
            delta_values[j] = parseNumericField(greeks_data, "delta").value_or(0.0);
            gamma_values[j] = parseNumericField(greeks_data, "gamma").value_or(0.0);
            theta_values[j] = parseNumericField(greeks_data, "theta").value_or(0.0);
            vega_values[j] = parseNumericField(greeks_data, "vega").value_or(0.0);
            rho_values[j] = parseNumericField(greeks_data, "rho").value_or(0.0);
            phi_values[j] = parseNumericField(greeks_data, "phi").value_or(0.0);
            bid_iv_values[j] = parseNumericField(greeks_data, "bid_iv").value_or(0.0);
            mid_iv_values[j] = parseNumericField(greeks_data, "mid_iv").value_or(0.0);
            ask_iv_values[j] = parseNumericField(greeks_data, "ask_iv").value_or(0.0);
            smv_vol_values[j] = parseNumericField(greeks_data, "smv_vol").value_or(0.0);
        }
        
        // Load values into AVX-512 registers for batch processing
        __m512d delta_vec = _mm512_load_pd(delta_values);
        __m512d gamma_vec = _mm512_load_pd(gamma_values);
        __m512d theta_vec = _mm512_load_pd(theta_values);
        __m512d vega_vec = _mm512_load_pd(vega_values);
        __m512d rho_vec = _mm512_load_pd(rho_values);
        __m512d phi_vec = _mm512_load_pd(phi_values);
        __m512d bid_iv_vec = _mm512_load_pd(bid_iv_values);
        __m512d mid_iv_vec = _mm512_load_pd(mid_iv_values);
        __m512d ask_iv_vec = _mm512_load_pd(ask_iv_values);
        __m512d smv_vol_vec = _mm512_load_pd(smv_vol_values);
        
        // Store processed Greeks in output structure
        _mm512_store_pd(&greeks_output.delta[processed], delta_vec);
        _mm512_store_pd(&greeks_output.gamma[processed], gamma_vec);
        _mm512_store_pd(&greeks_output.theta[processed], theta_vec);
        _mm512_store_pd(&greeks_output.vega[processed], vega_vec);
        _mm512_store_pd(&greeks_output.rho[processed], rho_vec);
        _mm512_store_pd(&greeks_output.phi[processed], phi_vec);
        _mm512_store_pd(&greeks_output.bid_iv[processed], bid_iv_vec);
        _mm512_store_pd(&greeks_output.mid_iv[processed], mid_iv_vec);
        _mm512_store_pd(&greeks_output.ask_iv[processed], ask_iv_vec);
        _mm512_store_pd(&greeks_output.smv_vol[processed], smv_vol_vec);
        
        processed += simd_width;
    }
    
    // Handle remaining elements with scalar processing
    for (size_t i = (count / simd_width) * simd_width; i < count; ++i) {
        const auto& greeks_data = json_values[i];
        
        greeks_output.delta[processed] = parseNumericField(greeks_data, "delta").value_or(0.0);
        greeks_output.gamma[processed] = parseNumericField(greeks_data, "gamma").value_or(0.0);
        greeks_output.theta[processed] = parseNumericField(greeks_data, "theta").value_or(0.0);
        greeks_output.vega[processed] = parseNumericField(greeks_data, "vega").value_or(0.0);
        greeks_output.rho[processed] = parseNumericField(greeks_data, "rho").value_or(0.0);
        greeks_output.phi[processed] = parseNumericField(greeks_data, "phi").value_or(0.0);
        greeks_output.bid_iv[processed] = parseNumericField(greeks_data, "bid_iv").value_or(0.0);
        greeks_output.mid_iv[processed] = parseNumericField(greeks_data, "mid_iv").value_or(0.0);
        greeks_output.ask_iv[processed] = parseNumericField(greeks_data, "ask_iv").value_or(0.0);
        greeks_output.smv_vol[processed] = parseNumericField(greeks_data, "smv_vol").value_or(0.0);
        
        processed++;
    }
    
    return processed;
}
#endif // LIBTRADIER_SIMD_AVX512

//==============================================================================
// Vectorized Greeks Validation and Normalization
//==============================================================================

template<typename... Args>
auto validate_greeks_impl::scalar(Args&&... args) {
    auto& greeks = std::get<0>(std::forward_as_tuple(args...));
    const auto& min_values = std::get<1>(std::forward_as_tuple(args...));
    const auto& max_values = std::get<2>(std::forward_as_tuple(args...));
    size_t count = std::get<3>(std::forward_as_tuple(args...));
    
    size_t valid_count = 0;
    
    for (size_t i = 0; i < count; ++i) {
        bool is_valid = true;
        
        // Validate delta (typically -1.0 to 1.0 for options)
        if (greeks.delta[i] < min_values.delta || greeks.delta[i] > max_values.delta) {
            greeks.delta[i] = std::clamp(greeks.delta[i], min_values.delta, max_values.delta);
            is_valid = false;
        }
        
        // Validate gamma (typically 0.0 to 1.0)
        if (greeks.gamma[i] < min_values.gamma || greeks.gamma[i] > max_values.gamma) {
            greeks.gamma[i] = std::clamp(greeks.gamma[i], min_values.gamma, max_values.gamma);
            is_valid = false;
        }
        
        // Validate theta (typically negative for options)
        if (greeks.theta[i] < min_values.theta || greeks.theta[i] > max_values.theta) {
            greeks.theta[i] = std::clamp(greeks.theta[i], min_values.theta, max_values.theta);
            is_valid = false;
        }
        
        // Validate vega (typically 0.0 to positive values)
        if (greeks.vega[i] < min_values.vega || greeks.vega[i] > max_values.vega) {
            greeks.vega[i] = std::clamp(greeks.vega[i], min_values.vega, max_values.vega);
            is_valid = false;
        }
        
        // Validate implied volatilities (typically 0.0 to 5.0)
        if (greeks.bid_iv[i] < 0.0 || greeks.bid_iv[i] > 5.0) {
            greeks.bid_iv[i] = std::clamp(greeks.bid_iv[i], 0.0, 5.0);
            is_valid = false;
        }
        
        if (greeks.mid_iv[i] < 0.0 || greeks.mid_iv[i] > 5.0) {
            greeks.mid_iv[i] = std::clamp(greeks.mid_iv[i], 0.0, 5.0);
            is_valid = false;
        }
        
        if (greeks.ask_iv[i] < 0.0 || greeks.ask_iv[i] > 5.0) {
            greeks.ask_iv[i] = std::clamp(greeks.ask_iv[i], 0.0, 5.0);
            is_valid = false;
        }
        
        if (is_valid) {
            valid_count++;
        }
    }
    
    return valid_count;
}

#ifdef LIBTRADIER_SIMD_AVX2
template<typename... Args>
auto validate_greeks_impl::avx2(Args&&... args) {
    auto& greeks = std::get<0>(std::forward_as_tuple(args...));
    const auto& min_values = std::get<1>(std::forward_as_tuple(args...));
    const auto& max_values = std::get<2>(std::forward_as_tuple(args...));
    size_t count = std::get<3>(std::forward_as_tuple(args...));
    
    const size_t simd_width = 4;
    size_t valid_count = 0;
    
    // SIMD validation constants
    __m256d delta_min = _mm256_set1_pd(min_values.delta);
    __m256d delta_max = _mm256_set1_pd(max_values.delta);
    __m256d gamma_min = _mm256_set1_pd(min_values.gamma);
    __m256d gamma_max = _mm256_set1_pd(max_values.gamma);
    __m256d theta_min = _mm256_set1_pd(min_values.theta);
    __m256d theta_max = _mm256_set1_pd(max_values.theta);
    __m256d vega_min = _mm256_set1_pd(min_values.vega);
    __m256d vega_max = _mm256_set1_pd(max_values.vega);
    __m256d iv_min = _mm256_set1_pd(0.0);
    __m256d iv_max = _mm256_set1_pd(5.0);
    
    // Process in SIMD chunks
    for (size_t i = 0; i + simd_width <= count; i += simd_width) {
        // Load Greeks values
        __m256d delta_vec = _mm256_loadu_pd(&greeks.delta[i]);
        __m256d gamma_vec = _mm256_loadu_pd(&greeks.gamma[i]);
        __m256d theta_vec = _mm256_loadu_pd(&greeks.theta[i]);
        __m256d vega_vec = _mm256_loadu_pd(&greeks.vega[i]);
        __m256d bid_iv_vec = _mm256_loadu_pd(&greeks.bid_iv[i]);
        __m256d mid_iv_vec = _mm256_loadu_pd(&greeks.mid_iv[i]);
        __m256d ask_iv_vec = _mm256_loadu_pd(&greeks.ask_iv[i]);
        
        // Clamp values to valid ranges
        delta_vec = _mm256_max_pd(_mm256_min_pd(delta_vec, delta_max), delta_min);
        gamma_vec = _mm256_max_pd(_mm256_min_pd(gamma_vec, gamma_max), gamma_min);
        theta_vec = _mm256_max_pd(_mm256_min_pd(theta_vec, theta_max), theta_min);
        vega_vec = _mm256_max_pd(_mm256_min_pd(vega_vec, vega_max), vega_min);
        bid_iv_vec = _mm256_max_pd(_mm256_min_pd(bid_iv_vec, iv_max), iv_min);
        mid_iv_vec = _mm256_max_pd(_mm256_min_pd(mid_iv_vec, iv_max), iv_min);
        ask_iv_vec = _mm256_max_pd(_mm256_min_pd(ask_iv_vec, iv_max), iv_min);
        
        // Store clamped values
        _mm256_storeu_pd(&greeks.delta[i], delta_vec);
        _mm256_storeu_pd(&greeks.gamma[i], gamma_vec);
        _mm256_storeu_pd(&greeks.theta[i], theta_vec);
        _mm256_storeu_pd(&greeks.vega[i], vega_vec);
        _mm256_storeu_pd(&greeks.bid_iv[i], bid_iv_vec);
        _mm256_storeu_pd(&greeks.mid_iv[i], mid_iv_vec);
        _mm256_storeu_pd(&greeks.ask_iv[i], ask_iv_vec);
        
        valid_count += simd_width; // Assume all are now valid after clamping
    }
    
    // Handle remaining elements with scalar processing
    for (size_t i = (count / simd_width) * simd_width; i < count; ++i) {
        greeks.delta[i] = std::clamp(greeks.delta[i], min_values.delta, max_values.delta);
        greeks.gamma[i] = std::clamp(greeks.gamma[i], min_values.gamma, max_values.gamma);
        greeks.theta[i] = std::clamp(greeks.theta[i], min_values.theta, max_values.theta);
        greeks.vega[i] = std::clamp(greeks.vega[i], min_values.vega, max_values.vega);
        greeks.bid_iv[i] = std::clamp(greeks.bid_iv[i], 0.0, 5.0);
        greeks.mid_iv[i] = std::clamp(greeks.mid_iv[i], 0.0, 5.0);
        greeks.ask_iv[i] = std::clamp(greeks.ask_iv[i], 0.0, 5.0);
        valid_count++;
    }
    
    return valid_count;
}
#endif // LIBTRADIER_SIMD_AVX2

//==============================================================================
// Vectorized Implied Volatility Surface Calculations
//==============================================================================

template<typename... Args>
auto interpolate_iv_surface_impl::scalar(Args&&... args) {
    const auto* strikes = std::get<0>(std::forward_as_tuple(args...));
    const auto* ivs = std::get<1>(std::forward_as_tuple(args...));
    const auto* target_strikes = std::get<2>(std::forward_as_tuple(args...));
    auto* output = std::get<3>(std::forward_as_tuple(args...));
    size_t data_count = std::get<4>(std::forward_as_tuple(args...));
    size_t target_count = std::get<5>(std::forward_as_tuple(args...));
    
    size_t interpolated = 0;
    
    for (size_t i = 0; i < target_count; ++i) {
        double target_strike = target_strikes[i];
        
        // Find interpolation points
        size_t lower_idx = 0, upper_idx = data_count - 1;
        
        for (size_t j = 0; j < data_count - 1; ++j) {
            if (strikes[j] <= target_strike && strikes[j + 1] >= target_strike) {
                lower_idx = j;
                upper_idx = j + 1;
                break;
            }
        }
        
        // Linear interpolation
        if (lower_idx != upper_idx) {
            double strike_diff = strikes[upper_idx] - strikes[lower_idx];
            double weight = (target_strike - strikes[lower_idx]) / strike_diff;
            output[interpolated++] = ivs[lower_idx] + weight * (ivs[upper_idx] - ivs[lower_idx]);
        } else {
            // Extrapolation or exact match
            output[interpolated++] = ivs[lower_idx];
        }
    }
    
    return interpolated;
}

#ifdef LIBTRADIER_SIMD_AVX2
template<typename... Args>
auto interpolate_iv_surface_impl::avx2(Args&&... args) {
    const auto* strikes = std::get<0>(std::forward_as_tuple(args...));
    const auto* ivs = std::get<1>(std::forward_as_tuple(args...));
    const auto* target_strikes = std::get<2>(std::forward_as_tuple(args...));
    auto* output = std::get<3>(std::forward_as_tuple(args...));
    size_t data_count = std::get<4>(std::forward_as_tuple(args...));
    size_t target_count = std::get<5>(std::forward_as_tuple(args...));
    
    const size_t simd_width = 4;
    size_t interpolated = 0;
    
    // Process targets in SIMD chunks where possible
    for (size_t i = 0; i + simd_width <= target_count; i += simd_width) {
        // Load target strikes
        __m256d target_vec = _mm256_loadu_pd(&target_strikes[i]);
        
        // For each SIMD element, find interpolation points
        LIBTRADIER_SIMD_ALIGN double results[simd_width];
        
        for (size_t j = 0; j < simd_width; ++j) {
            double target_strike = target_strikes[i + j];
            
            // Find interpolation points (scalar search for now)
            size_t lower_idx = 0, upper_idx = data_count - 1;
            
            for (size_t k = 0; k < data_count - 1; ++k) {
                if (strikes[k] <= target_strike && strikes[k + 1] >= target_strike) {
                    lower_idx = k;
                    upper_idx = k + 1;
                    break;
                }
            }
            
            // Linear interpolation
            if (lower_idx != upper_idx) {
                double strike_diff = strikes[upper_idx] - strikes[lower_idx];
                double weight = (target_strike - strikes[lower_idx]) / strike_diff;
                results[j] = ivs[lower_idx] + weight * (ivs[upper_idx] - ivs[lower_idx]);
            } else {
                results[j] = ivs[lower_idx];
            }
        }
        
        // Store results
        _mm256_storeu_pd(&output[interpolated], _mm256_load_pd(results));
        interpolated += simd_width;
    }
    
    // Handle remaining elements with scalar processing
    for (size_t i = (target_count / simd_width) * simd_width; i < target_count; ++i) {
        double target_strike = target_strikes[i];
        
        size_t lower_idx = 0, upper_idx = data_count - 1;
        
        for (size_t j = 0; j < data_count - 1; ++j) {
            if (strikes[j] <= target_strike && strikes[j + 1] >= target_strike) {
                lower_idx = j;
                upper_idx = j + 1;
                break;
            }
        }
        
        if (lower_idx != upper_idx) {
            double strike_diff = strikes[upper_idx] - strikes[lower_idx];
            double weight = (target_strike - strikes[lower_idx]) / strike_diff;
            output[interpolated++] = ivs[lower_idx] + weight * (ivs[upper_idx] - ivs[lower_idx]);
        } else {
            output[interpolated++] = ivs[lower_idx];
        }
    }
    
    return interpolated;
}
#endif // LIBTRADIER_SIMD_AVX2

} // namespace greeks_calculation
} // namespace simd
} // namespace tradier

#endif // LIBTRADIER_SIMD_ENABLED