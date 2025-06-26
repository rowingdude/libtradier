# libtradier - Tradier API C++ Library

A comprehensive C++ library for interfacing with the Tradier brokerage API, providing full access to trading, market data, account management, and streaming capabilities.

## Overview

libtradier is a modern C++20 library that enables developers to integrate Tradier's trading platform into their applications. The library supports both synchronous and asynchronous operations, real-time market data streaming, and comprehensive order management functionality.

## Features

### Core Functionality
- Complete Tradier API coverage for trading operations
- Account management and portfolio tracking
- Real-time and historical market data access
- Options chain data and analytics
- Watchlist management
- Corporate actions and dividend information

### Technical Capabilities
- Synchronous and asynchronous API calls
- WebSocket streaming for real-time data (6/24/25 - migrated from WebsocketPP to LibBoost.beast)
- Parallelized JSON processing - (6/25/25 - migrated from NlohmannJSON to SIMDJSON)
- Thread-safe operations with built-in thread pool

### Supported Environments
- Production and sandbox trading environments
- Multiple authentication methods
- Configurable endpoint management
- SSL/TLS secure communications

## Dependencies

### Core Runtime Dependencies

- [libcurl](https://curl.se/libcurl/) - HTTP client functionality
- [simdjson](https://github.com/simdjson/simdjson)) - JSON parsing and serialization
- [Boost](https://www.boost.org/) - System and thread libraries
- [OpenSSL](https://www.openssl.org/) - SSL/TLS support

### Development and Performance Testing Dependencies

- [Google Benchmark](https://github.com/google/benchmark) - Performance benchmarking framework
- [Google Perftools](https://github.com/gperftools/gperftools) - CPU profiling and performance analysis
- Linux perf tools - Low-level performance profiling
- Valgrind - Memory analysis and profiling
- ccache - Build caching for faster development
- ninja - High-performance build system

## Installation

### Prerequisites

Install the required system dependencies:

#### Ubuntu/Debian

**Core Dependencies:**
```bash
sudo apt update
sudo apt install build-essential cmake pkg-config
sudo apt install libcurl4-openssl-dev libsimdjson-dev
sudo apt install libboost-system-dev libboost-thread-dev libboost-url-dev
sudo apt install libssl-dev 
```

**If you're interested in furthering this library, it's requested that you use our included testbench and install these as well**
**Performance Development Tools (Optional):**
```bash
# Performance profiling and benchmarking
sudo apt install linux-tools-common linux-tools-generic linux-tools-$(uname -r)
sudo apt install valgrind google-perftools-dev libbenchmark-dev

# Build optimization
sudo apt install ccache ninja-build

# System monitoring
sudo apt install htop sysstat cpufrequtils
```

#### Building from Source

```bash
git clone https://github.com/your-username/libtradier.git
cd libtradier
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

### CMake Integration

Add to your CMakeLists.txt:

```cmake
find_package(libtradier REQUIRED)
target_link_libraries(your_target libtradier::tradier)
```

## Quick Start

### Basic Setup

```cpp
#include <tradier/client.hpp>
#include <iostream>

int main() {
    // Configure for sandbox environment
    tradier::Config config;
    config.setEnvironment(tradier::Environment::SANDBOX);
    config.setAccessToken("your-access-token");
    
    // Create client
    tradier::TradierClient client(config);
    
    // Get account information
    auto result = client.account().getProfile();
    if (result.isSuccess()) {
        auto profile = result.value();
        std::cout << "Account ID: " << profile.accountId << std::endl;
    }
    
    return 0;
}
```

### Market Data Access

```cpp
// Get real-time quotes
auto quotes = client.market().getQuotes({"AAPL", "MSFT", "GOOGL"});
if (quotes.isSuccess()) {
    for (const auto& quote : quotes.value()) {
        std::cout << quote.symbol << ": $" << quote.last << std::endl;
    }
}

// Get options chain
auto options = client.market().getOptionsChain("AAPL", "2024-01-19");
if (options.isSuccess()) {
    // Process options data
}
```

### Asynchronous Operations

```cpp
// Async market data with callback
client.market().getQuotesAsync({"AAPL"}, [](const auto& result) {
    if (result.isSuccess()) {
        auto quotes = result.value();
        // Process quotes in callback
    }
});

// Async with future
auto future = client.market().getQuotesAsync({"MSFT"});
auto result = future.get(); // Blocks until complete
```

### Trading Operations

```cpp
// Place a market order
tradier::OrderRequest order;
order.accountId = "your-account-id";
order.symbol = "AAPL";
order.side = tradier::OrderSide::BUY;
order.quantity = 100;
order.type = tradier::OrderType::MARKET;

auto result = client.trading().placeOrder(order);
if (result.isSuccess()) {
    std::cout << "Order placed: " << result.value().orderId << std::endl;
}
```

### WebSocket Streaming

```cpp
// Connect to real-time data stream
auto connection = client.streaming().connect("quotes");

connection.setMessageHandler([](const std::string& message) {
    // Process real-time market data
    std::cout << "Received: " << message << std::endl;
});

connection.connect();
// Stream data...
connection.disconnect();
```

## Configuration

### Environment Setup

```cpp
tradier::Config config;

// Production environment
config.setEnvironment(tradier::Environment::PRODUCTION);
config.setBaseUrl("https://api.tradier.com");

// Sandbox environment  
config.setEnvironment(tradier::Environment::SANDBOX);
config.setBaseUrl("https://sandbox.tradier.com");

// Authentication
config.setAccessToken("your-access-token");
config.setClientId("your-client-id");
config.setClientSecret("your-client-secret");
```

### Rate Limiting

```cpp
// Configure rate limiting (120 requests per minute default)
config.setRateLimit(120, std::chrono::minutes(1));

// Custom timeout settings
config.setTimeout(std::chrono::seconds(30));
```

## Error Handling

The library uses a Result pattern for comprehensive error handling:

```cpp
auto result = client.market().getQuote("AAPL");

if (result.isSuccess()) {
    auto quote = result.value();
    // Use quote data
} else {
    auto error = result.error();
    std::cerr << "Error " << error.statusCode 
              << ": " << error.message << std::endl;
}
```

## Documentation

### API Reference

Complete API documentation is available in the include directory headers. Key namespaces include:

- `tradier::market` - Market data operations
- `tradier::trading` - Order management and trading
- `tradier::account` - Account and portfolio management
- `tradier::streaming` - Real-time data streaming
- `tradier::auth` - Authentication and authorization

### Error Codes

The library provides specific error types for different failure scenarios:

- `ValidationError` - Input parameter validation failures
- `AuthenticationError` - Authentication and authorization issues
- `ConnectionError` - Network and connectivity problems
- `RateLimitError` - API rate limit exceeded
- `ApiError` - General API response errors

## Development

### Building with Debug Support

```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_DEBUG_LOGGING=ON ..
make
```

### Testing

```bash
cmake -DBUILD_TESTS=ON ..
make
ctest
```

### Memory Analysis

```bash
cmake -DENABLE_VALGRIND_TESTS=ON ..
make valgrind-test
```

## License

This project is licensed under the MIT License. See the LICENSE file for details.

## Contributing

Contributions are welcome. Please ensure all tests pass and follow the existing code style before submitting pull requests.

## Support

For issues and questions:

- GitHub Issues: Report bugs and feature requests
- Documentation: Refer to header files for detailed API documentation
- Tradier API Documentation: [https://documentation.tradier.com/](https://documentation.tradier.com/)

## Disclaimer

This software is provided free of charge under the MIT License. Users assume all responsibility for trading decisions and financial outcomes. The authors are not liable for any losses incurred through the use of this software.
