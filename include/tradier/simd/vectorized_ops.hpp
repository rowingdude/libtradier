/*
 * libtradier - High-Impact SIMD Vectorized Operations
 * 
 * Target functions for maximum performance improvement in trading applications:
 * 1. JSON Array Processing (1.5-2x speedup)
 * 2. Option Greeks Calculations (2-3x speedup)  
 * 3. Historical Data Processing (2-4x speedup)
 * 4. Streaming Event Processing (3-5x speedup)
 * 
 * Author: libtradier team
 * Version: 1.0
 */

#pragma once

#include "simd_traits.hpp"
#include <vector>
#include <span>
#include <string_view>
#include <optional>

namespace tradier {
namespace simd {

// Forward declarations
struct Quote;
struct Greeks;
struct HistoricalDataPoint;
struct StreamingEvent;

//==============================================================================
// 1. JSON Array Processing - Bulk Quote Parsing (1.5-2x speedup expected)
//==============================================================================

namespace json_processing {

/**
 * @brief Vectorized bulk conversion of JSON numeric strings to doubles
 * 
 * Target: parseQuotes() bulk operations processing 10s-100s of symbols
 * Current: Sequential string-to-double conversion
 * Optimization: SIMD string parsing + parallel conversion
 * 
 * @param json_strings Array of JSON numeric strings
 * @param output Pre-allocated output array
 * @param count Number of strings to process
 * @return Number of successfully converted values
 */
LIBTRADIER_SIMD_FUNCTION_VARIANTS(bulk_string_to_double);

/**
 * @brief Vectorized field extraction from JSON arrays
 * 
 * Target: Multi-symbol quote processing with repeated field access
 * Current: Sequential field-by-field extraction
 * Optimization: Parallel field extraction with SIMD comparisons
 * 
 * @param json_data Raw JSON data buffer
 * @param field_offsets Pre-computed field positions
 * @param output Extracted numeric values
 * @param count Number of records to process
 */
LIBTRADIER_SIMD_FUNCTION_VARIANTS(extract_numeric_fields);

/**
 * @brief Vectorized symbol string concatenation
 * 
 * Target: Multi-symbol API request URL building
 * Current: Sequential ostringstream concatenation  
 * Optimization: SIMD string operations for bulk concatenation
 * 
 * @param symbols Array of symbol strings
 * @param separator Separator character (typically ',')
 * @param output Pre-allocated output buffer
 * @return Length of concatenated string
 */
LIBTRADIER_SIMD_FUNCTION_VARIANTS(concat_symbols);

} // namespace json_processing

//==============================================================================
// 2. Option Greeks Calculations (2-3x speedup expected)  
//==============================================================================

namespace greeks_calculation {

/**
 * @brief Vectorized Greeks computation for option chains
 * 
 * Target: parseGreeks() processing 10 numeric values simultaneously
 * Current: Individual field extraction and assignment
 * Optimization: 4-way parallel Greeks calculation (AVX2 = 4 doubles)
 * 
 * Greeks processed: delta, gamma, theta, vega, rho, phi, IV values
 */
struct GreeksVector {
    LIBTRADIER_SIMD_ALIGN double delta[LIBTRADIER_SIMD_DOUBLE_WIDTH];
    LIBTRADIER_SIMD_ALIGN double gamma[LIBTRADIER_SIMD_DOUBLE_WIDTH];
    LIBTRADIER_SIMD_ALIGN double theta[LIBTRADIER_SIMD_DOUBLE_WIDTH];
    LIBTRADIER_SIMD_ALIGN double vega[LIBTRADIER_SIMD_DOUBLE_WIDTH];
    LIBTRADIER_SIMD_ALIGN double rho[LIBTRADIER_SIMD_DOUBLE_WIDTH];
    LIBTRADIER_SIMD_ALIGN double phi[LIBTRADIER_SIMD_DOUBLE_WIDTH];
    LIBTRADIER_SIMD_ALIGN double bid_iv[LIBTRADIER_SIMD_DOUBLE_WIDTH];
    LIBTRADIER_SIMD_ALIGN double mid_iv[LIBTRADIER_SIMD_DOUBLE_WIDTH];
    LIBTRADIER_SIMD_ALIGN double ask_iv[LIBTRADIER_SIMD_DOUBLE_WIDTH];
    LIBTRADIER_SIMD_ALIGN double smv_vol[LIBTRADIER_SIMD_DOUBLE_WIDTH];
};

/**
 * @brief Vectorized bulk Greeks extraction from JSON
 * 
 * @param json_values Array of parsed JSON numeric values
 * @param greeks_output Vectorized Greeks structure
 * @param count Number of option contracts to process
 */
LIBTRADIER_SIMD_FUNCTION_VARIANTS(bulk_greeks_extraction);

/**
 * @brief Vectorized Greeks validation and normalization
 * 
 * Target: Range checking and normalization of Greeks values
 * Current: Individual validation per Greek
 * Optimization: SIMD range comparisons and conditional updates
 * 
 * @param greeks Input/output Greeks vectors
 * @param min_values Minimum valid values per Greek
 * @param max_values Maximum valid values per Greek  
 * @param count Number of contracts
 */
LIBTRADIER_SIMD_FUNCTION_VARIANTS(validate_greeks);

/**
 * @brief Vectorized implied volatility surface calculations
 * 
 * Target: Real-time IV surface computation for options chains
 * Current: Sequential IV interpolation
 * Optimization: SIMD polynomial evaluation and surface fitting
 * 
 * @param strikes Array of strike prices
 * @param ivs Array of implied volatilities
 * @param target_strikes Strikes to interpolate
 * @param output Interpolated IV values
 * @param count Number of points
 */
LIBTRADIER_SIMD_FUNCTION_VARIANTS(interpolate_iv_surface);

} // namespace greeks_calculation

//==============================================================================
// 3. Historical Data Processing (2-4x speedup expected)
//==============================================================================

namespace historical_data {

/**
 * @brief Vectorized OHLCV data processing
 * 
 * Target: parseHistoricalDataList() processing 1000+ data points
 * Current: Sequential OHLCV extraction and validation
 * Optimization: SIMD processing of price and volume arrays
 */
struct OHLCVVector {
    LIBTRADIER_SIMD_ALIGN double open[LIBTRADIER_SIMD_DOUBLE_WIDTH];
    LIBTRADIER_SIMD_ALIGN double high[LIBTRADIER_SIMD_DOUBLE_WIDTH];
    LIBTRADIER_SIMD_ALIGN double low[LIBTRADIER_SIMD_DOUBLE_WIDTH];
    LIBTRADIER_SIMD_ALIGN double close[LIBTRADIER_SIMD_DOUBLE_WIDTH];
    LIBTRADIER_SIMD_ALIGN double volume[LIBTRADIER_SIMD_DOUBLE_WIDTH];
};

/**
 * @brief Vectorized historical data extraction and validation
 * 
 * @param raw_data Array of raw OHLCV data points
 * @param ohlcv_output Vectorized OHLCV structure
 * @param count Number of data points to process
 * @return Number of valid data points processed
 */
LIBTRADIER_SIMD_FUNCTION_VARIANTS(bulk_ohlcv_processing);

/**
 * @brief Vectorized technical indicator calculations
 * 
 * Target: Real-time technical analysis on historical data
 * Current: Sequential price calculations
 * Optimization: SIMD moving averages, RSI, MACD calculations
 * 
 * @param prices Array of price data
 * @param window_size Moving average window
 * @param output Calculated indicator values
 * @param count Number of data points
 */
LIBTRADIER_SIMD_FUNCTION_VARIANTS(calculate_moving_average);
LIBTRADIER_SIMD_FUNCTION_VARIANTS(calculate_rsi);
LIBTRADIER_SIMD_FUNCTION_VARIANTS(calculate_bollinger_bands);

/**
 * @brief Vectorized price volatility calculations
 * 
 * Target: Real-time volatility analysis for risk management
 * Current: Sequential variance and standard deviation
 * Optimization: SIMD statistical calculations with FMA
 * 
 * @param returns Array of price returns
 * @param window Window size for volatility calculation
 * @param output Calculated volatilities
 * @param count Number of periods
 */
LIBTRADIER_SIMD_FUNCTION_VARIANTS(calculate_volatility);

/**
 * @brief Vectorized correlation matrix calculations
 * 
 * Target: Portfolio correlation analysis
 * Current: Nested loops for correlation computation
 * Optimization: SIMD correlation coefficient calculation
 * 
 * @param price_matrix Matrix of price series (symbols x time)
 * @param correlation_matrix Output correlation matrix
 * @param num_symbols Number of symbols
 * @param num_periods Number of time periods
 */
LIBTRADIER_SIMD_FUNCTION_VARIANTS(calculate_correlation_matrix);

} // namespace historical_data

//==============================================================================
// 4. Streaming Event Processing (3-5x speedup expected) - HIGHEST IMPACT
//==============================================================================

namespace streaming {

/**
 * @brief Vectorized real-time event processing
 * 
 * Target: processEvent() in streaming.cpp:252-337 - CRITICAL PATH
 * Current: Sequential price/volume field extraction and validation
 * Optimization: SIMD event parsing + parallel field processing
 * 
 * This is the highest impact optimization for latency-sensitive trading
 */
struct StreamingEventVector {
    LIBTRADIER_SIMD_ALIGN double prices[LIBTRADIER_SIMD_DOUBLE_WIDTH];
    LIBTRADIER_SIMD_ALIGN double volumes[LIBTRADIER_SIMD_DOUBLE_WIDTH];
    LIBTRADIER_SIMD_ALIGN double sizes[LIBTRADIER_SIMD_DOUBLE_WIDTH];
    LIBTRADIER_SIMD_ALIGN double timestamps[LIBTRADIER_SIMD_DOUBLE_WIDTH];
    
