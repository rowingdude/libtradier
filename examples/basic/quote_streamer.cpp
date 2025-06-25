/**
 * @file quote_streamer.cpp
 * @brief Real-time quote streaming example using WebSocket
 * 
 * This example demonstrates:
 * 1. WebSocket streaming session creation
 * 2. Real-time quote subscription
 * 3. Event handling for market data updates
 * 4. Connection management and reconnection
 * 5. Graceful shutdown and cleanup
 * 6. Performance monitoring
 * 
 * Usage:
 *   export TRADIER_ACCESS_TOKEN="your-sandbox-token" 
 *   ./quote_streamer [symbol1] [symbol2] ...
 *   
 *   Press Ctrl+C to stop streaming gracefully
 * 
 * @author libtradier
 * @version 1.0
 */

#include <tradier/client.hpp>
#include <tradier/streaming.hpp>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstdlib>
#include <signal.h>
#include <atomic>
#include <chrono>
#include <map>

using namespace tradier;

// Global flag for graceful shutdown
std::atomic<bool> keep_streaming{true};

// Statistics tracking
struct StreamingStats {
    std::atomic<int> quotes_received{0};
    std::atomic<int> trades_received{0};
    std::atomic<int> errors_received{0};
    std::chrono::steady_clock::time_point start_time;
    
    StreamingStats() : start_time(std::chrono::steady_clock::now()) {}
};

StreamingStats stats;

/**
 * @brief Signal handler for graceful shutdown
 * @param signal Signal number
 */
void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\n\n🛑 Shutdown signal received. Stopping stream gracefully..." << std::endl;
        keep_streaming = false;
    }
}

/**
 * @brief Print streaming statistics
 */
void printStats() {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - stats.start_time);
    
    std::cout << "\n=== Streaming Statistics ===" << std::endl;
    std::cout << "Duration: " << duration.count() << " seconds" << std::endl;
    std::cout << "Quotes received: " << stats.quotes_received.load() << std::endl;
    std::cout << "Trades received: " << stats.trades_received.load() << std::endl;
    std::cout << "Errors received: " << stats.errors_received.load() << std::endl;
    
    if (duration.count() > 0) {
        double qps = static_cast<double>(stats.quotes_received.load()) / duration.count();
        std::cout << "Average quotes per second: " << std::fixed << std::setprecision(2) << qps << std::endl;
    }
}

/**
 * @brief Enhanced quote event handler
 * @param event Quote event data
 */
void onQuoteUpdate(const QuoteEvent& event) {
    stats.quotes_received++;
    
    // Clear previous line and print updated quote
    std::cout << "\r\033[K"; // Clear line
    std::cout << "💰 " << std::left << std::setw(6) << event.symbol 
              << " | Bid: $" << std::fixed << std::setprecision(2) << std::setw(8) << event.bid
              << " (" << std::setw(6) << event.bidsz << ")"
              << " | Ask: $" << std::setw(8) << event.ask
              << " (" << std::setw(6) << event.asksz << ")"
              << " | Spread: $" << std::setw(6) << (event.ask - event.bid)
              << " | " << std::setw(4) << stats.quotes_received.load() << " quotes";
    std::cout.flush();
}

/**
 * @brief Enhanced trade event handler  
 * @param event Trade event data
 */
void onTradeUpdate(const TradeEvent& event) {
    stats.trades_received++;
    
    std::cout << "\n📈 TRADE: " << event.symbol 
              << " - $" << std::fixed << std::setprecision(2) << event.price
              << " x " << event.size
              << " on " << event.exch
              << " (Vol: " << event.cvol << ")" << std::endl;
}

/**
 * @brief Error event handler
 * @param error Error message
 */
void onStreamError(const std::string& error) {
    stats.errors_received++;
    std::cout << "\n❌ Stream Error: " << error << std::endl;
}

/**
 * @brief Connection status handler
 * @param connected Connection status
 */
void onConnectionChange(bool connected) {
    if (connected) {
        std::cout << "\n✅ Connected to streaming service" << std::endl;
    } else {
        std::cout << "\n⚠️  Disconnected from streaming service" << std::endl;
    }
}

/**
 * @brief Get configuration from environment
 * @return Configured Config object
 */
Config getConfigFromEnvironment() {
    Config config;
    
    const char* token = std::getenv("TRADIER_ACCESS_TOKEN");
    if (!token) {
        throw std::runtime_error(
            "TRADIER_ACCESS_TOKEN environment variable not set.\n"
            "Get your sandbox token from: https://documentation.tradier.com/getting-started"
        );
    }
    
    config.accessToken = token;
    config.sandboxMode = true;
    config.timeoutSeconds = 30;
    
    return config;
}

