# libtradier Examples

This directory contains comprehensive examples demonstrating how to use the libtradier C++ library for Tradier API integration. Examples are organized by complexity and use case.

## 🚀 Quick Start

### Prerequisites

1. **Tradier Account**: Get a free sandbox account at [Tradier Developer](https://developer.tradier.com/)
2. **Access Token**: Generate a sandbox API token from your account dashboard
3. **Build Environment**: Ensure libtradier is built and installed

### Environment Setup

```bash
# Required: Your Tradier sandbox access token
export TRADIER_ACCESS_TOKEN="your-sandbox-token-here"

# Optional: Default account number for trading examples
export TRADIER_ACCOUNT_NUMBER="your-sandbox-account-number"
```

### Building Examples

```bash
# From the libtradier root directory
mkdir build && cd build
cmake -DBUILD_EXAMPLES=ON ..
make examples_all

# Run examples from build directory
cd examples
./account_info
./market_data AAPL MSFT
./simple_trading
./quote_streamer
```

## 📁 Example Categories

### 🌟 Basic Examples (`basic/`)

Perfect for getting started with libtradier. These examples demonstrate core functionality with comprehensive error handling and documentation.

#### `account_info.cpp`
**What it demonstrates:**
- Client configuration and authentication
- Account profile and details retrieval
- Balance and buying power information
- Position listing and management
- Error handling best practices

**Usage:**
```bash
export TRADIER_ACCESS_TOKEN="your-token"
./account_info
```

**Key features:**
- Environment-based configuration
- Comprehensive account data display
- Graceful error handling
- Production-ready patterns

---

#### `market_data.cpp`
**What it demonstrates:**
- Real-time quote retrieval (single and batch)
- Options chain data with Greeks
- Market calendar and trading hours
- Symbol search functionality
- Historical data concepts

**Usage:**
```bash
# Default symbols (AAPL, MSFT, GOOGL, TSLA)
./market_data

# Custom symbols
./market_data AMZN NFLX SPY QQQ
```

**Key features:**
- Multi-symbol quote display
- Options data visualization
- Market status information
- Symbol validation and search

---

#### `simple_trading.cpp`
**What it demonstrates:**
- Order preview (risk-free validation)
- Market and limit order placement
- Order status monitoring
- Order modification and cancellation
- Trading safety protocols

**Usage:**
```bash
export TRADIER_ACCESS_TOKEN="your-token"
export TRADIER_ACCOUNT_NUMBER="your-account"
./simple_trading
```

**Safety features:**
- ⚠️ **Sandbox-only by default**
- Order preview before placement
- User confirmation prompts
- Comprehensive error checking
- Position and balance validation

---

#### `quote_streamer.cpp`
**What it demonstrates:**
- WebSocket streaming session creation
- Real-time quote subscriptions
- Event handling and processing
- Connection management
- Performance monitoring

**Usage:**
```bash
# Default symbols
./quote_streamer

# Custom symbols  
./quote_streamer AAPL MSFT GOOGL

# Stop with Ctrl+C for graceful shutdown
```

**Features:**
- Real-time bid/ask updates
- Trade event notifications
- Streaming statistics
- Graceful shutdown handling
- Connection resilience

---

### 🎯 Advanced Examples (`advanced/`)

Sophisticated applications showcasing complex use cases and architectural patterns.

#### `trading_bot.cpp` *(Coming Soon)*
**What it will demonstrate:**
- Algorithmic trading framework
- Technical indicator integration
- Risk management systems
- Multi-threaded execution
- Strategy backtesting

#### `options_dashboard.cpp` *(Coming Soon)*
**What it will demonstrate:**
- Options chain analysis
- Greeks calculation and display
- Volatility surface visualization
- Options strategy modeling
- Real-time P&L tracking

#### `portfolio_manager.cpp` *(Coming Soon)*
**What it will demonstrate:**
- Portfolio composition analysis
- Performance attribution
- Risk metrics calculation
- Rebalancing algorithms
- Asset allocation optimization

#### `market_scanner.cpp` *(Coming Soon)*
**What it will demonstrate:**
- Multi-symbol monitoring
- Alert generation system
- Technical screening
- Volume and price analysis
- Custom filter implementation

---

### 🔧 Integration Examples (`integration/`)

Focused examples demonstrating specific integration patterns and best practices.

#### `error_handling.cpp` *(Coming Soon)*
**What it will demonstrate:**
- Exception handling strategies
- Retry logic implementation
- Rate limit management
- Graceful degradation
- Logging and monitoring

#### `async_patterns.cpp` *(Coming Soon)*
**What it will demonstrate:**
- Asynchronous operation patterns
- Future and promise usage
- Callback-based programming
- Thread pool management
- Concurrent request handling

#### `config_management.cpp` *(Coming Soon)*
**What it will demonstrate:**
- Configuration file handling
- Environment variable management
- Credential security
- Multi-environment setup
- Runtime configuration updates

#### `streaming_setup.cpp` *(Coming Soon)*
**What it will demonstrate:**
- WebSocket connection management
- Event subscription patterns
- Reconnection strategies
- Data buffering techniques
- Performance optimization

---

## 🛡️ Safety and Best Practices

### Sandbox vs Production

All examples are configured for **sandbox mode by default**. This ensures:
- ✅ No real money at risk
- ✅ Safe for experimentation
- ✅ Full API functionality testing
- ✅ No market impact

To use production mode (⚠️ **ADVANCED USERS ONLY**):
1. Obtain production API credentials
2. Modify `config.sandboxMode = false` in example code
3. **Triple-check all parameters**
4. Start with minimal position sizes

### Environment Variables

| Variable | Required | Description | Example |
|----------|----------|-------------|---------|
| `TRADIER_ACCESS_TOKEN` | ✅ Yes | API access token | `abc123...` |
| `TRADIER_ACCOUNT_NUMBER` | Trading only | Account ID for orders | `123456789` |
| `TRADIER_ENVIRONMENT` | No | Override environment | `sandbox` or `production` |

### Error Handling

All examples demonstrate proper error handling:
- **Network errors**: Automatic retry with exponential backoff
- **API errors**: Detailed error messages and troubleshooting
- **Validation errors**: Input checking before API calls
- **Rate limits**: Proper handling and user feedback

## 🚀 Running Examples

### Method 1: Individual Examples
```bash
cd build/examples
./account_info
./market_data AAPL MSFT
```

### Method 2: CMake Targets
```bash
# Build specific example groups
make examples_basic
make examples_advanced
make examples_integration

# Build all examples
make examples_all
```

### Method 3: Direct Compilation
```bash
# If libtradier is installed system-wide
g++ -std=c++20 account_info.cpp -ltradier -o account_info
```

## 📊 Expected Output

Each example includes detailed expected output in comments, showing:
- ✅ Successful execution flow
- 📊 Sample data formatting
- ⚠️ Error conditions and handling
- 💡 Next steps and learning paths

## 🔍 Troubleshooting

### Common Issues

**"TRADIER_ACCESS_TOKEN not set"**
```bash
# Get token from: https://developer.tradier.com/
export TRADIER_ACCESS_TOKEN="your-token-here"
```

**"Failed to connect" errors**
```bash
# Check network connectivity
curl -I https://sandbox.tradier.com/v1/markets/quotes?symbols=AAPL

# Verify token validity
curl -H "Authorization: Bearer $TRADIER_ACCESS_TOKEN" \
     https://sandbox.tradier.com/v1/user/profile
```

**Build errors**
```bash
# Ensure libtradier is built and installed
cd libtradier && mkdir build && cd build
cmake .. && make && sudo make install
```

**WebSocket connection issues**
```bash
# Check websocketpp dependency
sudo apt install libwebsocketpp-dev  # Ubuntu/Debian
```

### Debug Mode

Enable verbose logging:
```bash
export TRADIER_DEBUG=1
export TRADIER_LOG_LEVEL=DEBUG
./market_data
```

## 📚 Learning Path

### Beginner (New to trading APIs)
1. Start with `account_info.cpp`
2. Try `market_data.cpp` with default symbols
3. Explore `quote_streamer.cpp` for real-time data

### Intermediate (Some API experience)
1. Run `simple_trading.cpp` in sandbox
2. Modify examples with your own symbols
3. Explore advanced configuration options

### Advanced (Production considerations)
1. Study error handling patterns
2. Implement custom risk management
3. Integrate with external systems
4. Performance optimization techniques

## 🤝 Contributing

Found a bug or want to add an example?

1. **Bug Reports**: Include minimal reproduction code
2. **Feature Requests**: Describe the use case and expected behavior
3. **Pull Requests**: Follow existing code style and include tests

## 📖 Additional Resources

- **[Tradier API Documentation](https://documentation.tradier.com/)**
- **[libtradier API Reference](../docs/api/)**
- **[WebSocket Streaming Guide](../docs/streaming.md)**
- **[Trading Best Practices](../docs/trading-guide.md)**

---

*All examples use sandbox mode by default. No real trading occurs unless explicitly configured for production use.*