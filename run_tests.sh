#!/bin/bash

# Tradier API Library - Comprehensive Test Runner
# This script runs all example tests and provides a summary of results

set -e  

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' 

BUILD_DIR="build"
BIN_DIR="$BUILD_DIR/bin"
LOG_DIR="test_logs"

mkdir -p "$LOG_DIR"

print_color() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}

print_header() {
    echo
    print_color $BLUE "$(printf '=%.0s' {1..60})"
    print_color $BLUE "$1"
    print_color $BLUE "$(printf '=%.0s' {1..60})"
    echo
}

run_test() {
    local test_name=$1
    local test_binary=$2
    local description=$3
    local log_file="$LOG_DIR/${test_name}.log"
    
    print_color $CYAN "Running: $description"
    echo "Binary: $test_binary"
    echo "Log: $log_file"
    echo
    
    if [ ! -f "$test_binary" ]; then
        print_color $RED "❌ Binary not found: $test_binary"
        echo "FAILED - Binary not found" > "$log_file"
        return 1
    fi
    
    if timeout 120s "$test_binary" > "$log_file" 2>&1; then
        print_color $GREEN "✅ $test_name completed successfully"
        return 0
    else
        local exit_code=$?
        print_color $RED "❌ $test_name failed (exit code: $exit_code)"
        echo "Last 10 lines of output:"
        tail -n 10 "$log_file" | sed 's/^/   /'
        return 1
    fi
}

