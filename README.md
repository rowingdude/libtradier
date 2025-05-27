# libtradier
*A modern C++ library for the Tradier API, providing comprehensive access to market data, trading, account management, and real-time streaming.

## Features

- **Market Data** - Real-time quotes, option chains, historical data, and market fundamentals
- **Trading** -  Order placement, modification, cancellation with support for equity and options trading
- **Account Management** -  Portfolio positions, balances, order history, and account information
- **Real-time Streaming** -  WebSocket-based market data and account event streaming
- **OAuth Authentication** -  Full OAuth 2.0 flow with PKCE support and automatic token refresh
- **Watchlists** -  Create and manage symbol watchlists
- **Cross-platform** - Linux, macOS, and Windows support

## API Coverage

- **Market Data** - Quotes, option chains, historical data, time & sales, market calendar
- **Trading** - Equity and options orders, bracket orders, multi-leg strategies
- **Account** - Positions, balances, order history, account details
- **Streaming** - Real-time market data and account events via WebSocket
- **Authentication** - Full OAuth 2.0 implementation with token management
- **Fundamentals (COMING SOON)** - Company information, financial ratios, corporate actions

# Installation
**Prerequisites**
The library requires the following dependencies:

libcurl, nlohmann/json, OpenSSL

I also built this library using BOOST framework, please have Boost installed also.

## Implemented but *Unavailable* Features:

- **Company Fundamentals**: Tradier has not made public the company fundamentals endpoints, they will return a 302 redirect as of this update.

## Quick Start

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.14 or higher
- Dependencies (available via package managers):
  - libcurl
  - nlohmann-json
  - Boost (system, filesystem, thread)
  - OpenSSL

### "Hello World" Examples:

_Simple market data_
```cpp
#include <tradier/client.hpp>
#include <tradier/market.hpp>

int main() {
    auto config = tradier::Config::fromEnvironment();
    tradier::TradierClient client(config);
    
    auto marketService = client.market();
    auto quote = marketService.getQuote("AAPL");
    
    if (quote) {
        std::cout << "AAPL: $" << quote->last.value_or(0.0) << std::endl;
    }
    
    return 0;
} // Note: Please ensure your environment is configured
```

_Buy shares_

```cpp
#include <tradier/client.hpp>
#include <tradier/trading.hpp>
#include <iostream>

int main() {
    try {
        auto config = tradier::Config::fromEnvironment();
        tradier::TradierClient client(config);
        
        // Buy 100 shares of AAPL at market price
        auto result = client.trading().buyStock(
            config.accountNumber, 
            "AAPL", 
            100.0
        );
        
        if (result) {
            std::cout << "Order placed successfully! ID: " 
                      << result->id << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}
```
_Real-time Streaming Example_
```cpp
#include <tradier/client.hpp>
#include <tradier/streaming.hpp>
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    try {
        auto config = tradier::Config::fromEnvironment();
        tradier::TradierClient client(config);
        auto streaming = client.streaming();
        
        // Create market data session
        auto sessionResult = streaming.createMarketSession();
        if (!sessionResult) {
            std::cerr << "Failed to create session\n";
            return 1;
        }
        
        // Subscribe to real-time trades
        streaming.subscribeToTrades(*sessionResult, {"AAPL", "MSFT"}, 
            [](const tradier::TradeEvent& trade) {
                std::cout << trade.symbol << ": $" << trade.price 
                          << " (Size: " << trade.size << ")\n";
            });
        
        // Keep streaming for 30 seconds
        std::this_thread::sleep_for(std::chrono::seconds(30));
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}
```
### Installation

#### Linux

**Ubuntu and derivatives**

    sudo apt install libcurl4-openssl-dev libssl-dev libboost-all-dev nlohmann-json3-dev

**CentOS/RHEL**
    sudo yum install gcc-c++ cmake libcurl-devel openssl-devel
    sudo yum install nlohmann-json-devel  # or build from source if not available

**Fedora**
    sudo dnf install gcc-c++ cmake libcurl-devel openssl-devel nlohmann-json-devel

**Arch**
    sudo pacman -S base-devel cmake curl openssl nlohmann-json

#### macOS (Homebrew)

    brew install cmake curl openssl nlohmann-json boost

#### Windows (vcpkg)

    vcpkg install curl openssl nlohmann-json boost-system boost-filesystem boost-thread

#### Build the project!

    git clone https://github.com/rowingdude/libtradier.git
    cd libtradier
    mkdir build && cd build
    cmake ..
    make -j$(nproc)
    sudo make install

#### CMake Integration

I recommend using CMake as I've done in this project, adding this library to your project is pretty easy:

    find_package(libtradier REQUIRED)
    target_link_libraries(your_target PRIVATE tradier::tradier)

## Configuration

LibTradier is set to use environment variables for all settings, they are identified in config.cpp if you want to change them, but briefly they are:

    Enable or disable Sandbox mode      TRADIER_SBX_ENABLE          Expected response is "yes, no, 1, 0" respectively
    (Optional) API Timeout              TRADIER_API_TIMEOUT         Set equal to a number in seconds to wait before timing the API out

    Sandbox Account Number              TRADIER_SBX_ACCNUM          Expected here is a Tradier Sandbox account, currently they start with VA...
    Sandbox Account Token               TRADIER_SBX_TOKEN           Expected here is the Tradier Sandbox access token

    Production Access Token             TRADIER_PROD_TOKEN          Expected here is the Tradier primary access token

As a reminder, these are deployed on *nix (BASH shells) via

    export <var>=<value> such as export TRADIER_SBX_ENABLE="true"

On Windows (in Powershell), you can set Environment Variables using the `System.Environment` Class:

    [System.Environment]::SetEnvironmentVariable('TRADIER_SBX_ENABLE','true')


### Error Handling & Thread Safety
With all of the talk around the "danger" of C related languages, I built this project with safety in mind. Therefore, the library uses a Result<T> pattern for robust error handling ...

```cpp
auto result = client.market().getQuote("AAPL");
if (result) {
    std::cout << "Price: " << result->last.value_or(0.0) << std::endl;
} else {
    std::cerr << "Failed: " << result.error().what() << std::endl;
}
```

And then client and streaming service operations manage their own threads. We also use thread containerization for concurrent API calls such as when making bracket orders or when fetching data and simultaenously sending orders in.


## Contributing

I ask that anyone making contributions use this format

1. Fork the repository
2. Create a feature/fix branch (git checkout -b feature/amazing-feature)
3. Commit your changes (git commit -m 'Add amazing feature')
4. Push to the branch (git push origin feature/amazing-feature)
5. Open a Pull Request

So if you want to add debugging to src/market.cpp, you'd title your PR `add_market_debug` and then elaborate in your PR description.

## WHen sending in a PR, please...

1. Name your operating system and version (eg. Arch Linux, Kernel 6.12.24)
2. State your versions of installed component libraries (eg. CUrl 8.13.0 )
3. A code example that produces the error

I do check my repos frequently, so your Issues will be handled quickly.

# License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.

# Disclaimer
This software is provided "as is" without warranty. The authors are not liable for any claims or damages arising from its use. Please review the code carefully before using in production or live trading environments.
