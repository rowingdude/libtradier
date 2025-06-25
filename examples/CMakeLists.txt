cmake_minimum_required(VERSION 3.20)

# Examples configuration
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find libtradier library
find_package(libtradier QUIET)

if(NOT libtradier_FOUND)
    # If not installed, use the parent target
    message(STATUS "Using libtradier from parent project")
    set(LIBTRADIER_TARGET tradier)
else()
    message(STATUS "Using installed libtradier")
    set(LIBTRADIER_TARGET libtradier::tradier)
endif()

# Common include directories for all examples
set(EXAMPLE_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# Helper function to create examples
function(add_example EXAMPLE_NAME SOURCE_FILE)
    add_executable(${EXAMPLE_NAME} ${SOURCE_FILE})
    
    target_include_directories(${EXAMPLE_NAME} PRIVATE ${EXAMPLE_INCLUDE_DIRS})
    target_link_libraries(${EXAMPLE_NAME} PRIVATE ${LIBTRADIER_TARGET})
    
    # Set output directory
    set_target_properties(${EXAMPLE_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
    )
    
    # Add to examples group
    set_target_properties(${EXAMPLE_NAME} PROPERTIES FOLDER "Examples")
endfunction()

# Helper function to conditionally add examples if they exist
function(add_example_if_exists EXAMPLE_NAME SOURCE_FILE)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}")
        add_example(${EXAMPLE_NAME} ${SOURCE_FILE})
        message(STATUS "  ✓ ${EXAMPLE_NAME}")
    else()
        message(STATUS "  - ${EXAMPLE_NAME} (source not found)")
    endif()
endfunction()

# Basic Examples
message(STATUS "Configuring basic examples...")

add_example_if_exists(account_info_fixed basic/account_info_fixed.cpp)
add_example_if_exists(simple_trading_fixed basic/simple_trading_fixed.cpp)
# Temporarily disable problematic examples until API interfaces are fixed
# add_example_if_exists(market_data basic/market_data.cpp)
# add_example_if_exists(quote_streamer basic/quote_streamer.cpp)

# Advanced Examples
message(STATUS "Configuring advanced examples...")

add_example_if_exists(trading_bot advanced/trading_bot.cpp)
add_example_if_exists(options_dashboard advanced/options_dashboard.cpp)
add_example_if_exists(portfolio_manager advanced/portfolio_manager.cpp)
add_example_if_exists(market_scanner advanced/market_scanner.cpp)

# Integration Examples
message(STATUS "Configuring integration examples...")

add_example_if_exists(error_handling integration/error_handling.cpp)
add_example_if_exists(async_patterns integration/async_patterns.cpp)
add_example_if_exists(config_management integration/config_management.cpp)
add_example_if_exists(streaming_setup integration/streaming_setup.cpp)

# Collect built targets for dependency management  
set(BASIC_EXAMPLES)
set(ADVANCED_EXAMPLES)
set(INTEGRATION_EXAMPLES)
set(ALL_EXAMPLES)

# Helper to add to target lists
function(track_example CATEGORY EXAMPLE_NAME SOURCE_FILE)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}")
        if(CATEGORY STREQUAL "basic")
            list(APPEND BASIC_EXAMPLES ${EXAMPLE_NAME})
            set(BASIC_EXAMPLES ${BASIC_EXAMPLES} PARENT_SCOPE)
        elseif(CATEGORY STREQUAL "advanced")
            list(APPEND ADVANCED_EXAMPLES ${EXAMPLE_NAME})
            set(ADVANCED_EXAMPLES ${ADVANCED_EXAMPLES} PARENT_SCOPE)
        elseif(CATEGORY STREQUAL "integration")
            list(APPEND INTEGRATION_EXAMPLES ${EXAMPLE_NAME})
            set(INTEGRATION_EXAMPLES ${INTEGRATION_EXAMPLES} PARENT_SCOPE)
        endif()
        list(APPEND ALL_EXAMPLES ${EXAMPLE_NAME})
        set(ALL_EXAMPLES ${ALL_EXAMPLES} PARENT_SCOPE)
    endif()
endfunction()

# Track which examples actually exist
track_example(basic account_info_fixed basic/account_info_fixed.cpp)
track_example(basic simple_trading_fixed basic/simple_trading_fixed.cpp)
# track_example(basic market_data basic/market_data.cpp)
# track_example(basic quote_streamer basic/quote_streamer.cpp)

track_example(advanced trading_bot advanced/trading_bot.cpp)
track_example(advanced options_dashboard advanced/options_dashboard.cpp)
track_example(advanced portfolio_manager advanced/portfolio_manager.cpp)
track_example(advanced market_scanner advanced/market_scanner.cpp)

track_example(integration error_handling integration/error_handling.cpp)
track_example(integration async_patterns integration/async_patterns.cpp)
track_example(integration config_management integration/config_management.cpp)
track_example(integration streaming_setup integration/streaming_setup.cpp)

# Custom targets for running example groups
if(BASIC_EXAMPLES)
    add_custom_target(examples_basic
        DEPENDS ${BASIC_EXAMPLES}
        COMMENT "Basic examples: ${BASIC_EXAMPLES}"
    )
endif()

if(ADVANCED_EXAMPLES)
    add_custom_target(examples_advanced
        DEPENDS ${ADVANCED_EXAMPLES}  
        COMMENT "Advanced examples: ${ADVANCED_EXAMPLES}"
    )
endif()

if(INTEGRATION_EXAMPLES)
    add_custom_target(examples_integration
        DEPENDS ${INTEGRATION_EXAMPLES}
        COMMENT "Integration examples: ${INTEGRATION_EXAMPLES}"
    )
endif()

if(ALL_EXAMPLES)
    add_custom_target(examples_all
        DEPENDS ${ALL_EXAMPLES}
        COMMENT "Build all available example applications"
    )
endif()

# Installation targets for examples
if(LIBTRADIER_INSTALL_EXAMPLES)
    install(TARGETS 
        account_info market_data simple_trading quote_streamer
        trading_bot options_dashboard portfolio_manager market_scanner
        error_handling async_patterns config_management streaming_setup
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/libtradier-examples
    )
    
    # Install example source code for reference
    install(DIRECTORY basic advanced integration
        DESTINATION ${CMAKE_INSTALL_DOCDIR}/examples
        FILES_MATCHING PATTERN "*.cpp" PATTERN "*.hpp"
    )
endif()

message(STATUS "Examples configured:")
message(STATUS "  Basic: 4 examples")
message(STATUS "  Advanced: 4 examples") 
message(STATUS "  Integration: 4 examples")
message(STATUS "  Build target: examples_all")