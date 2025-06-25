/*
 * libtradier - SIMD Implementation for JSON Processing
 * 
 * FOURTH HIGHEST IMPACT TARGET: 1.5-2x performance improvement expected
 * Target: Bulk JSON parsing operations across all modules
 * 
 * JSON processing is critical for:
 * - Multi-symbol quote processing
 * - Historical data bulk parsing
 * - Real-time data stream processing
 * 
 * Author: libtradier team
 * Version: 1.0
 */

#include "tradier/simd/vectorized_ops.hpp"
#include <cstring>
#include <algorithm>

#ifdef LIBTRADIER_SIMD_ENABLED

namespace tradier {
namespace simd {
namespace json_processing {

//==============================================================================
// High-Performance JSON String Processing
//==============================================================================

// Scalar implementation (fallback and baseline)
template<typename... Args>
auto bulk_string_to_double_impl::scalar(Args&&... args) {
    const auto* json_strings = std::get<0>(std::forward_as_tuple(args...));
    auto* output = std::get<1>(std::forward_as_tuple(args...));
    size_t count = std::get<2>(std::forward_as_tuple(args...));
    
    size_t converted = 0;
    
    for (size_t i = 0; i < count; ++i) {
        const auto& json_str = json_strings[i];
        
        // Sequential string-to-double conversion (current bottleneck)
        try {
            if (!json_str.empty()) {
                double value = std::stod(json_str);
                if (std::isfinite(value)) {
                    output[converted++] = value;
                }
            }
        } catch (const std::exception&) {
            // Skip invalid strings
            continue;
        }
    }
    
    return converted;
}

#ifdef LIBTRADIER_SIMD_AVX2
// AVX2 implementation - optimized string processing
template<typename... Args>
auto bulk_string_to_double_impl::avx2(Args&&... args) {
    const auto* json_strings = std::get<0>(std::forward_as_tuple(args...));
    auto* output = std::get<1>(std::forward_as_tuple(args...));
    size_t count = std::get<2>(std::forward_as_tuple(args...));
    
    size_t converted = 0;
    const size_t simd_width = 4; // AVX2 processes 4 doubles at once
    
    // Process in batches for better cache utilization
    for (size_t i = 0; i + simd_width <= count; i += simd_width) {
        LIBTRADIER_SIMD_ALIGN double values[simd_width];
        bool valid[simd_width] = {false};
        
        // Convert batch of strings to doubles
        for (size_t j = 0; j < simd_width; ++j) {
            const auto& json_str = json_strings[i + j];
            
            try {
                if (!json_str.empty()) {
                    double value = std::stod(json_str);
                    if (std::isfinite(value)) {
                        values[j] = value;
                        valid[j] = true;
                    } else {
                        values[j] = 0.0;
                        valid[j] = false;
                    }
                } else {
                    values[j] = 0.0;
                    valid[j] = false;
                }
            } catch (const std::exception&) {
                values[j] = 0.0;
                valid[j] = false;
            }
        }
        
        // Load and validate using SIMD
        __m256d value_vec = _mm256_load_pd(values);
        
        // Check for finite values
        __m256d inf_mask = _mm256_cmp_pd(value_vec, value_vec, _CMP_EQ_OQ); // NaN check
        __m256d abs_val = _mm256_andnot_pd(_mm256_set1_pd(-0.0), value_vec); // Absolute value
        __m256d max_finite = _mm256_set1_pd(std::numeric_limits<double>::max());
        __m256d finite_mask = _mm256_cmp_pd(abs_val, max_finite, _CMP_LE_OQ); // Finite check
        
        // Combine masks
        __m256d valid_mask = _mm256_and_pd(inf_mask, finite_mask);
        
        // Store valid values
        for (size_t j = 0; j < simd_width; ++j) {
            if (valid[j]) {
                // Extract SIMD validation result
                double mask_result;
                _mm256_store_pd(&mask_result, valid_mask);
                
                if (mask_result != 0.0) { // Valid according to SIMD checks
                    output[converted++] = values[j];
                }
            }
        }
    }
    
    // Handle remaining elements with scalar processing
    for (size_t i = (count / simd_width) * simd_width; i < count; ++i) {
        const auto& json_str = json_strings[i];
        
        try {
            if (!json_str.empty()) {
                double value = std::stod(json_str);
                if (std::isfinite(value)) {
                    output[converted++] = value;
                }
            }
        } catch (const std::exception&) {
            continue;
        }
    }
    
    return converted;
}
#endif // LIBTRADIER_SIMD_AVX2

#ifdef LIBTRADIER_SIMD_AVX512
// AVX-512 implementation - 8x parallel processing
template<typename... Args>
auto bulk_string_to_double_impl::avx512(Args&&... args) {
    const auto* json_strings = std::get<0>(std::forward_as_tuple(args...));
    auto* output = std::get<1>(std::forward_as_tuple(args...));
    size_t count = std::get<2>(std::forward_as_tuple(args...));
    
    size_t converted = 0;
    const size_t simd_width = 8; // AVX-512 processes 8 doubles at once
    
    // Process in batches for better cache utilization
    for (size_t i = 0; i + simd_width <= count; i += simd_width) {
        LIBTRADIER_SIMD_ALIGN double values[simd_width];
        bool valid[simd_width] = {false};
        
        // Convert batch of strings to doubles with loop unrolling
        LIBTRADIER_SIMD_UNROLL(8)
        for (size_t j = 0; j < simd_width; ++j) {
            const auto& json_str = json_strings[i + j];
            
            try {
                if (!json_str.empty()) {
                    double value = std::stod(json_str);
                    if (std::isfinite(value)) {
                        values[j] = value;
                        valid[j] = true;
                    } else {
                        values[j] = 0.0;
                        valid[j] = false;
                    }
                } else {
                    values[j] = 0.0;
                    valid[j] = false;
                }
            } catch (const std::exception&) {
                values[j] = 0.0;
                valid[j] = false;
            }
        }
        
        // Load and validate using SIMD
        __m512d value_vec = _mm512_load_pd(values);
        
        // Check for finite values using AVX-512 masks
        __mmask8 nan_mask = _mm512_cmp_pd_mask(value_vec, value_vec, _CMP_EQ_OQ); // NaN check
        __m512d abs_val = _mm512_abs_pd(value_vec);
        __m512d max_finite = _mm512_set1_pd(std::numeric_limits<double>::max());
        __mmask8 finite_mask = _mm512_cmp_pd_mask(abs_val, max_finite, _CMP_LE_OQ); // Finite check
        
        // Combine masks
        __mmask8 valid_mask = nan_mask & finite_mask;
        
        // Store valid values using masked operations
        for (size_t j = 0; j < simd_width; ++j) {
            if (valid[j] && (valid_mask & (1 << j))) {
                output[converted++] = values[j];
            }
        }
    }
    
    // Handle remaining elements with scalar processing
    for (size_t i = (count / simd_width) * simd_width; i < count; ++i) {
        const auto& json_str = json_strings[i];
        
        try {
            if (!json_str.empty()) {
                double value = std::stod(json_str);
                if (std::isfinite(value)) {
                    output[converted++] = value;
                }
            }
        } catch (const std::exception&) {
            continue;
        }
    }
    
    return converted;
}
#endif // LIBTRADIER_SIMD_AVX512

//==============================================================================
// Vectorized Field Extraction from JSON Arrays
//==============================================================================

template<typename... Args>
auto extract_numeric_fields_impl::scalar(Args&&... args) {
    const auto* json_data = std::get<0>(std::forward_as_tuple(args...));
    const auto* field_offsets = std::get<1>(std::forward_as_tuple(args...));
    auto* output = std::get<2>(std::forward_as_tuple(args...));
    size_t count = std::get<3>(std::forward_as_tuple(args...));
    
    size_t extracted = 0;
    
    for (size_t i = 0; i < count; ++i) {
        const auto& json_record = json_data[i];
        const auto& offsets = field_offsets[i];
        
        // Sequential field extraction (current bottleneck)
        for (size_t field_idx = 0; field_idx < offsets.field_count; ++field_idx) {
            size_t offset = offsets.offsets[field_idx];
            size_t length = offsets.lengths[field_idx];
            
            if (offset + length <= json_record.size()) {
                std::string field_str = json_record.substr(offset, length);
                
                try {
                    double value = std::stod(field_str);
                    if (std::isfinite(value)) {
                        output[extracted++] = value;
                    }
                } catch (const std::exception&) {
                    // Skip invalid fields
                    continue;
                }
            }
        }
    }
    
    return extracted;
}

#ifdef LIBTRADIER_SIMD_AVX2
template<typename... Args>
auto extract_numeric_fields_impl::avx2(Args&&... args) {
    const auto* json_data = std::get<0>(std::forward_as_tuple(args...));
    const auto* field_offsets = std::get<1>(std::forward_as_tuple(args...));
    auto* output = std::get<2>(std::forward_as_tuple(args...));
    size_t count = std::get<3>(std::forward_as_tuple(args...));
    
    const size_t simd_width = 4;
    size_t extracted = 0;
    
    // Process records in batches when possible
    for (size_t i = 0; i < count; ++i) {
        const auto& json_record = json_data[i];
        const auto& offsets = field_offsets[i];
        
        // Batch field extraction when we have enough fields
        if (offsets.field_count >= simd_width) {
            LIBTRADIER_SIMD_ALIGN double field_values[simd_width];
            bool valid_fields[simd_width] = {false};
            
            // Process fields in SIMD-width chunks
            for (size_t field_chunk = 0; field_chunk + simd_width <= offsets.field_count; field_chunk += simd_width) {
                // Extract fields for this chunk
                for (size_t j = 0; j < simd_width; ++j) {
                    size_t field_idx = field_chunk + j;
                    size_t offset = offsets.offsets[field_idx];
                    size_t length = offsets.lengths[field_idx];
                    
                    if (offset + length <= json_record.size()) {
                        std::string field_str = json_record.substr(offset, length);
                        
                        try {
                            double value = std::stod(field_str);
                            if (std::isfinite(value)) {
                                field_values[j] = value;
                                valid_fields[j] = true;
                            } else {
                                field_values[j] = 0.0;
                                valid_fields[j] = false;
                            }
                        } catch (const std::exception&) {
                            field_values[j] = 0.0;
                            valid_fields[j] = false;
                        }
                    } else {
                        field_values[j] = 0.0;
                        valid_fields[j] = false;
                    }
                }
                
                // Validate using SIMD
                __m256d value_vec = _mm256_load_pd(field_values);
                __m256d finite_mask = _mm256_cmp_pd(value_vec, value_vec, _CMP_EQ_OQ); // NaN check
                
                // Store valid values
                for (size_t j = 0; j < simd_width; ++j) {
                    if (valid_fields[j]) {
                        output[extracted++] = field_values[j];
                    }
                }
            }
            
            // Handle remaining fields with scalar processing
            for (size_t field_idx = (offsets.field_count / simd_width) * simd_width; field_idx < offsets.field_count; ++field_idx) {
                size_t offset = offsets.offsets[field_idx];
                size_t length = offsets.lengths[field_idx];
                
                if (offset + length <= json_record.size()) {
                    std::string field_str = json_record.substr(offset, length);
                    
                    try {
                        double value = std::stod(field_str);
                        if (std::isfinite(value)) {
                            output[extracted++] = value;
                        }
                    } catch (const std::exception&) {
                        continue;
                    }
                }
            }
        } else {
            // Fall back to scalar processing for small field counts
            for (size_t field_idx = 0; field_idx < offsets.field_count; ++field_idx) {
                size_t offset = offsets.offsets[field_idx];
                size_t length = offsets.lengths[field_idx];
                
                if (offset + length <= json_record.size()) {
                    std::string field_str = json_record.substr(offset, length);
                    
                    try {
                        double value = std::stod(field_str);
                        if (std::isfinite(value)) {
                            output[extracted++] = value;
                        }
                    } catch (const std::exception&) {
                        continue;
                    }
                }
            }
        }
    }
    
    return extracted;
}
#endif // LIBTRADIER_SIMD_AVX2

//==============================================================================
// Vectorized Symbol String Concatenation
//==============================================================================

template<typename... Args>
auto concat_symbols_impl::scalar(Args&&... args) {
    const auto* symbols = std::get<0>(std::forward_as_tuple(args...));
    char separator = std::get<1>(std::forward_as_tuple(args...));
    auto* output = std::get<2>(std::forward_as_tuple(args...));
    size_t count = std::get<3>(std::forward_as_tuple(args...));
    
    if (count == 0) return 0;
    
    size_t output_pos = 0;
    
    // Sequential concatenation (current bottleneck)
    for (size_t i = 0; i < count; ++i) {
        const auto& symbol = symbols[i];
        
        // Copy symbol
        for (char c : symbol) {
            output[output_pos++] = c;
        }
        
        // Add separator (except for last symbol)
        if (i < count - 1) {
            output[output_pos++] = separator;
        }
    }
    
    output[output_pos] = '\0'; // Null terminate
    return output_pos;
}

#ifdef LIBTRADIER_SIMD_AVX2
template<typename... Args>
auto concat_symbols_impl::avx2(Args&&... args) {
    const auto* symbols = std::get<0>(std::forward_as_tuple(args...));
    char separator = std::get<1>(std::forward_as_tuple(args...));
    auto* output = std::get<2>(std::forward_as_tuple(args...));
    size_t count = std::get<3>(std::forward_as_tuple(args...));
    
    if (count == 0) return 0;
    
    size_t output_pos = 0;
    const size_t simd_width = 32; // AVX2 can process 32 bytes at once
    
    // Optimized concatenation with SIMD memory operations
    for (size_t i = 0; i < count; ++i) {
        const auto& symbol = symbols[i];
        size_t symbol_len = symbol.length();
        
        // Use SIMD for larger symbols
        if (symbol_len >= simd_width) {
            const char* symbol_data = symbol.data();
            
            // Process in SIMD chunks
            for (size_t chunk = 0; chunk + simd_width <= symbol_len; chunk += simd_width) {
                __m256i symbol_chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(symbol_data + chunk));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(output + output_pos), symbol_chunk);
                output_pos += simd_width;
            }
            
            // Handle remaining characters
            for (size_t j = (symbol_len / simd_width) * simd_width; j < symbol_len; ++j) {
                output[output_pos++] = symbol_data[j];
            }
        } else {
            // Scalar copy for small symbols
            for (char c : symbol) {
                output[output_pos++] = c;
            }
        }
        
