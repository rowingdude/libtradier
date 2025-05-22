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





