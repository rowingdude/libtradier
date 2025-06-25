/*
 * libtradier - SIMD Traits and Type Definitions
 * 
 * Compile-time traits and types for SIMD vectorization
 * Provides high-level interface for SIMD operations
 * 
 * Author: libtradier team
 * Version: 1.0
 */

#pragma once

#include "tradier/simd_config.h"
#include <type_traits>
#include <cstddef>
#include <cstdint>
#include <array>

#ifdef LIBTRADIER_SIMD_ENABLED

// Include appropriate SIMD headers based on configuration
#ifdef LIBTRADIER_SIMD_AVX512
    #include <immintrin.h>
#elif defined(LIBTRADIER_SIMD_AVX2) || defined(LIBTRADIER_SIMD_AVX)
    #include <immintrin.h>
#elif defined(LIBTRADIER_SIMD_SSE41)
    #include <smmintrin.h>
#endif

#endif // LIBTRADIER_SIMD_ENABLED

namespace tradier {
namespace simd {

// Helper macros for SIMD feature detection in constexpr functions
#ifdef LIBTRADIER_SIMD_SSE41
    #define LIBTRADIER_SIMD_HAS_SSE41 return true;
#else
    #define LIBTRADIER_SIMD_HAS_SSE41 return false;
#endif

#ifdef LIBTRADIER_SIMD_AVX
    #define LIBTRADIER_SIMD_HAS_AVX return true;
#else
    #define LIBTRADIER_SIMD_HAS_AVX return false;
#endif

#ifdef LIBTRADIER_SIMD_AVX2
    #define LIBTRADIER_SIMD_HAS_AVX2 return true;
#else
    #define LIBTRADIER_SIMD_HAS_AVX2 return false;
#endif

#ifdef LIBTRADIER_SIMD_AVX512
    #define LIBTRADIER_SIMD_HAS_AVX512 return true;
#else
    #define LIBTRADIER_SIMD_HAS_AVX512 return false;
#endif

// Forward declarations
template<typename T>
struct simd_traits;

// SIMD vector types for different data types
template<typename T>
struct vector_type;

// Double precision specialization
template<>
struct vector_type<double> {
#ifdef LIBTRADIER_SIMD_AVX512
    using type = __m512d;
    static constexpr size_t width = 8;
    static constexpr size_t alignment = 64;
#elif defined(LIBTRADIER_SIMD_AVX2) || defined(LIBTRADIER_SIMD_AVX)
    using type = __m256d;
    static constexpr size_t width = 4;
    static constexpr size_t alignment = 32;
#elif defined(LIBTRADIER_SIMD_SSE41)
    using type = __m128d;
    static constexpr size_t width = 2;
    static constexpr size_t alignment = 16;
#else
    using type = double;
    static constexpr size_t width = 1;
    static constexpr size_t alignment = 8;
#endif
    
    static constexpr bool is_vectorized = width > 1;
    static constexpr const char* instruction_set = LIBTRADIER_SIMD_PREFERRED;
};

// Single precision specialization
template<>
struct vector_type<float> {
#ifdef LIBTRADIER_SIMD_AVX512
    using type = __m512;
    static constexpr size_t width = 16;
    static constexpr size_t alignment = 64;
#elif defined(LIBTRADIER_SIMD_AVX2) || defined(LIBTRADIER_SIMD_AVX)
    using type = __m256;
    static constexpr size_t width = 8;
    static constexpr size_t alignment = 32;
#elif defined(LIBTRADIER_SIMD_SSE41)
    using type = __m128;
    static constexpr size_t width = 4;
    static constexpr size_t alignment = 16;
#else
    using type = float;
    static constexpr size_t width = 1;
    static constexpr size_t alignment = 4;
#endif
    
    static constexpr bool is_vectorized = width > 1;
    static constexpr const char* instruction_set = LIBTRADIER_SIMD_PREFERRED;
};

// Integer specialization (for indices and counts)
template<>
struct vector_type<int32_t> {
#ifdef LIBTRADIER_SIMD_AVX512
    using type = __m512i;
    static constexpr size_t width = 16;
    static constexpr size_t alignment = 64;
#elif defined(LIBTRADIER_SIMD_AVX2)
    using type = __m256i;
    static constexpr size_t width = 8;
    static constexpr size_t alignment = 32;
#elif defined(LIBTRADIER_SIMD_SSE41)
    using type = __m128i;
    static constexpr size_t width = 4;
    static constexpr size_t alignment = 16;
#else
    using type = int32_t;
    static constexpr size_t width = 1;
    static constexpr size_t alignment = 4;
#endif
    