/**
 * @brief Parse command line arguments for symbols
 * @param argc Argument count
 * @param argv Argument values  
 * @return Vector of symbols to stream
 */
std::vector<std::string> parseSymbols(int argc, char* argv[]) {
    std::vector<std::string> symbols;
    
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            symbols.emplace_back(argv[i]);
        }
    } else {
        // Default symbols for demonstration
        symbols = {"AAPL", "MSFT", "GOOGL"};
    }
    
    return symbols;
}

/**
 * @brief Display initial market data for context
 * @param client Reference to TradierClient
 * @param symbols Symbols to get quotes for
 */
void displayInitialQuotes(TradierClient& client, const std::vector<std::string>& symbols) {
    auto marketService = client.market();
    auto quotesResult = marketService.getQuotes(symbols);
    
    if (quotesResult.isSuccess()) {
        std::cout << "\n=== Initial Market Data ===" << std::endl;
        auto quotes = quotesResult.value();
        
        for (const auto& quote : quotes) {
            std::cout << quote.symbol << ": $" << std::fixed << std::setprecision(2) << quote.last
                      << " (Bid: $" << quote.bid << " | Ask: $" << quote.ask 
                      << " | Vol: " << quote.volume << ")" << std::endl;
        }
    }
}

/**
 * @brief Main streaming function
 */
int main(int argc, char* argv[]) {
    try {
        std::cout << "=== libtradier Real-Time Quote Streamer ===" << std::endl;
        std::cout << "This example demonstrates real-time market data streaming." << std::endl;
        
        // Setup signal handlers for graceful shutdown
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
        
        // 1. Parse symbols and configure client
        auto symbols = parseSymbols(argc, argv);
        auto config = getConfigFromEnvironment();
        
        std::cout << "\n✓ Configuration loaded" << std::endl;
        std::cout << "  Environment: " << (config.sandboxMode ? "Sandbox" : "Production") << std::endl;
        std::cout << "  Streaming symbols: ";
        for (size_t i = 0; i < symbols.size(); ++i) {
            std::cout << symbols[i];
            if (i < symbols.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
        
        // 2. Create client and services
        TradierClient client(config);
        auto streamingService = client.streaming();
        
        std::cout << "✓ Streaming service initialized" << std::endl;
        
        // 3. Show initial market data for context
        displayInitialQuotes(client, symbols);
        
        // 4. Create streaming session
        std::cout << "\n--- Creating Streaming Session ---" << std::endl;
        auto sessionResult = streamingService.createSession();
        
        if (!sessionResult.isSuccess()) {
            std::cerr << "✗ Failed to create streaming session: " << sessionResult.error().message << std::endl;
            return 1;
        }
        
        auto session = sessionResult.value();
        std::cout << "✓ Streaming session created" << std::endl;
        std::cout << "  Session ID: " << session.sessionid << std::endl;
        std::cout << "  WebSocket URL: " << session.url << std::endl;
        
        // 5. Set up event handlers
        streamingService.setQuoteHandler(onQuoteUpdate);
        streamingService.setTradeHandler(onTradeUpdate);
        streamingService.setErrorHandler(onStreamError);
        streamingService.setConnectionHandler(onConnectionChange);
        
        std::cout << "✓ Event handlers configured" << std::endl;
        
        // 6. Connect to WebSocket and subscribe to symbols
        std::cout << "\n--- Starting Stream ---" << std::endl;
        std::cout << "Connecting to WebSocket..." << std::endl;
        
        auto connectResult = streamingService.connect(session);
        if (!connectResult.isSuccess()) {
            std::cerr << "✗ Failed to connect to WebSocket: " << connectResult.error().message << std::endl;
            return 1;
        }
        
        // Subscribe to quote updates for all symbols
        for (const auto& symbol : symbols) {
            auto subscribeResult = streamingService.subscribeQuotes({symbol});
            if (subscribeResult.isSuccess()) {
                std::cout << "✓ Subscribed to quotes for " << symbol << std::endl;
            } else {
                std::cerr << "✗ Failed to subscribe to " << symbol << ": " 
                          << subscribeResult.error().message << std::endl;
            }
        }
        
        // 7. Main streaming loop
        std::cout << "\n🚀 Streaming started! Press Ctrl+C to stop." << std::endl;
        std::cout << "Watching for real-time updates..." << std::endl;
        std::cout << "\nLive quotes (Bid/Ask with sizes):" << std::endl;
        
        stats.start_time = std::chrono::steady_clock::now();
        
        // Keep the stream alive and process events
        while (keep_streaming) {
            // Process streaming events (this would be handled by the streaming service internally)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // Show periodic statistics every 30 seconds
            static auto lastStatsTime = std::chrono::steady_clock::now();
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - lastStatsTime).count() >= 30) {
                std::cout << "\n--- 30 Second Update ---" << std::endl;
                printStats();
                std::cout << "Still streaming... (Ctrl+C to stop)" << std::endl;
                lastStatsTime = now;
            }
        }
        
        // 8. Graceful shutdown
        std::cout << "\n--- Shutting Down Stream ---" << std::endl;
        
        // Unsubscribe from all symbols
        for (const auto& symbol : symbols) {
            streamingService.unsubscribeQuotes({symbol});
        }
        
        // Disconnect from WebSocket
        streamingService.disconnect();
        
        // Print final statistics
        printStats();
        
        std::cout << "\n✓ Stream stopped gracefully" << std::endl;
        std::cout << "\n=== Streaming Example completed ===" << std::endl;
        std::cout << "\nNext steps:" << std::endl;
        std::cout << "- Try streaming with different symbols" << std::endl;
        std::cout << "- Explore trade and summary event subscriptions" << std::endl;
        std::cout << "- Build a real-time dashboard with this data" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Error: " << e.what() << std::endl;
        std::cerr << "\nTroubleshooting:" << std::endl;
        std::cerr << "1. Ensure TRADIER_ACCESS_TOKEN is set" << std::endl;
        std::cerr << "2. Check network connectivity and firewall settings" << std::endl;
        std::cerr << "3. Verify WebSocket support is enabled" << std::endl;
        std::cerr << "4. Try with fewer symbols if experiencing rate limits" << std::endl;
        return 1;
    }
}

