# SIMD/AVX Vectorization Configuration for libtradier
# Optional compile-time optimizations for high-performance trading applications
# Author: libtradier team
# Version: 1.0

# Check if SIMD optimizations are requested
option(ENABLE_SIMD "Enable SIMD/AVX vectorization optimizations" OFF)
option(ENABLE_AVX2 "Enable AVX2 optimizations (requires AVX2 support)" OFF)
option(ENABLE_AVX512 "Enable AVX-512 optimizations (requires AVX-512 support)" OFF)
option(ENABLE_NATIVE_OPTIMIZATION "Enable native CPU optimization (-march=native)" OFF)

# Performance vs compatibility options
option(SIMD_RUNTIME_DETECTION "Enable runtime CPU feature detection for SIMD" ON)
option(SIMD_FALLBACK_SCALAR "Always provide scalar fallback implementations" ON)

if(ENABLE_SIMD)
    message(STATUS "SIMD vectorization: ENABLED")
    
    # Detect compiler support
    include(CheckCXXCompilerFlag)
    
    # Check for basic SIMD support
    check_cxx_compiler_flag("-msse4.1" COMPILER_SUPPORTS_SSE41)
    check_cxx_compiler_flag("-mavx" COMPILER_SUPPORTS_AVX)
    check_cxx_compiler_flag("-mavx2" COMPILER_SUPPORTS_AVX2)
    check_cxx_compiler_flag("-mavx512f" COMPILER_SUPPORTS_AVX512F)
    check_cxx_compiler_flag("-mfma" COMPILER_SUPPORTS_FMA)
    check_cxx_compiler_flag("-mf16c" COMPILER_SUPPORTS_F16C)
    
    # Set up compiler flags based on requested optimizations
    set(SIMD_COMPILE_FLAGS "")
    set(SIMD_COMPILE_DEFINITIONS "")
    
    # Base SIMD support (SSE4.1 minimum for financial calculations)
    if(COMPILER_SUPPORTS_SSE41)
        list(APPEND SIMD_COMPILE_FLAGS "-msse4.1")
        list(APPEND SIMD_COMPILE_DEFINITIONS "LIBTRADIER_SIMD_SSE41=1")
        message(STATUS "SIMD: SSE4.1 support enabled")
    else()
        message(WARNING "SSE4.1 not supported by compiler - SIMD optimizations may be limited")
    endif()
    
    # AVX support
    if(COMPILER_SUPPORTS_AVX)
        list(APPEND SIMD_COMPILE_FLAGS "-mavx")
        list(APPEND SIMD_COMPILE_DEFINITIONS "LIBTRADIER_SIMD_AVX=1")
        message(STATUS "SIMD: AVX support enabled")
    endif()
    
    # AVX2 support (major performance boost for double precision)
    if(ENABLE_AVX2 AND COMPILER_SUPPORTS_AVX2)
        list(APPEND SIMD_COMPILE_FLAGS "-mavx2")
        list(APPEND SIMD_COMPILE_DEFINITIONS "LIBTRADIER_SIMD_AVX2=1")
        message(STATUS "SIMD: AVX2 support enabled")
        
        # FMA support (often available with AVX2)
        if(COMPILER_SUPPORTS_FMA)
            list(APPEND SIMD_COMPILE_FLAGS "-mfma")
            list(APPEND SIMD_COMPILE_DEFINITIONS "LIBTRADIER_SIMD_FMA=1")
            message(STATUS "SIMD: FMA (Fused Multiply-Add) support enabled")
        endif()
        
        # F16C support for half-precision floats
        if(COMPILER_SUPPORTS_F16C)
            list(APPEND SIMD_COMPILE_FLAGS "-mf16c")
            list(APPEND SIMD_COMPILE_DEFINITIONS "LIBTRADIER_SIMD_F16C=1")
            message(STATUS "SIMD: F16C (half-precision) support enabled")
        endif()
    elseif(ENABLE_AVX2)
        message(WARNING "AVX2 requested but not supported by compiler")
    endif()
    
    # AVX-512 support (highest performance for server-grade CPUs)
    if(ENABLE_AVX512 AND COMPILER_SUPPORTS_AVX512F)
        list(APPEND SIMD_COMPILE_FLAGS "-mavx512f")
        list(APPEND SIMD_COMPILE_DEFINITIONS "LIBTRADIER_SIMD_AVX512=1")
        message(STATUS "SIMD: AVX-512 support enabled")
        
        # Additional AVX-512 extensions
        check_cxx_compiler_flag("-mavx512dq" COMPILER_SUPPORTS_AVX512DQ)
        check_cxx_compiler_flag("-mavx512bw" COMPILER_SUPPORTS_AVX512BW)
        check_cxx_compiler_flag("-mavx512vl" COMPILER_SUPPORTS_AVX512VL)
        
        if(COMPILER_SUPPORTS_AVX512DQ)
            list(APPEND SIMD_COMPILE_FLAGS "-mavx512dq")
            list(APPEND SIMD_COMPILE_DEFINITIONS "LIBTRADIER_SIMD_AVX512DQ=1")
        endif()
        
        if(COMPILER_SUPPORTS_AVX512BW)
            list(APPEND SIMD_COMPILE_FLAGS "-mavx512bw")
            list(APPEND SIMD_COMPILE_DEFINITIONS "LIBTRADIER_SIMD_AVX512BW=1")
        endif()
        
        if(COMPILER_SUPPORTS_AVX512VL)
            list(APPEND SIMD_COMPILE_FLAGS "-mavx512vl")
            list(APPEND SIMD_COMPILE_DEFINITIONS "LIBTRADIER_SIMD_AVX512VL=1")
        endif()
    elseif(ENABLE_AVX512)
        message(WARNING "AVX-512 requested but not supported by compiler")
    endif()
    
    # Native optimization
    if(ENABLE_NATIVE_OPTIMIZATION)
        list(APPEND SIMD_COMPILE_FLAGS "-march=native" "-mtune=native")
        list(APPEND SIMD_COMPILE_DEFINITIONS "LIBTRADIER_SIMD_NATIVE=1")
        message(STATUS "SIMD: Native CPU optimization enabled")
        message(WARNING "Native optimization may reduce binary portability")
    endif()
    
    # Runtime detection
    if(SIMD_RUNTIME_DETECTION)
        list(APPEND SIMD_COMPILE_DEFINITIONS "LIBTRADIER_SIMD_RUNTIME_DETECTION=1")
        message(STATUS "SIMD: Runtime CPU feature detection enabled")
    endif()
    
    # Fallback support
    if(SIMD_FALLBACK_SCALAR)
        list(APPEND SIMD_COMPILE_DEFINITIONS "LIBTRADIER_SIMD_FALLBACK_SCALAR=1")
        message(STATUS "SIMD: Scalar fallback implementations enabled")
    endif()
    
    # Set global compile definitions
    list(APPEND SIMD_COMPILE_DEFINITIONS "LIBTRADIER_SIMD_ENABLED=1")
    
