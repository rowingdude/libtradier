cmake_minimum_required(VERSION 3.15)
project(libtradier_tests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CURL REQUIRED libcurl)
pkg_check_modules(OPENSSL REQUIRED openssl)

find_package(nlohmann_json REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/tests)

file(GLOB_RECURSE LIB_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE TEST_SOURCES "${CMAKE_SOURCE_DIR}/tests/*.cpp")

add_library(tradier_lib STATIC ${LIB_SOURCES})
target_link_libraries(tradier_lib 
    ${CURL_LIBRARIES} 
    ${OPENSSL_LIBRARIES}
    nlohmann_json::nlohmann_json
)
target_include_directories(tradier_lib PUBLIC 
    ${CURL_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIRS}
)

add_executable(run_tests ${TEST_SOURCES})
target_link_libraries(run_tests tradier_lib)

enable_testing()
add_test(NAME unit_tests COMMAND run_tests)