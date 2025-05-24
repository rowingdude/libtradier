cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CURL REQUIRED libcurl)
pkg_check_modules(OPENSSL REQUIRED openssl)
find_package(nlohmann_json REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp")
add_executable(run_tests ${TEST_SOURCES})
target_link_libraries(run_tests 
    tradier
    ${CURL_LIBRARIES} 
    ${OPENSSL_LIBRARIES}
    nlohmann_json::nlohmann_json
)

target_include_directories(run_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CURL_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIRS}
)

target_compile_definitions(run_tests PRIVATE ${CURL_CFLAGS_OTHER})

enable_testing()
add_test(NAME libtradier_unit_tests COMMAND run_tests)

set_tests_properties(libtradier_unit_tests PROPERTIES
    TIMEOUT 300
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(test_libtradier
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --verbose
    DEPENDS run_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running libtradier tests"
)

foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    if(NOT ${TEST_NAME} STREQUAL "test_main")
        add_executable(${TEST_NAME}_individual ${TEST_SOURCE} test_main.cpp)
        target_link_libraries(${TEST_NAME}_individual tradier ${CURL_LIBRARIES} ${OPENSSL_LIBRARIES} nlohmann_json::nlohmann_json)
        target_include_directories(${TEST_NAME}_individual PRIVATE 
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CURL_INCLUDE_DIRS}
            ${OPENSSL_INCLUDE_DIRS}
        )
        target_compile_definitions(${TEST_NAME}_individual PRIVATE ${CURL_CFLAGS_OTHER})
    endif()
endforeach()