else()
    message(STATUS "SIMD vectorization: DISABLED (use -DENABLE_SIMD=ON to enable)")
    set(SIMD_COMPILE_FLAGS "")
    set(SIMD_COMPILE_DEFINITIONS "")
endif()

# Function to apply SIMD settings to a target
function(target_enable_simd target_name)
    if(ENABLE_SIMD)
        # Add generated headers include directory
        target_include_directories(${target_name} PRIVATE "${CMAKE_BINARY_DIR}/generated")
        
        # Apply compiler flags
        if(SIMD_COMPILE_FLAGS)
            target_compile_options(${target_name} PRIVATE ${SIMD_COMPILE_FLAGS})
        endif()
        
        # Apply preprocessor definitions
        if(SIMD_COMPILE_DEFINITIONS)
            target_compile_definitions(${target_name} PRIVATE ${SIMD_COMPILE_DEFINITIONS})
        endif()
        
        # Link with required libraries (if any)
        # Currently no additional libraries needed for basic SIMD
        
        message(STATUS "SIMD optimizations applied to target: ${target_name}")
    endif()
endfunction()

# Function to create SIMD-optimized variant of a target
function(create_simd_variant base_target_name simd_target_name sources)
    if(ENABLE_SIMD)
        # Create the SIMD-optimized target
        add_library(${simd_target_name} OBJECT ${sources})
        
        # Apply SIMD settings
        target_enable_simd(${simd_target_name})
        
        # Inherit properties from base target
        get_target_property(BASE_INCLUDE_DIRS ${base_target_name} INCLUDE_DIRECTORIES)
        if(BASE_INCLUDE_DIRS)
            target_include_directories(${simd_target_name} PRIVATE ${BASE_INCLUDE_DIRS})
        endif()
        
        get_target_property(BASE_COMPILE_DEFS ${base_target_name} COMPILE_DEFINITIONS)
        if(BASE_COMPILE_DEFS)
            target_compile_definitions(${simd_target_name} PRIVATE ${BASE_COMPILE_DEFS})
        endif()
        
        # Set additional SIMD-specific definitions
        target_compile_definitions(${simd_target_name} PRIVATE 
            LIBTRADIER_SIMD_VARIANT=1
            LIBTRADIER_SIMD_TARGET_NAME="${simd_target_name}"
        )
        
        message(STATUS "Created SIMD variant: ${simd_target_name}")
    endif()
endfunction()

