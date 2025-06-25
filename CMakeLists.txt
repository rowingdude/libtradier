cmake_minimum_required(VERSION 3.20)

project(libtradier
    VERSION 0.1.0
    DESCRIPTION "Tradier API C++ Library"
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(LIBTRADIER_INSTALL "Generate install target" ON)

# Debug and sanitizer options
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(ENABLE_MSAN "Enable MemorySanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(ENABLE_DEBUG_LOGGING "Enable debug logging" OFF)

# Testing and debugging tool options
option(ENABLE_VALGRIND_TESTS "Enable Valgrind memory testing" OFF)
option(ENABLE_GDB_SUPPORT "Enable GDB debugging support and test targets" OFF)

# Find dependencies
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

# Find nlohmann_json
find_package(nlohmann_json REQUIRED)

# Find libcurl
pkg_check_modules(CURL REQUIRED libcurl)

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Find Boost (required by websocketpp)
# Manual Boost detection for compatibility
find_path(BOOST_INCLUDE_DIR boost/version.hpp)
if(NOT BOOST_INCLUDE_DIR)
    message(FATAL_ERROR "Boost headers not found")
endif()

# Find Boost libraries manually
find_library(BOOST_SYSTEM_LIB boost_system)
find_library(BOOST_THREAD_LIB boost_thread)

if(NOT BOOST_SYSTEM_LIB OR NOT BOOST_THREAD_LIB)
    message(FATAL_ERROR "Boost system and thread libraries not found")
endif()

# Find websocketpp (header-only library)
# Try multiple paths and validate the installation
set(WEBSOCKETPP_SEARCH_PATHS
    /usr/include
    /usr/local/include
    /opt/homebrew/include
    ${CMAKE_PREFIX_PATH}/include
)

find_path(WEBSOCKETPP_INCLUDE_DIR 
    NAMES websocketpp/config/asio_client.hpp
    PATHS ${WEBSOCKETPP_SEARCH_PATHS}
    PATH_SUFFIXES websocketpp
)

if(NOT WEBSOCKETPP_INCLUDE_DIR)
    find_path(WEBSOCKETPP_INCLUDE_DIR 
        NAMES websocketpp/config/asio_no_tls.hpp
        PATHS ${WEBSOCKETPP_SEARCH_PATHS}
        PATH_SUFFIXES websocketpp
    )
endif()

if(NOT WEBSOCKETPP_INCLUDE_DIR)
    message(FATAL_ERROR "websocketpp headers not found. Please install libwebsocketpp-dev or websocketpp-devel package.")
endif()

# Validate websocketpp installation by checking for key headers
find_file(WEBSOCKETPP_CLIENT_HPP 
    NAMES websocketpp/client.hpp
    PATHS ${WEBSOCKETPP_INCLUDE_DIR}
    NO_DEFAULT_PATH
)

find_file(WEBSOCKETPP_CONFIG_ASIO_CLIENT_HPP 
    NAMES websocketpp/config/asio_client.hpp
    PATHS ${WEBSOCKETPP_INCLUDE_DIR}
    NO_DEFAULT_PATH
)

if(NOT WEBSOCKETPP_CLIENT_HPP OR NOT WEBSOCKETPP_CONFIG_ASIO_CLIENT_HPP)
    message(FATAL_ERROR "websocketpp installation appears incomplete. Missing required headers.")
endif()

message(STATUS "Found websocketpp headers in: ${WEBSOCKETPP_INCLUDE_DIR}")

# Check for websocketpp version if available
find_file(WEBSOCKETPP_VERSION_HPP 
    NAMES websocketpp/version.hpp
    PATHS ${WEBSOCKETPP_INCLUDE_DIR}
    NO_DEFAULT_PATH
)

if(WEBSOCKETPP_VERSION_HPP)
    file(READ ${WEBSOCKETPP_VERSION_HPP} WEBSOCKETPP_VERSION_CONTENT)
    string(REGEX MATCH "static int const major_version = ([0-9]+)" _ ${WEBSOCKETPP_VERSION_CONTENT})
    set(WEBSOCKETPP_VERSION_MAJOR ${CMAKE_MATCH_1})
    string(REGEX MATCH "static int const minor_version = ([0-9]+)" _ ${WEBSOCKETPP_VERSION_CONTENT})
    set(WEBSOCKETPP_VERSION_MINOR ${CMAKE_MATCH_1})
    
    if(WEBSOCKETPP_VERSION_MAJOR AND WEBSOCKETPP_VERSION_MINOR)
        message(STATUS "websocketpp version: ${WEBSOCKETPP_VERSION_MAJOR}.${WEBSOCKETPP_VERSION_MINOR}")
        
        # Warn about potentially problematic versions
        if(WEBSOCKETPP_VERSION_MAJOR EQUAL 0 AND WEBSOCKETPP_VERSION_MINOR LESS 8)
            message(WARNING "websocketpp version ${WEBSOCKETPP_VERSION_MAJOR}.${WEBSOCKETPP_VERSION_MINOR} may have compatibility issues. Consider upgrading to 0.8.0 or later.")
        endif()
    endif()
endif()

# Test compile websocketpp to ensure it works with the current compiler
include(CheckCXXSourceCompiles)
set(CMAKE_REQUIRED_INCLUDES ${WEBSOCKETPP_INCLUDE_DIR} ${BOOST_INCLUDE_DIR})
set(CMAKE_REQUIRED_LIBRARIES ${BOOST_SYSTEM_LIB} ${BOOST_THREAD_LIB})

# Try a simple compile test first
check_cxx_source_compiles("
#include <websocketpp/version.hpp>
int main() { return 0; }
" WEBSOCKETPP_BASIC_COMPILES)

if(WEBSOCKETPP_BASIC_COMPILES)
    # Try the full compile test
    check_cxx_source_compiles("
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>

int main() {
    websocketpp::client<websocketpp::config::asio_tls_client> client;
    client.init_asio();
    return 0;
}
" WEBSOCKETPP_COMPILES)
    
    if(WEBSOCKETPP_COMPILES)
        message(STATUS "websocketpp compile test passed")
        set(WEBSOCKETPP_WORKS TRUE)
    else()
        message(WARNING "websocketpp headers found but do not compile correctly. WebSocket functionality will be disabled.")
        set(WEBSOCKETPP_WORKS FALSE)
    endif()
else()
    message(WARNING "websocketpp headers are broken or incompatible. WebSocket functionality will be disabled.")
    set(WEBSOCKETPP_WORKS FALSE)
endif()

# Store websocketpp status for later use
if(WEBSOCKETPP_WORKS)
    message(STATUS "WebSocket support: ENABLED")
else()
    message(STATUS "WebSocket support: DISABLED (websocketpp issues)")
endif()

# SIMD vectorization support
option(ENABLE_SIMD "Enable SIMD vectorization" OFF)

# SIMD sources (conditionally included)
set(LIBTRADIER_SIMD_SOURCES "")
if(ENABLE_SIMD)
    message(STATUS "SIMD vectorization: ENABLED")
    set(LIBTRADIER_SIMD_SOURCES
        src/simd/streaming_simd.cpp
    )
else()
    message(STATUS "SIMD vectorization: DISABLED")
endif()

# Find debugging tools if requested
if(ENABLE_VALGRIND_TESTS)
    find_program(VALGRIND_EXECUTABLE NAMES valgrind)
    if(VALGRIND_EXECUTABLE)
        message(STATUS "Found Valgrind: ${VALGRIND_EXECUTABLE}")
        set(VALGRIND_AVAILABLE TRUE)
    else()
        message(WARNING "Valgrind requested but not found. Install with: sudo apt install valgrind")
        set(ENABLE_VALGRIND_TESTS OFF)
        set(VALGRIND_AVAILABLE FALSE)
    endif()
endif()

if(ENABLE_GDB_SUPPORT)
    find_program(GDB_EXECUTABLE NAMES gdb)
    if(GDB_EXECUTABLE)
        message(STATUS "Found GDB: ${GDB_EXECUTABLE}")
        set(GDB_AVAILABLE TRUE)
        # Force debug symbols for GDB support
        set(CMAKE_BUILD_TYPE Debug)
        message(STATUS "GDB support enabled, forcing Debug build type")
    else()
        message(WARNING "GDB requested but not found. Install with: sudo apt install gdb")
        set(ENABLE_GDB_SUPPORT OFF)
        set(GDB_AVAILABLE FALSE)
    endif()
endif()

# Create library target
file(GLOB_RECURSE LIBTRADIER_SOURCES
    "src/*.cpp"
    "src/*.hpp"
)

file(GLOB_RECURSE LIBTRADIER_HEADERS
    "include/*.hpp"
)

add_library(tradier ${LIBTRADIER_SOURCES})

# Set target properties
set_target_properties(tradier PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${LIBTRADIER_HEADERS}"
)

# Include directories
target_include_directories(tradier
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${WEBSOCKETPP_INCLUDE_DIR}
        ${CURL_INCLUDE_DIRS}
        ${BOOST_INCLUDE_DIR}
)

# Link libraries
target_link_libraries(tradier
    PUBLIC
        nlohmann_json::nlohmann_json
    PRIVATE
        Threads::Threads
        OpenSSL::SSL
        OpenSSL::Crypto
        ${CURL_LIBRARIES}
        ${BOOST_SYSTEM_LIB}
        ${BOOST_THREAD_LIB}
)

# Compiler definitions
target_compile_definitions(tradier
    PRIVATE
        ${CURL_CFLAGS_OTHER}
)

# Add websocketpp conditional compilation flag
if(WEBSOCKETPP_WORKS)
    target_compile_definitions(tradier PRIVATE WEBSOCKETPP_ENABLED=1)
else()
    target_compile_definitions(tradier PRIVATE WEBSOCKETPP_ENABLED=0)
endif()

# Apply SIMD vectorization settings to main target
# target_enable_simd(tradier) # SIMD disabled

# Configure SIMD settings
if(ENABLE_SIMD)
    # Add SIMD compile definitions
    target_compile_definitions(tradier PRIVATE LIBTRADIER_SIMD_ENABLED=1)
    
    # Add SIMD compile flags
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(tradier PRIVATE -mavx2 -mfma)
        
        # Check for AVX-512 support
        include(CheckCXXCompilerFlag)
        check_cxx_compiler_flag("-mavx512f" COMPILER_SUPPORTS_AVX512)
        if(COMPILER_SUPPORTS_AVX512)
            target_compile_options(tradier PRIVATE -mavx512f)
        endif()
    endif()
else()
    target_compile_definitions(tradier PRIVATE LIBTRADIER_SIMD_ENABLED=0)
endif()

# Always include generated headers directory (for SIMD config)
target_include_directories(tradier PRIVATE "${CMAKE_BINARY_DIR}/generated")

# Sanitizer configuration
set(SANITIZER_FLAGS "")
set(SANITIZER_LINK_FLAGS "")

if(ENABLE_ASAN)
    list(APPEND SANITIZER_FLAGS "-fsanitize=address" "-fno-omit-frame-pointer")
    list(APPEND SANITIZER_LINK_FLAGS "-fsanitize=address")
    message(STATUS "AddressSanitizer enabled")
endif()

if(ENABLE_TSAN)
    list(APPEND SANITIZER_FLAGS "-fsanitize=thread" "-fno-omit-frame-pointer")
    list(APPEND SANITIZER_LINK_FLAGS "-fsanitize=thread")
    message(STATUS "ThreadSanitizer enabled")
endif()

if(ENABLE_MSAN)
    list(APPEND SANITIZER_FLAGS "-fsanitize=memory" "-fno-omit-frame-pointer")
    list(APPEND SANITIZER_LINK_FLAGS "-fsanitize=memory")
    message(STATUS "MemorySanitizer enabled")
endif()

if(ENABLE_UBSAN)
    list(APPEND SANITIZER_FLAGS "-fsanitize=undefined" "-fno-omit-frame-pointer")
    list(APPEND SANITIZER_LINK_FLAGS "-fsanitize=undefined")
    message(STATUS "UndefinedBehaviorSanitizer enabled")
endif()

# Debug logging configuration
if(ENABLE_DEBUG_LOGGING)
    target_compile_definitions(tradier PRIVATE LIBTRADIER_DEBUG_LOGGING=1)
    message(STATUS "Debug logging enabled")
endif()

# GDB debugging configuration
if(ENABLE_GDB_SUPPORT)
    target_compile_options(tradier PRIVATE
        -g3                    # Maximum debug info
        -ggdb                  # GDB-specific debug info
        -fno-omit-frame-pointer # Better stack traces
        -fno-optimize-sibling-calls # Clearer call stack
    )
    target_compile_definitions(tradier PRIVATE
        LIBTRADIER_DEBUG_BUILD=1
        _GLIBCXX_DEBUG=1       # STL debug mode
    )
endif()

# Compiler options
target_compile_options(tradier
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O3 -DNDEBUG>
        ${SANITIZER_FLAGS}
)

# Linker options for sanitizers
if(SANITIZER_LINK_FLAGS)
    target_link_options(tradier PRIVATE ${SANITIZER_LINK_FLAGS})
endif()

# Installation
if(LIBTRADIER_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    # Install library
    install(TARGETS tradier
        EXPORT libtradierTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tradier
    )

    # Install headers with directory structure
    install(DIRECTORY include/tradier
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hpp"
    )

    # Create and install package config files
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/libtradierConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/libtradierConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/libtradierConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libtradier
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/libtradierConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/libtradierConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libtradier
    )

    install(EXPORT libtradierTargets
        FILE libtradierTargets.cmake
        NAMESPACE libtradier::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libtradier
    )

    # Create pkg-config file
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/libtradier.pc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/libtradier.pc"
        @ONLY
    )

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libtradier.pc"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    )
endif()

# Add tests subdirectory
option(BUILD_TESTS "Build test programs" OFF)
if(BUILD_TESTS)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
        add_subdirectory(tests)
    else()
        message(STATUS "Tests directory not found, skipping test build")
        set(BUILD_TESTS OFF)
    endif()
endif()

# Add examples subdirectory
option(BUILD_EXAMPLES "Build example programs" OFF)
option(LIBTRADIER_INSTALL_EXAMPLES "Install example programs and source code" OFF)
if(BUILD_EXAMPLES)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/CMakeLists.txt")
        add_subdirectory(examples)
    else()
        message(STATUS "Examples directory not found, skipping examples build")
        set(BUILD_EXAMPLES OFF)
    endif()
endif()

# Testing targets with debugging tools
enable_testing()

# Custom testing targets for Valgrind
if(VALGRIND_AVAILABLE AND BUILD_TESTS)
    # Valgrind memory leak detection
    add_custom_target(valgrind-test
        COMMAND ${VALGRIND_EXECUTABLE} 
            --leak-check=full 
            --show-leak-kinds=all 
            --track-origins=yes
            --verbose
            --error-exitcode=1
            --suppressions=${CMAKE_CURRENT_SOURCE_DIR}/.valgrind-suppressions
            $<TARGET_FILE:tradier>
        DEPENDS tradier
        COMMENT "Running Valgrind memory leak detection"
        VERBATIM
    )
    
    # Valgrind thread safety analysis
    add_custom_target(helgrind-test
        COMMAND ${VALGRIND_EXECUTABLE} 
            --tool=helgrind
            --verbose
            --error-exitcode=1
            $<TARGET_FILE:tradier>
        DEPENDS tradier
        COMMENT "Running Helgrind thread safety analysis"
        VERBATIM
    )
    
    # Valgrind cache profiling
    add_custom_target(cachegrind-test
        COMMAND ${VALGRIND_EXECUTABLE} 
            --tool=cachegrind
            --verbose
            $<TARGET_FILE:tradier>
        DEPENDS tradier
        COMMENT "Running Cachegrind cache profiling"
        VERBATIM
    )
    
    message(STATUS "Valgrind targets available: valgrind-test, helgrind-test, cachegrind-test")
endif()

# Custom testing targets for GDB
if(GDB_AVAILABLE AND BUILD_TESTS)
    # GDB backtrace on crash
    add_custom_target(gdb-test
        COMMAND ${GDB_EXECUTABLE} 
            --batch
            --ex "set confirm off"
            --ex "run"
            --ex "bt"
            --ex "info registers"
            --ex "quit"
            --args $<TARGET_FILE:tradier>
        DEPENDS tradier
        COMMENT "Running GDB crash analysis"
        VERBATIM
    )
    
    # GDB with core dump analysis
    add_custom_target(gdb-core-analysis
        COMMAND bash -c "ulimit -c unlimited && ${GDB_EXECUTABLE} --batch --ex 'set confirm off' --ex 'run' --ex 'generate-core-file core.libtradier' --ex 'quit' --args $<TARGET_FILE:tradier> || ${GDB_EXECUTABLE} --batch --ex 'bt' --ex 'info registers' --ex 'quit' $<TARGET_FILE:tradier> core.libtradier"
        DEPENDS tradier
        COMMENT "Running GDB with core dump generation and analysis"
        VERBATIM
    )
    
    message(STATUS "GDB targets available: gdb-test, gdb-core-analysis")
endif()

# Combined memory and thread safety test
if(VALGRIND_AVAILABLE AND BUILD_TESTS)
    add_custom_target(memory-safety-test
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target valgrind-test
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target helgrind-test
        DEPENDS tradier
        COMMENT "Running comprehensive memory and thread safety tests"
    )
    
    message(STATUS "Combined target available: memory-safety-test")
endif()

# Cleanup targets for testing artifacts
add_custom_target(clean-test-artifacts
    COMMAND ${CMAKE_COMMAND} -E remove -f 
        ${CMAKE_BINARY_DIR}/core.*
        ${CMAKE_BINARY_DIR}/*.core
        ${CMAKE_BINARY_DIR}/vgcore.*
        ${CMAKE_BINARY_DIR}/callgrind.out.*
        ${CMAKE_BINARY_DIR}/cachegrind.out.*
        ${CMAKE_BINARY_DIR}/massif.out.*
        ${CMAKE_BINARY_DIR}/helgrind.log.*
        ${CMAKE_BINARY_DIR}/valgrind.log.*
        ${CMAKE_BINARY_DIR}/*.tmp
        ${CMAKE_BINARY_DIR}/*.log
    COMMENT "Cleaning up test artifacts and temporary files"
    VERBATIM
)

# Enhanced cleanup targets that run after testing
if(VALGRIND_AVAILABLE AND BUILD_TESTS)
    # Add cleanup step to valgrind targets
    add_custom_command(TARGET valgrind-test POST_BUILD
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean-test-artifacts
        COMMENT "Cleaning up after Valgrind testing"
    )
    
    add_custom_command(TARGET helgrind-test POST_BUILD
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean-test-artifacts
        COMMENT "Cleaning up after Helgrind testing"
    )
    
    add_custom_command(TARGET cachegrind-test POST_BUILD
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean-test-artifacts
        COMMENT "Cleaning up after Cachegrind testing"
    )
endif()

if(GDB_AVAILABLE AND BUILD_TESTS)
    # Add cleanup step to GDB targets
    add_custom_command(TARGET gdb-test POST_BUILD
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean-test-artifacts
        COMMENT "Cleaning up after GDB testing"
    )
    
    add_custom_command(TARGET gdb-core-analysis POST_BUILD
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean-test-artifacts
        COMMENT "Cleaning up after GDB core analysis"
    )
endif()

message(STATUS "Cleanup target available: clean-test-artifacts")

# Print summary
message(STATUS "")
message(STATUS "libtradier ${PROJECT_VERSION} configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Shared Libraries: ${BUILD_SHARED_LIBS}")
message(STATUS "  Install: ${LIBTRADIER_INSTALL}")
message(STATUS "  Build Tests: ${BUILD_TESTS}")
message(STATUS "  Build Examples: ${BUILD_EXAMPLES}")
message(STATUS "  AddressSanitizer: ${ENABLE_ASAN}")
message(STATUS "  ThreadSanitizer: ${ENABLE_TSAN}")
message(STATUS "  MemorySanitizer: ${ENABLE_MSAN}")
message(STATUS "  UBSanitizer: ${ENABLE_UBSAN}")
message(STATUS "  Debug Logging: ${ENABLE_DEBUG_LOGGING}")
message(STATUS "  Valgrind Tests: ${ENABLE_VALGRIND_TESTS}")
message(STATUS "  GDB Support: ${ENABLE_GDB_SUPPORT}")
message(STATUS "")