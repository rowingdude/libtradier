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
find_package(Boost REQUIRED COMPONENTS system filesystem thread)
find_package(OpenSSL REQUIRED)

add_library(tradier
    src/client.cpp
    src/account.cpp
    src/trading.cpp
    src/market.cpp
    src/streaming.cpp
    src/common/http_client.cpp
    src/common/config.cpp
    src/common/json_utils.cpp
    src/common/websocket_client.cpp
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
        OpenSSL::SSL
        OpenSSL::Crypto
)

target_compile_features(tradier PUBLIC cxx_std_17)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(tradier PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(basic_example examples/basic_example.cpp)
target_link_libraries(basic_example PRIVATE tradier)

add_executable(trading_example examples/trading_example.cpp)
target_link_libraries(trading_example PRIVATE tradier)

add_executable(streaming_example examples/streaming_example.cpp)
target_link_libraries(streaming_example PRIVATE tradier)

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