# Runtime CPU feature detection function
if(ENABLE_SIMD AND SIMD_RUNTIME_DETECTION)
    # Check if we need additional libraries for CPU feature detection
    find_library(CPUID_LIB cpuid)
    if(CPUID_LIB)
        set(SIMD_LINK_LIBRARIES ${CPUID_LIB})
        message(STATUS "SIMD: Using external cpuid library for feature detection")
    else()
        # We'll use compiler built-ins
        message(STATUS "SIMD: Using compiler built-ins for CPU feature detection")
    endif()
endif()

# SIMD benchmarking support
if(ENABLE_SIMD AND BUILD_BENCHMARKS)
    set(SIMD_BENCHMARKS_ENABLED TRUE)
    message(STATUS "SIMD: Benchmarking support enabled")
else()
    set(SIMD_BENCHMARKS_ENABLED FALSE)
endif()

# Set variables for SIMD configuration header generation (always set regardless of SIMD enable status)
if(ENABLE_SIMD)
    set(LIBTRADIER_SIMD_ENABLED 1)
else()
    set(LIBTRADIER_SIMD_ENABLED 0)
endif()

if(COMPILER_SUPPORTS_SSE41 AND ENABLE_SIMD)
    set(LIBTRADIER_SIMD_SSE41 1)
else()
    set(LIBTRADIER_SIMD_SSE41 0)
endif()

if(COMPILER_SUPPORTS_AVX AND ENABLE_SIMD)
    set(LIBTRADIER_SIMD_AVX 1)
else()
    set(LIBTRADIER_SIMD_AVX 0)
endif()

if(COMPILER_SUPPORTS_AVX2 AND ENABLE_AVX2 AND ENABLE_SIMD)
    set(LIBTRADIER_SIMD_AVX2 1)
else()
    set(LIBTRADIER_SIMD_AVX2 0)
endif()

if(COMPILER_SUPPORTS_AVX512F AND ENABLE_AVX512 AND ENABLE_SIMD)
    set(LIBTRADIER_SIMD_AVX512 1)
    set(LIBTRADIER_SIMD_AVX512DQ 1)
    set(LIBTRADIER_SIMD_AVX512BW 1)
    set(LIBTRADIER_SIMD_AVX512VL 1)
else()
    set(LIBTRADIER_SIMD_AVX512 0)
    set(LIBTRADIER_SIMD_AVX512DQ 0)
    set(LIBTRADIER_SIMD_AVX512BW 0)
    set(LIBTRADIER_SIMD_AVX512VL 0)
endif()

if(COMPILER_SUPPORTS_FMA AND ENABLE_SIMD)
    set(LIBTRADIER_SIMD_FMA 1)
else()
    set(LIBTRADIER_SIMD_FMA 0)
endif()

if(COMPILER_SUPPORTS_F16C AND ENABLE_SIMD)
    set(LIBTRADIER_SIMD_F16C 1)
else()
    set(LIBTRADIER_SIMD_F16C 0)
endif()

if(ENABLE_NATIVE_OPTIMIZATION AND ENABLE_SIMD)
    set(LIBTRADIER_SIMD_NATIVE 1)
else()
    set(LIBTRADIER_SIMD_NATIVE 0)
endif()

if(SIMD_RUNTIME_DETECTION AND ENABLE_SIMD)
    set(LIBTRADIER_SIMD_RUNTIME_DETECTION 1)
else()
    set(LIBTRADIER_SIMD_RUNTIME_DETECTION 0)
endif()

if(SIMD_FALLBACK_SCALAR AND ENABLE_SIMD)
    set(LIBTRADIER_SIMD_FALLBACK_SCALAR 1)
else()
    set(LIBTRADIER_SIMD_FALLBACK_SCALAR 0)
endif()

if(ENABLE_SIMD)
    set(LIBTRADIER_SIMD_VARIANT 1)
else()
    set(LIBTRADIER_SIMD_VARIANT 0)
endif()

# Create SIMD configuration header (always generated)
configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/simd_config.h.in"
    "${CMAKE_BINARY_DIR}/generated/tradier/simd_config.h"
    @ONLY
)

# Make sure the generated headers directory is always available
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated/tradier")

# Summary
if(ENABLE_SIMD)
    message(STATUS "")
    message(STATUS "SIMD Configuration Summary:")
    message(STATUS "  Base SIMD: ${COMPILER_SUPPORTS_AVX}")
    message(STATUS "  AVX2: ${ENABLE_AVX2}")
    message(STATUS "  AVX-512: ${ENABLE_AVX512}")
    message(STATUS "  Native optimization: ${ENABLE_NATIVE_OPTIMIZATION}")
    message(STATUS "  Runtime detection: ${SIMD_RUNTIME_DETECTION}")
    message(STATUS "  Scalar fallback: ${SIMD_FALLBACK_SCALAR}")
    message(STATUS "  Compiler flags: ${SIMD_COMPILE_FLAGS}")
    message(STATUS "")
endif()