    static constexpr bool is_vectorized = width > 1;
    static constexpr const char* instruction_set = LIBTRADIER_SIMD_PREFERRED;
};

// Convenience aliases
template<typename T>
using simd_vector_t = typename vector_type<T>::type;

template<typename T>
constexpr size_t simd_width_v = vector_type<T>::width;

template<typename T>
constexpr size_t simd_alignment_v = vector_type<T>::alignment;

template<typename T>
constexpr bool is_simd_vectorized_v = vector_type<T>::is_vectorized;

// SIMD-aligned array type
template<typename T, size_t N>
using simd_array = std::array<T, N>;

// Memory alignment utilities
template<typename T>
constexpr bool is_simd_aligned(const void* ptr) noexcept {
    return (reinterpret_cast<uintptr_t>(ptr) % simd_alignment_v<T>) == 0;
}

template<typename T>
constexpr size_t simd_aligned_size(size_t count) noexcept {
    const size_t width = simd_width_v<T>;
    return ((count + width - 1) / width) * width;
}

// SIMD operation traits
template<typename Operation, typename T>
struct simd_operation_traits {
    static constexpr bool is_supported = false;
    static constexpr bool is_optimized = false;
    static constexpr size_t expected_speedup = 1;
};

// Financial computation operation tags
struct addition_tag {};
struct subtraction_tag {};
struct multiplication_tag {};
struct division_tag {};
struct fused_multiply_add_tag {};
struct absolute_value_tag {};
struct min_max_tag {};
struct comparison_tag {};
struct conversion_tag {};

// Specializations for double precision financial operations
template<>
struct simd_operation_traits<addition_tag, double> {
    static constexpr bool is_supported = true;
#ifdef LIBTRADIER_SIMD_AVX512
    static constexpr bool is_optimized = true;
    static constexpr size_t expected_speedup = 8;
#elif defined(LIBTRADIER_SIMD_AVX2) || defined(LIBTRADIER_SIMD_AVX)
    static constexpr bool is_optimized = true;
    static constexpr size_t expected_speedup = 4;
#elif defined(LIBTRADIER_SIMD_SSE41)
    static constexpr bool is_optimized = true;
    static constexpr size_t expected_speedup = 2;
#else
    static constexpr bool is_optimized = false;
    static constexpr size_t expected_speedup = 1;
#endif
};

template<>
struct simd_operation_traits<fused_multiply_add_tag, double> {
    static constexpr bool is_supported = true;
#if defined(LIBTRADIER_SIMD_FMA) && defined(LIBTRADIER_SIMD_AVX512)
    static constexpr bool is_optimized = true;
    static constexpr size_t expected_speedup = 16; // 8 elements * 2 ops
#elif defined(LIBTRADIER_SIMD_FMA) && (defined(LIBTRADIER_SIMD_AVX2) || defined(LIBTRADIER_SIMD_AVX))
    static constexpr bool is_optimized = true;
    static constexpr size_t expected_speedup = 8;  // 4 elements * 2 ops
#else
    static constexpr bool is_optimized = false;
    static constexpr size_t expected_speedup = 1;
#endif
};

// Runtime CPU feature detection
#ifdef LIBTRADIER_SIMD_RUNTIME_DETECTION

struct cpu_features {
    bool sse41 : 1;
    bool avx : 1;
    bool avx2 : 1;
    bool fma : 1;
    bool avx512f : 1;
    bool avx512dq : 1;
    bool avx512bw : 1;
    bool avx512vl : 1;
    bool f16c : 1;
    
    cpu_features() noexcept;
    
    static const cpu_features& instance() noexcept {
        static const cpu_features features;
        return features;
    }
    
    bool supports_vectorization() const noexcept {
        return sse41 || avx || avx2 || avx512f;
    }
    
