#pragma once

/*
 * SIMD Configuration and Feature Detection
 * 
 * This header provides compile-time and runtime SIMD configuration.
 * It includes proper feature detection and fallback mechanisms.
 */

// SIMD availability detection
#ifdef __AVX2__
    #define LIBTRADIER_SIMD_AVX2_AVAILABLE 1
#else
    #define LIBTRADIER_SIMD_AVX2_AVAILABLE 0
#endif

#ifdef __AVX512F__
    #define LIBTRADIER_SIMD_AVX512_AVAILABLE 1
#else
    #define LIBTRADIER_SIMD_AVX512_AVAILABLE 0
#endif

// SIMD configuration
#ifndef LIBTRADIER_SIMD_ENABLED
    #define LIBTRADIER_SIMD_ENABLED 0
#endif

#if LIBTRADIER_SIMD_ENABLED && (LIBTRADIER_SIMD_AVX2_AVAILABLE || LIBTRADIER_SIMD_AVX512_AVAILABLE)
    #define LIBTRADIER_SIMD_ACTIVE 1
    
    // SIMD vector widths
    #if LIBTRADIER_SIMD_AVX512_AVAILABLE
        #define LIBTRADIER_SIMD_DOUBLE_WIDTH 8
        #define LIBTRADIER_SIMD_FLOAT_WIDTH 16
        #define LIBTRADIER_SIMD_PREFERRED avx512
    #elif LIBTRADIER_SIMD_AVX2_AVAILABLE
        #define LIBTRADIER_SIMD_DOUBLE_WIDTH 4
        #define LIBTRADIER_SIMD_FLOAT_WIDTH 8
        #define LIBTRADIER_SIMD_PREFERRED avx2
    #endif
    
    #define LIBTRADIER_SIMD_ALIGNMENT 64
    #define LIBTRADIER_SIMD_ALIGN alignas(LIBTRADIER_SIMD_ALIGNMENT)
    
#else
    #define LIBTRADIER_SIMD_ACTIVE 0
    
    // Fallback to scalar
    #define LIBTRADIER_SIMD_DOUBLE_WIDTH 1
    #define LIBTRADIER_SIMD_FLOAT_WIDTH 1
    #define LIBTRADIER_SIMD_ALIGNMENT 8
    #define LIBTRADIER_SIMD_PREFERRED scalar
    #define LIBTRADIER_SIMD_ALIGN
#endif

// Function attributes
#define LIBTRADIER_SIMD_INLINE inline __attribute__((always_inline))
#define LIBTRADIER_SIMD_NOINLINE __attribute__((noinline))
#define LIBTRADIER_SIMD_PURE __attribute__((pure))

// Conditional compilation helpers
#if LIBTRADIER_SIMD_ACTIVE
    #define LIBTRADIER_SIMD_IF_ENABLED(code) code
    #define LIBTRADIER_SIMD_IF_DISABLED(code)
#else
    #define LIBTRADIER_SIMD_IF_ENABLED(code)
    #define LIBTRADIER_SIMD_IF_DISABLED(code) code
#endif