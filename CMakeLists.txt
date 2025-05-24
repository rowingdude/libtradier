cmake_minimum_required(VERSION 3.14...3.28)
project(libtradier VERSION 0.1.0 LANGUAGES CXX)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem thread url)
find_package(OpenSSL REQUIRED)

if(Boost_VERSION_STRING VERSION_LESS "1.66.0")
    message(FATAL_ERROR "Boost.Beast requires Boost >= 1.66.0, found ${Boost_VERSION_STRING}")
endif()

add_library(tradier
    src/client.cpp
    src/account.cpp
    src/trading.cpp
    src/market.cpp
    src/streaming.cpp
    src/watchlist.cpp
    src/auth.cpp

    src/json/market.cpp
    src/json/account.cpp  
    src/json/streaming.cpp
    src/json/trading.cpp
    
    src/common/json_utils_base.cpp
    src/common/http_client.cpp
    src/common/config.cpp
    src/common/websocket_client.cpp
    src/common/debug.cpp
    src/common/utils.cpp
)

target_include_directories(tradier
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(tradier
    PUBLIC
        CURL::libcurl
        nlohmann_json::nlohmann_json
        Boost::system
        Boost::thread
        Boost::url
        OpenSSL::SSL
        OpenSSL::Crypto
)

target_compile_features(tradier PUBLIC cxx_std_17)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(tradier PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_compile_definitions(tradier PRIVATE
    BOOST_BEAST_USE_STD_STRING_VIEW
    BOOST_ASIO_NO_DEPRECATED
)

# Example executables
#add_executable(trading_example examples/trading_example.cpp)
#target_link_libraries(trading_example PRIVATE tradier)

#add_executable(streaming_feature_test examples/streaming_feature_test.cpp)
#target_link_libraries(streaming_feature_test PRIVATE tradier)

#add_executable(watchlist_feature_test examples/watchlist_feature_test.cpp)
#target_link_libraries(watchlist_feature_test PRIVATE tradier)

#add_executable(market_feature_test examples/market_feature_test.cpp)
#target_link_libraries(market_feature_test PRIVATE tradier)

#add_executable(streaming_debug_test examples/streaming_debug_test.cpp)
#target_link_libraries(streaming_debug_test PRIVATE tradier)

#add_executable(trading_example_plus_plus examples/trading_example_plus_plus.cpp)
#target_link_libraries(trading_example_plus_plus PRIVATE tradier)

# Add auth example
#add_executable(auth_example examples/auth_example.cpp)
#target_link_libraries(auth_example PRIVATE tradier)

# Installation
include(GNUInstallDirs)

install(TARGETS tradier
    EXPORT tradier-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT tradier-targets
    FILE tradier-targets.cmake
    NAMESPACE tradier::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tradier
)

# Create config file
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/tradier-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/tradier-config.cmake"
    @ONLY
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/tradier-config.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tradier
)

# Optional: Enable testing
option(ENABLE_TESTING "Enable unit testing" OFF)

if(ENABLE_TESTING)
    enable_testing()
    find_package(GTest REQUIRED)
    
    add_executable(tradier_tests
        tests/test_auth.cpp
        tests/test_market.cpp
        tests/test_websocket.cpp
    )
    
    target_link_libraries(tradier_tests
        PRIVATE
            tradier
            GTest::GTest
            GTest::Main
    )
    
    add_test(NAME tradier_unit_tests COMMAND tradier_tests)
endif()