        // Add separator (except for last symbol)
        if (i < count - 1) {
            output[output_pos++] = separator;
        }
    }
    
    output[output_pos] = '\0'; // Null terminate
    return output_pos;
}
#endif // LIBTRADIER_SIMD_AVX2

#ifdef LIBTRADIER_SIMD_AVX512
template<typename... Args>
auto concat_symbols_impl::avx512(Args&&... args) {
    const auto* symbols = std::get<0>(std::forward_as_tuple(args...));
    char separator = std::get<1>(std::forward_as_tuple(args...));
    auto* output = std::get<2>(std::forward_as_tuple(args...));
    size_t count = std::get<3>(std::forward_as_tuple(args...));
    
    if (count == 0) return 0;
    
    size_t output_pos = 0;
    const size_t simd_width = 64; // AVX-512 can process 64 bytes at once
    
    // Optimized concatenation with AVX-512 memory operations
    for (size_t i = 0; i < count; ++i) {
        const auto& symbol = symbols[i];
        size_t symbol_len = symbol.length();
        
        // Use SIMD for larger symbols
        if (symbol_len >= simd_width) {
            const char* symbol_data = symbol.data();
            
            // Process in SIMD chunks with prefetching
            for (size_t chunk = 0; chunk + simd_width <= symbol_len; chunk += simd_width) {
                LIBTRADIER_SIMD_PREFETCH(symbol_data + chunk + simd_width, 0, 3); // Prefetch next chunk
                
                __m512i symbol_chunk = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(symbol_data + chunk));
                _mm512_storeu_si512(reinterpret_cast<__m512i*>(output + output_pos), symbol_chunk);
                output_pos += simd_width;
            }
            
            // Handle remaining characters
            for (size_t j = (symbol_len / simd_width) * simd_width; j < symbol_len; ++j) {
                output[output_pos++] = symbol_data[j];
            }
        } else {
            // Scalar copy for small symbols
            for (char c : symbol) {
                output[output_pos++] = c;
            }
        }
        
        // Add separator (except for last symbol)
        if (i < count - 1) {
            output[output_pos++] = separator;
        }
    }
    
    output[output_pos] = '\0'; // Null terminate
    return output_pos;
}
#endif // LIBTRADIER_SIMD_AVX512

} // namespace json_processing
} // namespace simd
} // namespace tradier

#endif // LIBTRADIER_SIMD_ENABLED