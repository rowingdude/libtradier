#!/bin/bash

# libtradier Performance Benchmark Runner
# Specialized script for performance testing and regression detection
# Author: libtradier team
# Version: 1.0

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Configuration
BENCHMARK_DIR="benchmarks"
RESULTS_DIR="benchmark_results"
BASELINE_FILE="performance_baseline.json"
BUILD_DIR="build"

# Benchmark settings
BENCHMARK_REPETITIONS=3
BENCHMARK_MIN_TIME=1.0
BENCHMARK_THREADS=$(nproc)

print_header() {
    echo -e "\n${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

print_info() {
    echo -e "${BLUE}ℹ $1${NC}"
}

setup_cpu_performance() {
    print_header "Setting up CPU for Benchmarking"
    
    # Disable CPU frequency scaling for consistent results
    if [[ -f /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor ]]; then
        local original_governor
        original_governor=$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor)
        print_info "Current CPU governor: $original_governor"
        
        if [[ "$original_governor" != "performance" ]]; then
            if echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor >/dev/null 2>&1; then
                print_success "Set CPU governor to performance mode"
                
                # Save original governor for restoration
                echo "$original_governor" > .original_cpu_governor
            else
                print_warning "Could not set CPU governor to performance mode"
            fi
        else
            print_success "CPU already in performance mode"
        fi
    else
        print_info "CPU frequency scaling not available"
    fi
    
    # Disable CPU idle states if possible
    if [[ -d /sys/devices/system/cpu/cpu0/cpuidle ]]; then
        print_info "Disabling CPU idle states for consistent timing"
        for cpu_idle in /sys/devices/system/cpu/cpu*/cpuidle/state*/disable; do
            if [[ -w "$cpu_idle" ]]; then
                echo 1 | sudo tee "$cpu_idle" >/dev/null 2>&1 || true
            fi
        done
    fi
    
    # Set CPU affinity to isolated cores if available
    local isolated_cpus
    isolated_cpus=$(cat /sys/devices/system/cpu/isolated 2>/dev/null || echo "")
    if [[ -n "$isolated_cpus" ]]; then
        print_info "Isolated CPUs available: $isolated_cpus"
        export CPU_AFFINITY="$isolated_cpus"
    fi
}

restore_cpu_settings() {
    print_info "Restoring original CPU settings"
    
    if [[ -f .original_cpu_governor ]]; then
        local original_governor
        original_governor=$(cat .original_cpu_governor)
        
        if echo "$original_governor" | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor >/dev/null 2>&1; then
            print_success "Restored CPU governor to $original_governor"
        fi
        
        rm -f .original_cpu_governor
    fi
    
    # Re-enable CPU idle states
    if [[ -d /sys/devices/system/cpu/cpu0/cpuidle ]]; then
        for cpu_idle in /sys/devices/system/cpu/cpu*/cpuidle/state*/disable; do
            if [[ -w "$cpu_idle" ]]; then
                echo 0 | sudo tee "$cpu_idle" >/dev/null 2>&1 || true
            fi
        done
    fi
}

build_benchmarks() {
    print_header "Building Performance Benchmarks"
    
    if [[ ! -d "$BUILD_DIR" ]]; then
        mkdir -p "$BUILD_DIR"
    fi
    
    cd "$BUILD_DIR"
    
    # Configure with optimizations
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_BENCHMARKS=ON \
        -DENABLE_NATIVE_OPTIMIZATION=ON \
        -DCMAKE_CXX_FLAGS="-O3 -march=native -mtune=native -flto" \
        ..
    
    # Build with maximum optimization
    make -j"$BENCHMARK_THREADS" benchmarks
    
    cd ..
    
    print_success "Benchmarks built successfully"
}

