# libtradier Debugging and Memory Safety Guide

## Building with Sanitizers

The library now includes built-in support for various memory and thread debugging tools through CMake options.

### Available Sanitizers

```bash
# Enable AddressSanitizer (detects memory errors)
cmake -DENABLE_ASAN=ON ..

# Enable ThreadSanitizer (detects race conditions)  
cmake -DENABLE_TSAN=ON ..

# Enable MemorySanitizer (detects uninitialized memory)
cmake -DENABLE_MSAN=ON ..

# Enable UndefinedBehaviorSanitizer (detects undefined behavior)
cmake -DENABLE_UBSAN=ON ..

# Enable debug logging
cmake -DENABLE_DEBUG_LOGGING=ON ..
```

### Example Usage

```bash
# Build with AddressSanitizer
mkdir build-debug && cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON ..
make -j$(nproc)

# Run your application - any memory errors will be reported
./your_application
```

## Built-in Testing Targets

The library includes automated testing targets for debugging tools:

### Valgrind Testing
```bash
# Enable Valgrind testing during build
cmake -DENABLE_VALGRIND_TESTS=ON -DBUILD_TESTS=ON ..
make -j$(nproc)

# Available targets:
make valgrind-test       # Memory leak detection
make helgrind-test       # Thread safety analysis  
make cachegrind-test     # Cache profiling
make memory-safety-test  # Combined memory + thread safety

# Manual cleanup if needed
make clean-test-artifacts
```

### GDB Testing
```bash
# Enable GDB support during build
cmake -DENABLE_GDB_SUPPORT=ON -DBUILD_TESTS=ON ..
make -j$(nproc)

# Available targets:
make gdb-test            # Crash analysis with backtrace
make gdb-core-analysis   # Core dump generation and analysis

# Manual cleanup if needed
make clean-test-artifacts
```

### Combined Usage
```bash
# Enable both Valgrind and GDB testing
cmake -DENABLE_VALGRIND_TESTS=ON -DENABLE_GDB_SUPPORT=ON -DBUILD_TESTS=ON ..
make -j$(nproc)

# Run comprehensive testing
make memory-safety-test
```

## Memory Leak Detection with Valgrind

```bash
# Compile with debug symbols
g++ -g -O0 your_code.cpp -ltradier

# Run with Valgrind
valgrind --leak-check=full --show-leak-kinds=all ./your_application
```

## Thread Safety Analysis

```bash
# Build with ThreadSanitizer
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_TSAN=ON ..
make -j$(nproc)

# Or use Valgrind's Helgrind
valgrind --tool=helgrind ./your_application
```

## Input Validation

The library now includes comprehensive input validation:

```cpp
#include <tradier/common/validation.hpp>

// All validation functions throw ValidationError on failure
validation::Validator::requireValidSymbol("AAPL");      // Valid
validation::Validator::requireValidSymbol("");          // Throws: symbol cannot be empty
validation::Validator::requireValidSymbol("TOOLONG");   // Throws: Symbol cannot exceed 10 characters

validation::Validator::requireValidOrderId(12345);      // Valid  
validation::Validator::requireValidOrderId(-1);         // Throws: Order ID must be positive

validation::Validator::requireValidQuantity(100.0);     // Valid
validation::Validator::requireValidQuantity(-10.0);     // Throws: Quantity must be positive

validation::Validator::requireValidPrice(50.25);        // Valid
validation::Validator::requireValidPrice(-5.0);         // Throws: Price cannot be negative
```

## Error Handling Best Practices

### Specific Exception Types

The library now uses specific exception types instead of generic `std::runtime_error`:

```cpp
try {
    auto result = client.market().getQuote("INVALID_SYMBOL");
} catch (const ValidationError& e) {
    // Handle validation errors (bad input)
    std::cerr << "Input validation failed: " << e.what() << std::endl;
} catch (const ParseError& e) {
    // Handle JSON parsing errors
    std::cerr << "Response parsing failed: " << e.what() << std::endl;
} catch (const NetworkError& e) {
    // Handle network-related errors
    std::cerr << "Network error: " << e.what() << std::endl;
} catch (const ApiError& e) {
    // Handle API-specific errors
    std::cerr << "API error " << e.statusCode << ": " << e.what() << std::endl;
}
```

### Using ApiResult Pattern

```cpp
auto result = client.market().getQuote("AAPL");
if (result.isSuccess()) {
    std::cout << "Price: $" << result.value().last.value_or(0.0) << std::endl;
} else {
    std::cerr << "Error: " << result.error().what() << std::endl;
    
    // Check if retryable
    if (result.isRetryable()) {
        std::cout << "This error may be temporary, consider retrying" << std::endl;
    }
}
```

## Performance Profiling

### CPU Profiling with perf

```bash
# Install perf tools (Ubuntu)
sudo apt install linux-tools-common linux-tools-generic

# Profile your application
perf record ./your_application
perf report
```

### Memory Profiling

```bash
# Use Valgrind's Massif for heap profiling
valgrind --tool=massif ./your_application
ms_print massif.out.* | head -50
```

## Debugging Crashes

### Core Dumps

```bash
# Enable core dumps
ulimit -c unlimited

# Run with GDB when crashed
gdb ./your_application core
(gdb) bt        # Show backtrace
(gdb) info registers
(gdb) list      # Show source around crash
```

### Live Debugging

```bash
# Attach GDB to running process
gdb -p $(pidof your_application)

# Or start with GDB
gdb ./your_application
(gdb) run
(gdb) bt        # When it crashes
```

## Common Issues and Solutions

### ThreadPool Memory Corruption
**Fixed**: The library now uses proper atomic operations and safe shared pointer management.

### WebSocket Race Conditions  
**Fixed**: All connection state changes now use proper memory ordering and synchronization.

### Silent JSON Parsing Failures
**Fixed**: All parsing errors are now properly reported with specific error messages.

### Validation Bypass
**Fixed**: Comprehensive input validation is now applied at all API entry points.

## Environment Variables

```bash
# Enable detailed debugging (if compiled with ENABLE_DEBUG_LOGGING)
export LIBTRADIER_DEBUG=1

# AddressSanitizer options
export ASAN_OPTIONS=abort_on_error=1:check_initialization_order=1

# ThreadSanitizer options  
export TSAN_OPTIONS=halt_on_error=1:second_deadlock_stack=1
```