check_environment() {
    print_header "ENVIRONMENT CHECK"
    
    local missing_vars=()
    
    if [ -z "$TRADIER_SBX_TOKEN" ] && [ -z "$TRADIER_PROD_TOKEN" ]; then
        missing_vars+=("TRADIER_SBX_TOKEN or TRADIER_PROD_TOKEN")
    fi
    
    if [ ${#missing_vars[@]} -gt 0 ]; then
        print_color $RED "❌ Missing required environment variables:"
        for var in "${missing_vars[@]}"; do
            echo "   - $var"
        done
        echo
        print_color $YELLOW "Please set your Tradier API credentials:"
        echo "   export TRADIER_SBX_TOKEN=your_sandbox_token"
        echo "   export TRADIER_SBX_ACCNUM=your_sandbox_account  # Optional"
        echo "   # OR for production:"
        echo "   export TRADIER_PROD_TOKEN=your_production_token"
        echo "   export TRADIER_SBX_ENABLE=false"
        return 1
    fi
    
    if [ "${TRADIER_SBX_ENABLE:-true}" != "false" ]; then
        print_color $YELLOW "🧪 Using SANDBOX environment"
        echo "   Token: ${TRADIER_SBX_TOKEN:0:12}..."
        [ -n "$TRADIER_SBX_ACCNUM" ] && echo "   Account: $TRADIER_SBX_ACCNUM"
    else
        print_color $PURPLE "🚀 Using PRODUCTION environment"
        echo "   Token: ${TRADIER_PROD_TOKEN:0:12}..."
    fi
    
    echo "   Timeout: ${TRADIER_API_TIMEOUT:-30} seconds"
    
    print_color $GREEN "✅ Environment configuration looks good"
    return 0
}

build_project() {
    print_header "BUILD PROJECT"
    
    if [ ! -d "$BUILD_DIR" ]; then
        print_color $CYAN "Creating build directory..."
        mkdir -p "$BUILD_DIR"
    fi
    
    cd "$BUILD_DIR"
    
    print_color $CYAN "Running CMake configuration..."
    if cmake .. > ../build.log 2>&1; then
        print_color $GREEN "✅ CMake configuration successful"
    else
        print_color $RED "❌ CMake configuration failed"
        echo "Build log:"
        cat ../build.log
        return 1
    fi
    
    print_color $CYAN "Building project..."
    if make -j$(nproc) >> ../build.log 2>&1; then
        print_color $GREEN "✅ Build successful"
    else
        print_color $RED "❌ Build failed"
        echo "Build log:"
        tail -n 20 ../build.log
        return 1
    fi
    
    cd ..
    return 0
}

run_all_tests() {
    print_header "RUNNING ALL TESTS"
    
    local tests_passed=0
    local tests_failed=0
    local failed_tests=()
    
    local tests=(
        "market_feature_test|$BIN_DIR/market_feature_test|Market Data Features (Quotes, Options, Historical)"
        "trading_example|$BIN_DIR/trading_example|Basic Trading Example"
        "trading_example_plus_plus|$BIN_DIR/trading_enhanced_test|Enhanced Trading Features (Preview, Options, Bracket Orders)"
        "streaming_feature_test|$BIN_DIR/streaming_feature_test|Real-time Streaming (Market & Account Events)"
        "watchlist_feature_test|$BIN_DIR/watchlist_feature_test|Watchlist Management (CRUD Operations)"
        "streaming_debug_test|$BIN_DIR/streaming_debug_test|Streaming Debug & Session Creation"
    )
    
    for test_info in "${tests[@]}"; do
        IFS='|' read -r test_name test_binary description <<< "$test_info"
        
        echo
        print_color $CYAN "$(printf '─%.0s' {1..40})"
        
        if run_test "$test_name" "$test_binary" "$description"; then
            ((tests_passed++))
        else
            ((tests_failed++))
            failed_tests+=("$test_name")
        fi
        
        echo
    done
    
    print_header "TEST SUMMARY"
    
    local total_tests=$((tests_passed + tests_failed))
    echo "Total Tests: $total_tests"
    print_color $GREEN "Passed: $tests_passed"
    
    if [ $tests_failed -gt 0 ]; then
        print_color $RED "Failed: $tests_failed"
        echo
        print_color $RED "Failed tests:"
        for test in "${failed_tests[@]}"; do
            echo "   ❌ $test"
        done
        echo
        print_color $YELLOW "Check individual log files in $LOG_DIR/ for details"
        return 1
    else
        print_color $GREEN "🎉 All tests passed!"
        return 0
    fi
}

cleanup() {
    print_header "CLEANUP"
    
    if [ "$1" = "--clean" ]; then
        print_color $CYAN "Cleaning build directory..."
        rm -rf "$BUILD_DIR"
        rm -rf "$LOG_DIR"
        rm -f build.log
        print_color $GREEN "✅ Cleanup complete"
    else
        print_color $YELLOW "Logs saved in $LOG_DIR/"
        print_color $YELLOW "Use $0 --clean to remove build artifacts"
    fi
}

show_usage() {
    echo "Tradier API Library Test Runner"
    echo
    echo "Usage: $0 [OPTIONS]"
    echo
    echo "Options:"
    echo "  --help          Show this help message"
    echo "  --clean         Clean build artifacts and logs"
    echo "  --build-only    Only build, don't run tests"
    echo "  --test-only     Only run tests (skip building)"
    echo "  --env-check     Only check environment setup"
    echo "  --sandbox       Force sandbox mode"
    echo "  --production    Force production mode"
    echo
    echo "Environment Variables:"
    echo "  TRADIER_SBX_TOKEN    Sandbox API token"
    echo "  TRADIER_SBX_ACCNUM   Sandbox account number (optional)"
    echo "  TRADIER_PROD_TOKEN   Production API token"
    echo "  TRADIER_SBX_ENABLE   Set to 'false' for production mode"
    echo "  TRADIER_API_TIMEOUT  API timeout in seconds (default: 30)"
    echo
    echo "Examples:"
    echo "  $0                    # Full test run"
    echo "  $0 --env-check       # Check environment only"
    echo "  $0 --sandbox         # Force sandbox mode"
    echo "  $0 --clean           # Clean up artifacts"
}

main() {
    local build_only=false
    local test_only=false
    local env_check_only=false
    
    while [[ $# -gt 0 ]]; do
        case $1 in
            --help)
                show_usage
                exit 0
                ;;
            --clean)
                cleanup --clean
                exit 0
                ;;
            --build-only)
                build_only=true
                shift
                ;;
            --test-only)
                test_only=true
                shift
                ;;
            --env-check)
                env_check_only=true
                shift
                ;;
            --sandbox)
                export TRADIER_SBX_ENABLE=true
                shift
                ;;
            --production)
                export TRADIER_SBX_ENABLE=false
                shift
                ;;
            *)
                print_color $RED "Unknown option: $1"
                show_usage
                exit 1
                ;;
        esac
    done
    
    print_color $PURPLE "$(figlet -f small 'Tradier Tests' 2>/dev/null || echo 'TRADIER API LIBRARY TEST RUNNER')"
    echo "$(date)"
    echo
    
    if ! check_environment; then
        exit 1
    fi
    
    if [ "$env_check_only" = true ]; then
        exit 0
    fi
    
    if [ "$test_only" != true ]; then
        if ! build_project; then
            exit 1
        fi
    fi
    
    if [ "$build_only" = true ]; then
        exit 0
    fi
    
    if ! run_all_tests; then
        cleanup
        exit 1
    fi
    
    cleanup
    print_color $GREEN "🚀 All tests completed successfully!"
}

trap 'echo; print_color $YELLOW "Test run interrupted"; cleanup; exit 130' INT

main "$@"