run_micro_benchmarks() {
    print_header "Running Micro-Benchmarks"
    
    local timestamp
    timestamp=$(date +%Y%m%d_%H%M%S)
    
    mkdir -p "$RESULTS_DIR"
    
    # Find benchmark executables
    local benchmark_executables
    benchmark_executables=$(find "$BUILD_DIR" -name "*benchmark*" -type f -executable || echo "")
    
    if [[ -z "$benchmark_executables" ]]; then
        print_warning "No benchmark executables found"
        return 0
    fi
    
    while IFS= read -r benchmark_exe; do
        local benchmark_name
        benchmark_name=$(basename "$benchmark_exe")
        
        print_info "Running $benchmark_name..."
        
        local output_file="$RESULTS_DIR/${benchmark_name}_${timestamp}.json"
        
        # Run benchmark with optimal settings
        if [[ -n "$CPU_AFFINITY" ]]; then
            taskset -c "$CPU_AFFINITY" "$benchmark_exe" \
                --benchmark_format=json \
                --benchmark_repetitions="$BENCHMARK_REPETITIONS" \
                --benchmark_min_time="$BENCHMARK_MIN_TIME" \
                --benchmark_display_aggregates_only=true \
                --benchmark_out="$output_file"
        else
            "$benchmark_exe" \
                --benchmark_format=json \
                --benchmark_repetitions="$BENCHMARK_REPETITIONS" \
                --benchmark_min_time="$BENCHMARK_MIN_TIME" \
                --benchmark_display_aggregates_only=true \
                --benchmark_out="$output_file"
        fi
        
        print_success "Completed $benchmark_name - results saved to $output_file"
        
    done <<< "$benchmark_executables"
}

run_system_benchmarks() {
    print_header "Running System-Level Benchmarks"
    
    # Memory bandwidth test
    print_info "Testing memory bandwidth..."
    if command -v stream >/dev/null 2>&1; then
        stream | tee "$RESULTS_DIR/memory_bandwidth_$(date +%Y%m%d_%H%M%S).txt"
    else
        print_warning "STREAM benchmark not available"
    fi
    
    # Cache performance test
    print_info "Testing cache performance..."
    if [[ -f "$BUILD_DIR/cache_benchmark" ]]; then
        "$BUILD_DIR/cache_benchmark" --benchmark_format=console
    fi
    
    # Disk I/O test (if relevant for market data processing)
    print_info "Testing disk I/O performance..."
    local temp_file="/tmp/libtradier_io_test"
    local io_size="100M"
    
    # Sequential write test
    dd if=/dev/zero of="$temp_file" bs=1M count=100 conv=fdatasync 2>&1 | \
        grep -E "(copied|transferred)" | tee "$RESULTS_DIR/disk_io_$(date +%Y%m%d_%H%M%S).txt"
    
    # Sequential read test
    dd if="$temp_file" of=/dev/null bs=1M 2>&1 | \
        grep -E "(copied|transferred)" | tee -a "$RESULTS_DIR/disk_io_$(date +%Y%m%d_%H%M%S).txt"
    
    rm -f "$temp_file"
}