    // Event metadata (packed for SIMD comparisons)
    LIBTRADIER_SIMD_ALIGN uint32_t event_types[simd_width_v<int32_t>];
    LIBTRADIER_SIMD_ALIGN uint32_t exchange_ids[simd_width_v<int32_t>];
};

/**
 * @brief Vectorized trade event processing
 * 
 * Target: High-frequency trade event parsing and validation
 * Current: parseNumericField() called sequentially for each field
 * Optimization: Batch processing of multiple trade events
 * 
 * @param raw_events Array of raw trade event data
 * @param event_output Vectorized event structure
 * @param count Number of events to process
 * @return Number of valid events processed
 */
LIBTRADIER_SIMD_FUNCTION_VARIANTS(process_trade_events);

/**
 * @brief Vectorized quote event processing
 * 
 * Target: Real-time bid/ask update processing
 * Current: Sequential quote field extraction
 * Optimization: SIMD bid/ask processing with spread calculation
 * 
 * @param raw_quotes Array of raw quote data
 * @param quote_output Processed quote data
 * @param count Number of quotes to process
 */
LIBTRADIER_SIMD_FUNCTION_VARIANTS(process_quote_events);

/**
 * @brief Vectorized market data aggregation
 * 
 * Target: Real-time market data consolidation
 * Current: Sequential aggregation of trade and quote data
 * Optimization: SIMD aggregation with parallel min/max/sum operations
 * 
 * @param events Array of market events
 * @param aggregated_data Output aggregated market data
 * @param time_window Aggregation time window
 * @param count Number of events
 */
LIBTRADIER_SIMD_FUNCTION_VARIANTS(aggregate_market_data);

/**
 * @brief Vectorized price impact calculations
 * 
 * Target: Real-time market impact analysis
 * Current: Sequential impact calculations per trade
 * Optimization: SIMD impact calculation with order book modeling
 * 
 * @param trades Array of trade data
 * @param order_book Current order book state
 * @param impact_output Calculated price impacts
 * @param count Number of trades
 */
LIBTRADIER_SIMD_FUNCTION_VARIANTS(calculate_price_impact);

/**
 * @brief Vectorized latency-critical operations
 * 
 * Target: Ultra-low latency event processing (<1μs)
 * Current: Multiple function calls in critical path
 * Optimization: Single SIMD function for complete event processing
 * 
 * @param raw_event Single raw event data
 * @param processed_event Output processed event
 * @param validation_flags Validation and processing flags
 */
LIBTRADIER_SIMD_FUNCTION_VARIANTS(fast_event_processing);

} // namespace streaming

//==============================================================================
// 5. Cross-Cutting Performance Utilities
//==============================================================================

namespace utilities {

/**
 * @brief Vectorized string-to-double conversion with validation
 * 
 * Target: JSON parsing bottlenecks across all modules
 * Current: std::stod() or similar sequential conversion
 * Optimization: SIMD string parsing with parallel validation
 * 
 * @param strings Array of numeric strings
 * @param output Array of converted doubles
 * @param valid_flags Array of validation flags
 * @param count Number of strings
 * @return Number of successful conversions
 */
LIBTRADIER_SIMD_FUNCTION_VARIANTS(bulk_string_to_double_validated);

/**
 * @brief Vectorized array validation and sanitization
 * 
 * Target: Data validation across all financial calculations
 * Current: Sequential range checking and NaN detection
 * Optimization: SIMD range checking and conditional replacement
 * 
 * @param input Array of input values
 * @param output Array of sanitized values
 * @param min_val Minimum valid value
 * @param max_val Maximum valid value
 * @param replacement Replacement value for invalid data
 * @param count Number of values
 */
LIBTRADIER_SIMD_FUNCTION_VARIANTS(validate_and_sanitize);

/**
 * @brief Vectorized financial math operations
 * 
 * Target: Common financial calculations used throughout the library
 * Current: Sequential arithmetic operations
 * Optimization: SIMD arithmetic with FMA for accuracy
 */
LIBTRADIER_SIMD_FUNCTION_VARIANTS(compound_interest);
LIBTRADIER_SIMD_FUNCTION_VARIANTS(present_value);
LIBTRADIER_SIMD_FUNCTION_VARIANTS(percentage_change);
LIBTRADIER_SIMD_FUNCTION_VARIANTS(logarithmic_returns);

/**
 * @brief Vectorized memory operations
 * 
 * Target: Large data movement and initialization
 * Current: Standard library memcpy/memset
 * Optimization: SIMD memory operations with prefetching
 */
LIBTRADIER_SIMD_FUNCTION_VARIANTS(fast_copy_aligned);
LIBTRADIER_SIMD_FUNCTION_VARIANTS(fast_zero_aligned);
LIBTRADIER_SIMD_FUNCTION_VARIANTS(fast_compare_arrays);

} // namespace utilities

//==============================================================================
// Performance Monitoring and Benchmarking
//==============================================================================

#if SIMD_BENCHMARKS_ENABLED

namespace benchmarking {

/**
 * @brief SIMD performance measurement utilities
 * 
 * Provides timing and throughput measurement for SIMD operations
 * Integrated with Google Benchmark for regression testing
 */
struct simd_benchmark_results {
    double scalar_time_ns;
    double simd_time_ns;
    double speedup_ratio;
    size_t elements_processed;
    double throughput_mops; // Million operations per second
};

/**
 * @brief Benchmark SIMD vs scalar performance
 * 
 * @param operation_name Name of the operation being benchmarked
 * @param scalar_func Scalar implementation function
 * @param simd_func SIMD implementation function
 * @param data_size Size of test data
 * @param iterations Number of benchmark iterations
 * @return Benchmark results
 */
template<typename ScalarFunc, typename SIMDFunc>
simd_benchmark_results benchmark_simd_operation(
    const std::string& operation_name,
    ScalarFunc scalar_func,
    SIMDFunc simd_func,
    size_t data_size,
    size_t iterations = 1000
);

/**
 * @brief Generate SIMD performance report
 * 
 * @param results Array of benchmark results
 * @param output_file Output file for performance report
 */
void generate_simd_performance_report(
    const std::vector<simd_benchmark_results>& results,
    const std::string& output_file
);

} // namespace benchmarking

#endif // SIMD_BENCHMARKS_ENABLED

} // namespace simd
} // namespace tradier

//==============================================================================
// Implementation Selection Macros
//==============================================================================

// Macro to automatically choose the best available SIMD implementation
#define LIBTRADIER_SIMD_CALL(func_name, ...) \
    ::tradier::simd::func_name(__VA_ARGS__)

// Macro for compile-time SIMD optimization selection
#ifdef LIBTRADIER_SIMD_ENABLED
    #define LIBTRADIER_SIMD_OPTIMIZE(scalar_code, simd_code) simd_code
#else
    #define LIBTRADIER_SIMD_OPTIMIZE(scalar_code, simd_code) scalar_code
#endif

// Macro for conditional SIMD compilation
#define LIBTRADIER_SIMD_IF_ENABLED(code) \
    LIBTRADIER_SIMD_OPTIMIZE(, code)

// Performance annotation macros
#define LIBTRADIER_SIMD_HOT __attribute__((hot))
#define LIBTRADIER_SIMD_COLD __attribute__((cold))