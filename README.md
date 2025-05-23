# libTradier Library Setup Instructions

## Before we begin...

All code is in reference to the comprehensive documentation at: [Tradier Developer API Reference](https://documentation.tradier.com/brokerage-api).

This code, all exmaples, tests, and my writings in the libtradier project are hereby released under the  MIT license, which stipulates indemnity for myself, *"The authors/owners are not liable for any claims or damages arising from the software."*, as well as *No Warranty* and my code should be carefully reviewed before using it in any production or live trading activities. 

## Building from Source

To build and install the libTradier library, you need the following dependencies:

- CMake 3.14 or higher
- C++17 compatible compiler
- [libcurl](https://github.com/curl/curl)
- [nlohmann-json](https://github.com/nlohmann/json)
- [websocketpp ](https://github.com/zaphoyd/websocketpp)
- [Boost.Asio ](https://github.com/boostorg/asio)
- [OpenSSL ](https://github.com/openssl/openssl)

Every major distribution includes these libraries in their repos, links provided for reference

### Linux/macOS & Windows Installation

1. Install dependencies:

 a. On Ubuntu:
    
    sudo apt-get install libcurl4-openssl-dev libssl-dev
   
 b. Or Mac (HomeBrew):
   
    brew install cmake curl openssl nlohmann-json boost

 c. Or Windows (vcpkg)

    vcpkg install curl openssl nlohmann-json boost websocketpp

Make sure to add:

    find_package(libTradier REQUIRED)
    target_link_libraries(your_target_name PRIVATE tradier::libTradier)

To your CMakeLists.txt!


2. Compile!

 a. On *nix/Mac OS

    mkdir build && cd build
    cmake ..
    make -j4
    sudo make install

 b. On 'doze

    mkdir build && cd build
    cmake .. -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake
    cmake --build . --config Release
    cmake --install . --config Release


## Environment Configuration

The libtradier library uses separate environment variables for sandbox and production environments:

### Sandbox Environment (Default)

- `TRADIER_SBX_TOKEN`: Your Tradier sandbox API token
- `TRADIER_SBX_ACCNUM`: Your Tradier sandbox account number
- `TRADIER_SBX_ENABLE`: Set to "true" for sandbox mode (default)

### Production Environment

- `TRADIER_PROD_TOKEN`: Your Tradier production API token
- `TRADIER_SBX_ENABLE`: Set to "false" to use production mode

### General Settings

- `TRADIER_API_TIMEOUT`: HTTP timeout in seconds (default: 30)

### Example setup for sandbox:


# Set sandbox credentials (default mode)
    export TRADIER_SBX_TOKEN=your_sandbox_token_here
    export TRADIER_SBX_ACCNUM=your_sandbox_account_number

# Run an example
    ./run.sh account_info

## Usage

You can use these libraries in any several ways, but see "Examples" for basic use-cases.

This repo contains a number of test programs, to illustrate functionality. Some output of each is below:

### Market Feature Test

This is my "overall function" test, basically it makes sure that we can get and read all data types from Tradier. 

    ./bin/market_feature_test 
    === Tradier Market Data Feature Test ===
    Using SANDBOX environment

    Test 1: Getting market status...
    Market Clock:
    Date: 2025-05-23
    State: open
    Description: Market is open from 09:30 to 16:00
    Next Change: 16:00 (postmarket)

    Test 2: Getting single stock quote for AAPL...
    Found 1 quotes:
    AAPL (Apple Inc):
        Last: $196.87 | Bid: $196.87 | Ask: $196.90 | Volume: 25253449

    Test 3: Getting multiple stock quotes...
    Found 5 quotes:
    AAPL (Apple Inc):
        Last: $196.87 | Bid: $196.87 | Ask: $196.90 | Volume: 25253449
    MSFT (Microsoft Corp):
        Last: $451.45 | Bid: $451.37 | Ask: $451.48 | Volume: 3651792
    GOOGL (Alphabet Inc):
        Last: $169.34 | Bid: $169.33 | Ask: $169.35 | Volume: 9950508
    TSLA (Tesla Inc):
        Last: $337.13 | Bid: $337.03 | Ask: $337.13 | Volume: 25476324
    SPY (SPDR S&P 500):
        Last: $578.15 | Bid: $578.15 | Ask: $578.16 | Volume: 22012749

    Test 4: Testing POST method for quotes...
    Found 2 quotes:
    QQQ (Invesco QQQ Trust, Series 1):
        Last: $509.11 | Bid: $509.10 | Ask: $509.11 | Volume: 16142180
    IWM (iShares Russell 2000 ETF):
        Last: $201.84 | Bid: $201.84 | Ask: $201.86 | Volume: 8447679

    Test 5: Getting option expirations for SPY...
    Found 32 expiration dates:
    2025-05-23 (weeklys) - 178 strikes, contract size: 100
    2025-05-27 (weeklys) - 115 strikes, contract size: 100
    2025-05-28 (weeklys) - 101 strikes, contract size: 100
    2025-05-29 (weeklys) - 131 strikes, contract size: 100
    2025-05-30 (eom) - 189 strikes, contract size: 100
    ... and 27 more expirations

    Test 6: Getting option chain for SPY 2025-05-23...
    ⚠️  Option chain data not available in sandbox mode (this is normal)
    Sandbox environments typically don't provide full options data
    ✅ Test would pass in production mode

    Test 8: Getting historical data for AAPL...
    Found 21 historical data points:
    2024-01-02: O: $187.15 H: $188.44 L: $183.88 C: $185.64 Vol: 82488674
    2024-01-03: O: $184.22 H: $185.88 L: $183.43 C: $184.25 Vol: 58414460
    2024-01-04: O: $182.15 H: $183.09 L: $180.88 C: $181.91 Vol: 71983570
    2024-01-05: O: $181.99 H: $182.76 L: $180.17 C: $181.18 Vol: 62379661
    2024-01-08: O: $182.09 H: $185.60 L: $181.50 C: $185.56 Vol: 59144470
    ... and 16 more entries

    Test 9: Getting time and sales data for SPY...
    ⚠️  Time and sales data may not be available in sandbox mode

    Test 10: Searching for 'apple' symbols...
    Apple Search Results (4 results):
    AAPL (Q) - Apple Inc [stock]
    APLE (N) - Apple Hospitality REIT Inc [stock]
    AAPI (V) - APPLE ISPORTS GROUP INC by Apple iSports Group Inc. [stock]
    APRU (V) - APPLE RUSH COMP INC by Apple Rush Co., Inc. [stock]

    Test 11: Looking up 'GOOG' symbols...
    GOOG Lookup Results (2 results):
    GOOG (Q) - Alphabet Inc. - Class C Capital Stock [stock]
    GOOGL (Q) - Alphabet Inc [stock]

    Test 12: Looking up option symbols for AAPL...
    Found 1 option symbol groups:
    Root: AAPL (2464 options)
        Examples: AAPL250620C00145000, AAPL250620C00250000, AAPL250620P00245000 ... +2461 more

    Test 13: Getting ETB (Easy to Borrow) list sample...
    ETB Securities Sample (2365 results):
    NVEE (Q) - NV5 Global Inc [stock]
    KIM (N) - Kimco Realty Corp [stock]
    USHY (Z) - iShares Broad USD High Yield Corporate Bond ETF [etf]
    MKC (N) - McCormick & Co Inc [stock]
    VOYA (N) - Voya Financial Inc [stock]
    HDIUF (V) - ADENTRA INC by ADENTRA Inc. [stock]
    BKE (N) - Buckle Inc [stock]
    TDY (N) - Teledyne Technologies Inc [stock]
    FBK (N) - FB Financial Corp [stock]
    BBIO (Q) - BridgeBio Pharma Inc [stock]
    ... and 2355 more results

    Test 14: Getting market calendar for January 2024...
    Market Calendar for 1/2024:
    Found 31 days. Sample:
    2024-01-01 - closed (Market is closed for New Years Day)
    2024-01-02 - open (Market is open)
        Market Hours: 09:30 - 16:00
    2024-01-03 - open (Market is open)
        Market Hours: 09:30 - 16:00
    2024-01-04 - open (Market is open)
        Market Hours: 09:30 - 16:00
    2024-01-05 - open (Market is open)
        Market Hours: 09:30 - 16:00
    ... and 26 more days

    === All Market Data Features Tested Successfully ===

    📊 Test Summary:
    ✅ Market clock and status
    ✅ Single and multiple stock quotes
    ✅ GET and POST quote methods
    ✅ Option expirations and strikes
    ✅ Option chains
    ✅ Historical price data
    ✅ Time and sales data
    ✅ Symbol search and lookup
    ✅ Option symbol lookup
    ✅ ETB (Easy to Borrow) list
    ✅ Market calendar