analyze_performance_regression() {
    print_header "Performance Regression Analysis"
    
    if [[ ! -f "$BASELINE_FILE" ]]; then
        print_warning "No performance baseline found"
        print_info "Creating new baseline from current results"
        
        # Create baseline from latest results
        local latest_results
        latest_results=$(find "$RESULTS_DIR" -name "*.json" -type f -printf '%T@ %p\n' | \
            sort -rn | head -1 | cut -d' ' -f2-)
        
        if [[ -n "$latest_results" ]]; then
            cp "$latest_results" "$BASELINE_FILE"
            print_success "Baseline created: $BASELINE_FILE"
        fi
        
        return 0
    fi
    
    print_info "Comparing against baseline: $BASELINE_FILE"
    
    # Find latest benchmark results
    local latest_results
    latest_results=$(find "$RESULTS_DIR" -name "*.json" -type f -printf '%T@ %p\n' | \
        sort -rn | head -5)
    
    local regression_found=false
    local improvement_found=false
    
    # Simple regression detection (would be enhanced with proper JSON parsing)
    while IFS= read -r result_info; do
        local result_file
        result_file=$(echo "$result_info" | cut -d' ' -f2-)
        
        if [[ -f "$result_file" ]]; then
            print_info "Analyzing: $(basename "$result_file")"
            
            # Extract timing information (simplified)
            local current_times
            current_times=$(grep -o '"real_time":[0-9.]*' "$result_file" | cut -d':' -f2 || echo "")
            
            local baseline_times
            baseline_times=$(grep -o '"real_time":[0-9.]*' "$BASELINE_FILE" | cut -d':' -f2 || echo "")
            
            if [[ -n "$current_times" && -n "$baseline_times" ]]; then
                # Simple comparison (would use proper statistical analysis in production)
                local avg_current
                avg_current=$(echo "$current_times" | awk '{sum+=$1; count++} END {print sum/count}')
                
                local avg_baseline
                avg_baseline=$(echo "$baseline_times" | awk '{sum+=$1; count++} END {print sum/count}')
                
                if (( $(echo "$avg_current > $avg_baseline * 1.1" | bc -l) )); then
                    print_warning "Potential regression detected in $(basename "$result_file")"
                    print_info "Current: ${avg_current}ns, Baseline: ${avg_baseline}ns"
                    regression_found=true
                elif (( $(echo "$avg_current < $avg_baseline * 0.9" | bc -l) )); then
                    print_success "Performance improvement detected in $(basename "$result_file")"
                    print_info "Current: ${avg_current}ns, Baseline: ${avg_baseline}ns"
                    improvement_found=true
                fi
            fi
        fi
    done <<< "$latest_results"
    
    if [[ "$regression_found" == true ]]; then
        print_warning "Performance regressions detected - review required"
        return 1
    elif [[ "$improvement_found" == true ]]; then
        print_success "Performance improvements detected!"
    else
        print_success "Performance stable compared to baseline"
    fi
    
    return 0
}

