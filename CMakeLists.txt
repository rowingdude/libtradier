cmake_minimum_required(VERSION 3.15)
project(libtradier VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_EXAMPLES "Build example programs" ON)
option(BUILD_TESTS "Build test suite" ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CURL REQUIRED libcurl)
pkg_check_modules(OPENSSL REQUIRED openssl)
find_package(nlohmann_json REQUIRED)

file(GLOB_RECURSE LIB_SOURCES "src/*.cpp")
file(GLOB_RECURSE LIB_HEADERS "include/*.hpp")

add_library(tradier STATIC ${LIB_SOURCES})

target_include_directories(tradier 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CURL_INCLUDE_DIRS}
        ${OPENSSL_INCLUDE_DIRS}
)

target_link_libraries(tradier 
    PUBLIC
        nlohmann_json::nlohmann_json
    PRIVATE
        ${CURL_LIBRARIES}
        ${OPENSSL_LIBRARIES}
)

target_compile_definitions(tradier PRIVATE ${CURL_CFLAGS_OTHER})

if(BUILD_EXAMPLES)
    file(GLOB EXAMPLE_SOURCES "examples/*.cpp")
    foreach(EXAMPLE_SOURCE ${EXAMPLE_SOURCES})
        get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE} NAME_WE)
        add_executable(${EXAMPLE_NAME} ${EXAMPLE_SOURCE})
        target_link_libraries(${EXAMPLE_NAME} tradier)
    endforeach()
endif()

#if(BUILD_TESTS)
#    add_subdirectory(testing)
#endif()

install(TARGETS tradier
    EXPORT tradierTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/ DESTINATION include)

install(EXPORT tradierTargets
    FILE tradierTargets.cmake
    NAMESPACE tradier::
    DESTINATION lib/cmake/tradier
)