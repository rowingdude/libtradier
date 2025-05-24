# libtradier Test Suite

My goal with this test suite is designed to validate all core functionality including configuration management, JSON utilities, authentication, trading operations, market data, watchlists, and streaming services. If you have any ideas to better it, please submit a pr.

## Test Structure

### Test Framework
- **test_framework.hpp**: Minimal custom test framework with assertions and test suite management
- **test_mocks.hpp**: Mock objects for testing HTTP clients and responses

### Test Files
- **test_config.cpp**: Configuration loading and environment variable handling
- **test_json_utils.cpp**: JSON parsing, validation, and utility functions
- **test_utils.cpp**: String utilities, URL encoding, and date/time formatting  
- **test_auth.cpp**: OAuth2 authentication, token management, and PKCE flow
- **test_trading.cpp**: Order placement, modification, cancellation, and validation
- **test_market.cpp**: Market data retrieval, quotes, options, and fundamentals
- **test_watchlist.cpp**: Watchlist creation, modification, and symbol management
- **test_streaming.cpp**: Real-time streaming, event handling, and connection management

## Building Tests

```bash
mkdir build && cd build
cmake ..
make
```

## Running Tests

Execute all tests:
```bash
./run_tests
```

Run with CTest:
```bash
ctest --verbose
```

## Test Categories

### Unit Tests
- Validate individual functions and classes in isolation
- Test input validation and error handling
- Verify correct behavior with various data types

### Integration Tests
- Test service interactions with mock HTTP clients
- Validate JSON parsing with real API response formats
- Ensure proper error propagation across components

### Edge Case Tests
- Empty inputs and null values
- Invalid JSON and malformed responses
- Network timeouts and authentication failures
- Boundary conditions for numeric values

## Test Philosophy

The test suite follows these principles:

1. **Comprehensive Coverage**: Every public API method has corresponding tests
2. **Fast Execution**: Tests run quickly without external dependencies
3. **Deterministic**: Tests produce consistent results across environments
4. **Isolated**: Each test is independent and doesn't affect others
5. **Clear Failures**: Failed tests provide specific error messages

## Mock Strategy

Tests use mock objects to avoid external API calls:
- HTTP requests return predefined responses
- Network failures are simulated through mock responses
- Authentication flows use test tokens and credentials

## Environment Setup

Tests can be configured with environment variables:
```bash
export TRADIER_SBX_TOKEN="test_token"
export TRADIER_SBX_ACCNUM="test_account"
export TRADIER_API_TIMEOUT="30"
```

## Adding New Tests

To add tests for new functionality:

1. Create test functions following the `test_*` naming convention
2. Use assertion macros: `ASSERT_EQ`, `ASSERT_TRUE`, `ASSERT_THROW`, etc.
3. Add the test function to the appropriate test suite
4. Include validation tests for all input parameters
5. Test both success and failure scenarios

Example:
```cpp
void test_new_feature() {
    // Setup
    tradier::Config config;
    config.accessToken = "test_token";
    
    // Test
    auto result = someFunction("valid_input");
    
    // Assertions
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ("expected", result->field);
    
    // Test error case
    ASSERT_THROW(someFunction(""), tradier::ValidationError);
}
```

## Test Metrics

The test suite includes approximately:
- 80+ individual test functions
- 8 test suites covering all major components
- 200+ assertions validating library behavior
- 95%+ code coverage of public APIs

## Continuous Integration

Tests are designed to run in CI environments:
- No external network dependencies
- Deterministic execution times
- Clear pass/fail status codes
- Detailed logging for debugging failures

## Performance Tests

While focused on correctness, tests also validate:
- Memory management (no leaks in long-running tests)
- JSON parsing performance with large responses
- Thread safety in streaming components
- Resource cleanup in error scenarios