# libTradier - C++ Tradier API Library

A modern C++17 library for the Tradier brokerage API, providing comprehensive access to market data, trading, account management, streaming, and watchlist functionality.

## Features

- **Market Data**: Real-time quotes, options chains, historical data, and market information
- **Account Management**: Portfolio positions, balances, order history, and account details
- **Trading**: Order placement, modification, and cancellation with full order lifecycle support
- **Streaming**: Real-time market data and account event streams
- **Watchlists**: Create, manage, and monitor custom symbol lists
- **Dual Environment**: Seamless sandbox and production environment support

## Quick Start

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.14 or higher
- Dependencies (available via package managers):
  - libcurl
  - nlohmann-json
  - Boost (system, filesystem, thread)
  - OpenSSL

### Installation

#### Ubuntu/Debian

    sudo apt install libcurl4-openssl-dev libssl-dev libboost-all-dev nlohmann-json3-dev

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

## Usage Examples

1. Gather basic market data ...

```
    #include "tradier/client.hpp"
    #include "tradier/market.hpp"

    auto config = tradier::Config::fromEnvironment();
    tradier::TradierClient client(config);
    auto market = client.market();

    auto quote = market.getQuote("AAPL");
    if (quote) {
        std::cout << "AAPL: $" << *quote->last << std::endl;
    }

    std::vector<std::string> symbols = {"AAPL", "MSFT", "GOOGL"};
    auto quotes = market.getQuotes(symbols);
```

2. Get account information ...

```
    auto accounts = client.accounts();
    auto profile = accounts.getProfile();
    auto balances = accounts.getBalances(profile->accounts[0].number);

    std::cout << "Buying Power: $" << balances->buyingPower << std::endl;
```

3. Stream Data ...

```
    auto streaming = client.streaming();
    auto session = streaming.createMarketSession();

    streaming.connectMarket(*session, {"AAPL", "SPY"}, 
        [](const tradier::MarketEvent& event) {
            std::cout << event.symbol << ": $" << event.price << std::endl;
        });
```

## Error Handling

I have tried to maintain a pretty clean accessible interface through the codebase. Accordingly, I have tried to make errors recognizable and specific. I use structured error handling throughout the code, though it is an area for improvement!

## Testing

To keep everything working and cohesive, I have assembled a few test programs in `examples/` where we guarentee basic library functionality. If you are going to submit code revisions, please make sure that these basic tests function BEFORE you submit a PR

## Architecture
libTradier is my introduction and first lesson in modern C++ design principles, as I'm coming from C!

## Contributing

I ask that anyone making contributions use this format

1. Fork the repository
2. Create a feature/fix branch (git checkout -b feature/amazing-feature)
3. Commit your changes (git commit -m 'Add amazing feature')
4. Push to the branch (git push origin feature/amazing-feature)
5. Open a Pull Request

So if you want to add debugging to src/market.cpp, you'd title your PR `add_market_debug` and then elaborate in your PR description.



# License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.

# Disclaimer
This software is provided "as is" without warranty. The authors are not liable for any claims or damages arising from its use. Please review the code carefully before using in production or live trading environments.