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
find_package(Boost REQUIRED COMPONENTS system thread)

# Find websocketpp (header-only)
find_path(WEBSOCKETPP_INCLUDE_DIR websocketpp/config/asio_no_tls.hpp)
if(NOT WEBSOCKETPP_INCLUDE_DIR)
    message(FATAL_ERROR "websocketpp not found")
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
        ${Boost_INCLUDE_DIRS}
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
        Boost::system
        Boost::thread
)

# Compiler definitions
target_compile_definitions(tradier
    PRIVATE
        ${CURL_CFLAGS_OTHER}
)

# Compiler options
target_compile_options(tradier
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

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
option(BUILD_TESTS "Build test programs" ON)
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

# Print summary
message(STATUS "")
message(STATUS "libtradier ${PROJECT_VERSION} configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Shared Libraries: ${BUILD_SHARED_LIBS}")
message(STATUS "  Install: ${LIBTRADIER_INSTALL}")
message(STATUS "  Build Tests: ${BUILD_TESTS}")
message(STATUS "")