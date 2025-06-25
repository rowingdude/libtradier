#pragma once

/*
 * SIMD Streaming Operations
 * 
 * High-performance streaming event processing with SIMD acceleration.
 */

#include "simd_traits.hpp"
#include <nlohmann/json.hpp>
#include <vector>
#include <string>

namespace tradier {
namespace simd {
namespace streaming {

// Implementation struct for bulk_string_to_double
struct bulk_string_to_double_impl {
    static size_t scalar(const nlohmann::json* json_strings, double* output, size_t count);
    
#if LIBTRADIER_SIMD_AVX2_AVAILABLE
    static size_t avx2(const nlohmann::json* json_strings, double* output, size_t count);
#endif

#if LIBTRADIER_SIMD_AVX512_AVAILABLE
    static size_t avx512(const nlohmann::json* json_strings, double* output, size_t count);
#endif
};

/**
 * @brief Vectorized bulk string-to-double conversion
 * 
 * Converts arrays of JSON string values to doubles using SIMD operations.
 * 
 * @param json_strings Array of JSON values containing numeric strings
 * @param output Pre-allocated output array for doubles
 * @param count Number of strings to convert
 * @return Number of successfully converted values
 */
LIBTRADIER_SIMD_FUNCTION_VARIANTS(bulk_string_to_double);

} // namespace streaming
} // namespace simd
} // namespace tradier