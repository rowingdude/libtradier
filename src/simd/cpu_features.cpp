/*
 * libtradier - CPU Feature Detection for Runtime SIMD Dispatch
 * 
 * Provides runtime detection of CPU SIMD capabilities
 * for optimal performance across different hardware
 * 
 * Author: libtradier team
 * Version: 1.0
 */

#include "tradier/simd/simd_traits.hpp"

#ifdef LIBTRADIER_SIMD_RUNTIME_DETECTION

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    #include <cpuid.h>
    #define LIBTRADIER_X86_CPUID_AVAILABLE 1
#else
    #define LIBTRADIER_X86_CPUID_AVAILABLE 0
#endif

namespace tradier {
namespace simd {

#if LIBTRADIER_X86_CPUID_AVAILABLE

namespace {

// CPUID leaf values for feature detection
constexpr uint32_t CPUID_LEAF_BASIC_FEATURES = 0x00000001;
constexpr uint32_t CPUID_LEAF_EXTENDED_FEATURES = 0x00000007;

// Feature bit positions
constexpr uint32_t SSE41_BIT = 19;  // ECX register
constexpr uint32_t AVX_BIT = 28;    // ECX register
constexpr uint32_t FMA_BIT = 12;    // ECX register
constexpr uint32_t F16C_BIT = 29;   // ECX register

constexpr uint32_t AVX2_BIT = 5;    // EBX register (leaf 7)
constexpr uint32_t AVX512F_BIT = 16; // EBX register (leaf 7)
constexpr uint32_t AVX512DQ_BIT = 17; // EBX register (leaf 7)
constexpr uint32_t AVX512BW_BIT = 30; // EBX register (leaf 7)
constexpr uint32_t AVX512VL_BIT = 31; // EBX register (leaf 7)

struct cpuid_result {
    uint32_t eax, ebx, ecx, edx;
};

cpuid_result get_cpuid(uint32_t leaf, uint32_t subleaf = 0) {
    cpuid_result result{};
    
#if defined(__GNUC__) || defined(__clang__)
    __cpuid_count(leaf, subleaf, result.eax, result.ebx, result.ecx, result.edx);
#elif defined(_MSC_VER)
    int cpu_info[4];
    __cpuidex(cpu_info, leaf, subleaf);
    result.eax = cpu_info[0];
    result.ebx = cpu_info[1];
    result.ecx = cpu_info[2];
    result.edx = cpu_info[3];
#endif
    
    return result;
}

bool check_bit(uint32_t value, uint32_t bit_position) {
    return (value & (1U << bit_position)) != 0;
}

// Check if OS supports XSAVE and AVX state management
bool check_os_avx_support() {
    // Check OSXSAVE bit (bit 27 in ECX)
    auto basic_features = get_cpuid(CPUID_LEAF_BASIC_FEATURES);
    if (!check_bit(basic_features.ecx, 27)) {
        return false;
    }
    
    // Check XCR0 register for AVX state support
#if defined(__GNUC__) || defined(__clang__)
    uint32_t xcr0_low, xcr0_high;
    __asm__ ("xgetbv" : "=a" (xcr0_low), "=d" (xcr0_high) : "c" (0));
    uint64_t xcr0 = ((uint64_t)xcr0_high << 32) | xcr0_low;
    
    // Check if XMM and YMM state are enabled (bits 1 and 2)
    return (xcr0 & 0x6) == 0x6;
#elif defined(_MSC_VER)
    return (_xgetbv(0) & 0x6) == 0x6;
#else
    return false;
#endif
}

// Check if OS supports AVX-512 state management
bool check_os_avx512_support() {
    if (!check_os_avx_support()) {
        return false;
    }
    
#if defined(__GNUC__) || defined(__clang__)
    uint32_t xcr0_low, xcr0_high;
    __asm__ ("xgetbv" : "=a" (xcr0_low), "=d" (xcr0_high) : "c" (0));
    uint64_t xcr0 = ((uint64_t)xcr0_high << 32) | xcr0_low;
    
    // Check if ZMM state is enabled (bits 5, 6, 7 for ZMM_Hi256, Hi16_ZMM, PKRU)
    return (xcr0 & 0xE0) == 0xE0;
#elif defined(_MSC_VER)
    return (_xgetbv(0) & 0xE0) == 0xE0;
#else
    return false;
#endif
}

} // anonymous namespace

cpu_features::cpu_features() noexcept 
    : sse41(false), avx(false), avx2(false), fma(false)
    , avx512f(false), avx512dq(false), avx512bw(false), avx512vl(false)
    , f16c(false) {
    
    // Get basic features (leaf 1)
    auto basic_features = get_cpuid(CPUID_LEAF_BASIC_FEATURES);
    
    // Check SSE4.1 support
    sse41 = check_bit(basic_features.ecx, SSE41_BIT);
    
    // Check AVX support (requires both CPU and OS support)
    bool cpu_avx = check_bit(basic_features.ecx, AVX_BIT);
    bool os_avx = check_os_avx_support();
    avx = cpu_avx && os_avx;
    
    // FMA and F16C require AVX
    if (avx) {
        fma = check_bit(basic_features.ecx, FMA_BIT);
        f16c = check_bit(basic_features.ecx, F16C_BIT);
    }
    
    // Get extended features (leaf 7, subleaf 0)
    auto extended_features = get_cpuid(CPUID_LEAF_EXTENDED_FEATURES, 0);
    
    // Check AVX2 support (requires AVX)
    if (avx) {
        avx2 = check_bit(extended_features.ebx, AVX2_BIT);
    }
    
    // Check AVX-512 support (requires AVX and OS support)
    bool cpu_avx512f = check_bit(extended_features.ebx, AVX512F_BIT);
    bool os_avx512 = check_os_avx512_support();
    
    if (avx && cpu_avx512f && os_avx512) {
        avx512f = true;
        avx512dq = check_bit(extended_features.ebx, AVX512DQ_BIT);
        avx512bw = check_bit(extended_features.ebx, AVX512BW_BIT);
        avx512vl = check_bit(extended_features.ebx, AVX512VL_BIT);
    }
}

#else // !LIBTRADIER_X86_CPUID_AVAILABLE

// Fallback implementation for non-x86 architectures
cpu_features::cpu_features() noexcept 
    : sse41(false), avx(false), avx2(false), fma(false)
    , avx512f(false), avx512dq(false), avx512bw(false), avx512vl(false)
    , f16c(false) {
    
    // On non-x86 architectures, assume no SIMD support
    // This could be extended for ARM NEON, etc.
}

#endif // LIBTRADIER_X86_CPUID_AVAILABLE

} // namespace simd
} // namespace tradier

#endif // LIBTRADIER_SIMD_RUNTIME_DETECTION