    const char* best_instruction_set() const noexcept {
        if (avx512f) return "AVX512";
        if (avx2) return "AVX2";
        if (avx) return "AVX";
        if (sse41) return "SSE41";
        return "SCALAR";
    }
};

// Function to get runtime CPU features
LIBTRADIER_SIMD_INLINE const cpu_features& get_cpu_features() noexcept {
    return cpu_features::instance();
}

#endif // LIBTRADIER_SIMD_RUNTIME_DETECTION

// Performance hint macros
#define LIBTRADIER_SIMD_LIKELY(x)   __builtin_expect(!!(x), 1)
#define LIBTRADIER_SIMD_UNLIKELY(x) __builtin_expect(!!(x), 0)

// Loop unrolling hints
#if defined(__clang__)
    #define LIBTRADIER_SIMD_UNROLL(n) _Pragma("clang loop unroll_count(" #n ")")
    #define LIBTRADIER_SIMD_VECTORIZE _Pragma("clang loop vectorize(enable)")
#elif defined(__GNUC__)
    #define LIBTRADIER_SIMD_UNROLL(n) _Pragma("GCC unroll " #n)
    #define LIBTRADIER_SIMD_VECTORIZE _Pragma("GCC ivdep")
#else
    #define LIBTRADIER_SIMD_UNROLL(n)
    #define LIBTRADIER_SIMD_VECTORIZE
#endif

// Memory prefetch hints
#if defined(__GNUC__) || defined(__clang__)
    #define LIBTRADIER_SIMD_PREFETCH(addr, rw, locality) __builtin_prefetch(addr, rw, locality)
#else
    #define LIBTRADIER_SIMD_PREFETCH(addr, rw, locality)
#endif

// SIMD function dispatch mechanism
#ifdef LIBTRADIER_SIMD_RUNTIME_DETECTION

template<typename Func>
struct simd_dispatcher {
    template<typename... Args>
    static auto dispatch(Args&&... args) -> decltype(Func::scalar(std::forward<Args>(args)...)) {
        const auto& features = get_cpu_features();
        
#ifdef LIBTRADIER_SIMD_AVX512
        if (features.avx512f && Func::has_avx512()) {
            return Func::avx512(std::forward<Args>(args)...);
        }
#endif
        
#ifdef LIBTRADIER_SIMD_AVX2
        if (features.avx2 && Func::has_avx2()) {
            return Func::avx2(std::forward<Args>(args)...);
        }
#endif
        
#ifdef LIBTRADIER_SIMD_AVX
        if (features.avx && Func::has_avx()) {
            return Func::avx(std::forward<Args>(args)...);
        }
#endif
        
#ifdef LIBTRADIER_SIMD_SSE41
        if (features.sse41 && Func::has_sse41()) {
            return Func::sse41(std::forward<Args>(args)...);
        }
#endif
        
        return Func::scalar(std::forward<Args>(args)...);
    }
};

// Macro to define SIMD function variants
#define LIBTRADIER_SIMD_FUNCTION_VARIANTS(name) \
    struct name##_impl { \
        static constexpr bool has_scalar() { return true; } \
        static constexpr bool has_sse41() { \
            LIBTRADIER_SIMD_HAS_SSE41 \
        } \
        static constexpr bool has_avx() { \
            LIBTRADIER_SIMD_HAS_AVX \
        } \
        static constexpr bool has_avx2() { \
            LIBTRADIER_SIMD_HAS_AVX2 \
        } \
        static constexpr bool has_avx512() { \
            LIBTRADIER_SIMD_HAS_AVX512 \
        } \
        \
        template<typename... Args> static auto scalar(Args&&... args); \
        template<typename... Args> static auto sse41(Args&&... args); \
        template<typename... Args> static auto avx(Args&&... args); \
        template<typename... Args> static auto avx2(Args&&... args); \
        template<typename... Args> static auto avx512(Args&&... args); \
    }; \
    \
    template<typename... Args> \
    auto name(Args&&... args) { \
        return simd_dispatcher<name##_impl>::dispatch(std::forward<Args>(args)...); \
    }

#else // !LIBTRADIER_SIMD_RUNTIME_DETECTION

// Static dispatch when runtime detection is disabled
#define LIBTRADIER_SIMD_FUNCTION_VARIANTS(name) \
    template<typename... Args> \
    auto name(Args&&... args) { \
        return name##_impl::LIBTRADIER_SIMD_PREFERRED(std::forward<Args>(args)...); \
    }

#endif // LIBTRADIER_SIMD_RUNTIME_DETECTION

} // namespace simd
} // namespace tradier