generate_performance_report() {
    print_header "Generating Performance Report"
    
    local report_file="performance_report_$(date +%Y%m%d_%H%M%S).html"
    
    cat > "$report_file" << 'EOF'
<!DOCTYPE html>
<html>
<head>
    <title>libtradier Performance Report</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; }
        .header { background-color: #f0f0f0; padding: 20px; border-radius: 5px; }
        .section { margin: 20px 0; }
        .benchmark { background-color: #f9f9f9; padding: 15px; margin: 10px 0; border-radius: 3px; }
        .regression { background-color: #ffe6e6; }
        .improvement { background-color: #e6ffe6; }
        table { border-collapse: collapse; width: 100%; }
        th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
        th { background-color: #f2f2f2; }
    </style>
</head>
<body>
    <div class="header">
        <h1>libtradier Performance Report</h1>
        <p>Generated: $(date)</p>
        <p>System: $(uname -a)</p>
        <p>CPU: $(lscpu | grep "Model name" | cut -d: -f2 | xargs)</p>
    </div>
    
    <div class="section">
        <h2>Benchmark Results Summary</h2>
        <p>Latest benchmark execution results and performance analysis.</p>
    </div>
EOF
    
    # Add benchmark results to report
    if [[ -d "$RESULTS_DIR" ]]; then
        echo "    <div class=\"section\">" >> "$report_file"
        echo "        <h3>Recent Benchmark Files</h3>" >> "$report_file"
        echo "        <ul>" >> "$report_file"
        
        find "$RESULTS_DIR" -name "*.json" -type f -printf '%T@ %p\n' | \
            sort -rn | head -10 | while IFS= read -r file_info; do
            local file_path
            file_path=$(echo "$file_info" | cut -d' ' -f2-)
            local file_name
            file_name=$(basename "$file_path")
            local file_date
            file_date=$(date -d "@$(echo "$file_info" | cut -d' ' -f1)" '+%Y-%m-%d %H:%M:%S')
            
            echo "            <li><strong>$file_name</strong> - $file_date</li>" >> "$report_file"
        done
        
        echo "        </ul>" >> "$report_file"
        echo "    </div>" >> "$report_file"
    fi
    
    cat >> "$report_file" << 'EOF'
    <div class="section">
        <h2>Performance Trends</h2>
        <p>Placeholder for performance trend charts (future enhancement)</p>
    </div>
    
    <div class="section">
        <h2>System Information</h2>
        <table>
            <tr><th>Metric</th><th>Value</th></tr>
EOF
    
    # Add system information
    echo "            <tr><td>CPU Cores</td><td>$(nproc)</td></tr>" >> "$report_file"
    echo "            <tr><td>Memory</td><td>$(free -h | grep '^Mem:' | awk '{print $2}')</td></tr>" >> "$report_file"
    echo "            <tr><td>Compiler</td><td>$(g++ --version | head -1)</td></tr>" >> "$report_file"
    
    cat >> "$report_file" << 'EOF'
        </table>
    </div>
</body>
</html>
EOF
    
    print_success "Performance report generated: $report_file"
}

cleanup_benchmark_environment() {
    print_header "Cleaning Up Benchmark Environment"
    
    restore_cpu_settings
    
    # Clean up temporary files
    rm -f .original_cpu_governor
    
    print_success "Cleanup completed"
}

usage() {
    cat << EOF
Usage: $0 [OPTIONS]

Specialized performance benchmarking script for libtradier

OPTIONS:
    -h, --help              Show this help message
    -b, --build             Build benchmarks before running
    -s, --setup-cpu         Setup CPU for optimal benchmarking
    -m, --micro-only        Run only micro-benchmarks
    -S, --system-only       Run only system-level benchmarks
    -r, --regression        Run regression analysis only
    -R, --report            Generate performance report only
    --baseline <file>       Use custom baseline file
    --results-dir <dir>     Use custom results directory

EXAMPLES:
    $0 -b -s               # Build and run full benchmark suite
    $0 -m                  # Run only micro-benchmarks
    $0 -r                  # Run regression analysis
    $0 --baseline old.json # Compare against specific baseline

EOF
}

main() {
    local build_benchmarks=false
    local setup_cpu=false
    local micro_only=false
    local system_only=false
    local regression_only=false
    local report_only=false
    
    # Parse arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                usage
                exit 0
                ;;
            -b|--build)
                build_benchmarks=true
                shift
                ;;
            -s|--setup-cpu)
                setup_cpu=true
                shift
                ;;
            -m|--micro-only)
                micro_only=true
                shift
                ;;
            -S|--system-only)
                system_only=true
                shift
                ;;
            -r|--regression)
                regression_only=true
                shift
                ;;
            -R|--report)
                report_only=true
                shift
                ;;
            --baseline)
                BASELINE_FILE="$2"
                shift 2
                ;;
            --results-dir)
                RESULTS_DIR="$2"
                shift 2
                ;;
            *)
                echo "Unknown option: $1"
                usage
                exit 1
                ;;
        esac
    done
    
    print_header "libtradier Performance Benchmark Suite"
    
    # Trap signals for cleanup
    trap cleanup_benchmark_environment EXIT INT TERM
    
    if [[ "$report_only" == true ]]; then
        generate_performance_report
        exit 0
    fi
    
    if [[ "$regression_only" == true ]]; then
        analyze_performance_regression
        exit $?
    fi
    
    if [[ "$setup_cpu" == true ]]; then
        setup_cpu_performance
    fi
    
    if [[ "$build_benchmarks" == true ]]; then
        build_benchmarks
    fi
    
    if [[ "$system_only" != true ]]; then
        run_micro_benchmarks
    fi
    
    if [[ "$micro_only" != true ]]; then
        run_system_benchmarks
    fi
    
    analyze_performance_regression
    local regression_exit_code=$?
    
    generate_performance_report
    
    print_header "Benchmark Suite Completed"
    
    if [[ "$regression_exit_code" -ne 0 ]]; then
        print_warning "Performance regressions detected - review required"
        exit 1
    else
        print_success "All benchmarks completed successfully"
        exit 0
    fi
}

# Run main function
main "$@"