/*
Expected Output:
================

=== libtradier Real-Time Quote Streamer ===
This example demonstrates real-time market data streaming.

✓ Configuration loaded
  Environment: Sandbox
  Streaming symbols: AAPL, MSFT, GOOGL

✓ Streaming service initialized

=== Initial Market Data ===
AAPL: $150.25 (Bid: $150.20 | Ask: $150.30 | Vol: 1250000)
MSFT: $285.67 (Bid: $285.60 | Ask: $285.75 | Vol: 980000)
GOOGL: $125.43 (Bid: $125.40 | Ask: $125.50 | Vol: 745000)

--- Creating Streaming Session ---
✓ Streaming session created
  Session ID: ws-session-abc123
  WebSocket URL: wss://sandbox.tradier.com/v1/markets/events

✓ Event handlers configured

--- Starting Stream ---
Connecting to WebSocket...
✅ Connected to streaming service
✓ Subscribed to quotes for AAPL
✓ Subscribed to quotes for MSFT  
✓ Subscribed to quotes for GOOGL

🚀 Streaming started! Press Ctrl+C to stop.
Watching for real-time updates...

Live quotes (Bid/Ask with sizes):
💰 AAPL   | Bid: $150.19  (100   ) | Ask: $150.31  (200   ) | Spread: $0.12  |   45 quotes
📈 TRADE: MSFT - $285.70 x 100 on Q (Vol: 980543)
💰 MSFT   | Bid: $285.68  (150   ) | Ask: $285.74  (250   ) | Spread: $0.06  |   67 quotes
💰 GOOGL  | Bid: $125.38  (300   ) | Ask: $125.52  (180   ) | Spread: $0.14  |   89 quotes

--- 30 Second Update ---
=== Streaming Statistics ===
Duration: 30 seconds
Quotes received: 142
Trades received: 8
Errors received: 0
Average quotes per second: 4.73
Still streaming... (Ctrl+C to stop)

🛑 Shutdown signal received. Stopping stream gracefully...

--- Shutting Down Stream ---
⚠️  Disconnected from streaming service

=== Streaming Statistics ===
Duration: 67 seconds
Quotes received: 328
Trades received: 19
Errors received: 0
Average quotes per second: 4.90

✓ Stream stopped gracefully

=== Streaming Example completed ===

Next steps:
- Try streaming with different symbols
- Explore trade and summary event subscriptions  
- Build a real-time dashboard with this data
*/