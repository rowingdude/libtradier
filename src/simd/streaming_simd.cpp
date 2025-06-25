/*
 * SIMD Streaming Implementation
 */

#include "tradier/simd/streaming_simd.hpp"
#include <algorithm>
#include <charconv>
#include <cstring>

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

} // namespace streaming
} // namespace simd
} // namespace tradier