#pragma once

/*
 * SIMD Traits and Type System
 * 
 * Provides type traits and utilities for SIMD operations.
 */

#include "simd_config.hpp"
#include <cstddef>
#include <type_traits>
#include <utility>

namespace tradier {
namespace simd {

// SIMD width traits
template<typename T>
struct simd_width {
    static constexpr size_t value = 1; // Scalar fallback
};

template<>
struct simd_width<double> {
    static constexpr size_t value = LIBTRADIER_SIMD_DOUBLE_WIDTH;
};

template<>
struct simd_width<float> {
    static constexpr size_t value = LIBTRADIER_SIMD_FLOAT_WIDTH;
};

template<typename T>
inline constexpr size_t simd_width_v = simd_width<T>::value;

// SIMD implementation dispatcher
template<typename ImplStruct>
struct simd_dispatcher {
    template<typename... Args>
    static auto dispatch(Args&&... args) {
#if LIBTRADIER_SIMD_ACTIVE
        #if LIBTRADIER_SIMD_AVX512_AVAILABLE
            return ImplStruct::avx512(std::forward<Args>(args)...);
        #elif LIBTRADIER_SIMD_AVX2_AVAILABLE
            return ImplStruct::avx2(std::forward<Args>(args)...);
        #else
            return ImplStruct::scalar(std::forward<Args>(args)...);
        #endif
#else
        return ImplStruct::scalar(std::forward<Args>(args)...);
#endif
    }
};

// Function variant macro - simplified and safe
#define LIBTRADIER_SIMD_FUNCTION_VARIANTS(name) \
    template<typename... Args> \
    auto name(Args&&... args) { \
        return simd_dispatcher<name##_impl>::dispatch(std::forward<Args>(args)...); \
    }

} // namespace simd
} // namespace tradier