cmake_minimum_required(VERSION 3.20)

# Test configuration
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find test dependencies
find_package(PkgConfig REQUIRED)

# Try to find Catch2 (preferred)
find_package(Catch2 QUIET)

if(NOT Catch2_FOUND)
    # Try pkg-config for catch2
    pkg_check_modules(CATCH2 QUIET catch2-with-main)
    
    if(NOT CATCH2_FOUND)
        message(STATUS "Catch2 not found, downloading single-header version")
        
        # Download Catch2 single header
        include(FetchContent)
        FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v3.4.0
        )
        
        FetchContent_MakeAvailable(Catch2)
        
        # Include Catch2 CMake modules
        list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
        include(CTest)
        include(Catch)
        
        set(CATCH2_FOUND TRUE)
        message(STATUS "Using downloaded Catch2 v3.4.0")
    else()
        message(STATUS "Found Catch2 via pkg-config")
    endif()
else()
    message(STATUS "Found Catch2 via find_package")
endif()

# Create unit test directory structure
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/unit)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/integration)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/mocks)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/fixtures)

# Unit tests
set(UNIT_TEST_SOURCES
    unit/test_simple.cpp
)

# Integration tests
set(INTEGRATION_TEST_SOURCES
    integration/test_api_integration.cpp
)

# Mock and fixture sources
set(MOCK_SOURCES
    # mocks/mock_http_client.cpp
    # fixtures/test_data.cpp
)

# Create unit test executable
add_executable(unit_tests ${UNIT_TEST_SOURCES} ${MOCK_SOURCES})

target_include_directories(unit_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
    ${CMAKE_CURRENT_SOURCE_DIR}/fixtures
)

target_link_libraries(unit_tests PRIVATE
    tradier
    Catch2::Catch2WithMain
)

# Create integration test executable
add_executable(integration_tests ${INTEGRATION_TEST_SOURCES} ${MOCK_SOURCES})

target_include_directories(integration_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
    ${CMAKE_CURRENT_SOURCE_DIR}/fixtures
)

target_link_libraries(integration_tests PRIVATE
    tradier
    Catch2::Catch2WithMain
)

# Register tests with CTest
if(CATCH2_FOUND)
    catch_discover_tests(unit_tests)
    catch_discover_tests(integration_tests)
endif()

# Custom test targets
add_custom_target(run_unit_tests
    COMMAND unit_tests
    DEPENDS unit_tests
    COMMENT "Running unit tests"
)

add_custom_target(run_integration_tests
    COMMAND integration_tests
    DEPENDS integration_tests
    COMMENT "Running integration tests"
)

add_custom_target(run_all_tests
    COMMAND unit_tests
    COMMAND integration_tests
    DEPENDS unit_tests integration_tests
    COMMENT "Running all tests"
)

# Test coverage target (if gcov is available)
find_program(GCOV_EXECUTABLE gcov)
find_program(LCOV_EXECUTABLE lcov)
find_program(GENHTML_EXECUTABLE genhtml)

if(GCOV_EXECUTABLE AND LCOV_EXECUTABLE AND GENHTML_EXECUTABLE)
    # Add coverage flags
    target_compile_options(unit_tests PRIVATE --coverage)
    target_link_options(unit_tests PRIVATE --coverage)
    target_compile_options(integration_tests PRIVATE --coverage)
    target_link_options(integration_tests PRIVATE --coverage)
    
    # Create coverage target
    add_custom_target(coverage
        COMMAND ${LCOV_EXECUTABLE} --capture --directory . --output-file coverage.info
        COMMAND ${LCOV_EXECUTABLE} --remove coverage.info '/usr/*' --output-file coverage.info
        COMMAND ${LCOV_EXECUTABLE} --remove coverage.info '*/tests/*' --output-file coverage.info
        COMMAND ${GENHTML_EXECUTABLE} coverage.info --output-directory coverage_html
        DEPENDS run_all_tests
        COMMENT "Generating test coverage report"
    )
    
    message(STATUS "Code coverage target available: coverage")
endif()

# Valgrind test targets (if Valgrind is available)
find_program(VALGRIND_EXECUTABLE valgrind)
if(VALGRIND_EXECUTABLE)
    add_custom_target(valgrind_unit_tests
        COMMAND ${VALGRIND_EXECUTABLE} 
            --leak-check=full 
            --show-leak-kinds=all 
            --track-origins=yes
            --error-exitcode=1
            ./unit_tests
        DEPENDS unit_tests
        COMMENT "Running unit tests with Valgrind"
    )
    
    add_custom_target(valgrind_integration_tests
        COMMAND ${VALGRIND_EXECUTABLE} 
            --leak-check=full 
            --show-leak-kinds=all 
            --track-origins=yes
            --error-exitcode=1
            ./integration_tests
        DEPENDS integration_tests
        COMMENT "Running integration tests with Valgrind"
    )
    
    message(STATUS "Valgrind test targets available: valgrind_unit_tests, valgrind_integration_tests")
endif()

message(STATUS "Test targets configured:")
message(STATUS "  unit_tests - Unit test executable")
message(STATUS "  integration_tests - Integration test executable") 
message(STATUS "  run_unit_tests - Run unit tests")
message(STATUS "  run_integration_tests - Run integration tests")
message(STATUS "  run_all_tests - Run all tests")