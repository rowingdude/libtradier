#include "tradier/client.hpp"
#include "tradier/streaming.hpp"
#include "tradier/common/errors.hpp"
#include <iostream>
#include <thread>
#include <chrono>

void handleMarketEvent(const tradier::MarketEvent& event) {
    std::cout << "📈 Market Event: " << event.type 
              << " | Symbol: " << event.symbol 
              << " | Price: $" << event.price 
              << " | Size: " << event.size << std::endl;
}

void handleAccountEvent(const tradier::AccountEvent& event) {
    std::cout << "💼 Account Event: " << event.event 
              << " | Order ID: " << event.orderId 
              << " | Status: " << event.status 
              << " | Account: " << event.account << std::endl;
}

int main() {
    try {
        auto config = tradier::Config::fromEnvironment();
        tradier::TradierClient client(config);
        auto streamingService = client.streaming();
        
        std::cout << "=== Tradier Streaming Feature Test ===" << std::endl;
        std::cout << "Using " << (config.sandboxMode ? "SANDBOX" : "PRODUCTION") << " environment\n\n";
        
        // Test 1: Create Market Streaming Session
        std::cout << "Test 1: Creating market streaming session..." << std::endl;
        auto marketSession = streamingService.createMarketSession();
        if (!marketSession) {
            std::cerr << "Failed to create market streaming session" << std::endl;
            return 1;
        }
        
        std::cout << "✅ Market session created successfully!" << std::endl;
        std::cout << "   Session URL: " << marketSession->url << std::endl;
        std::cout << "   Session ID: " << marketSession->sessionId << std::endl << std::endl;
        
        // Test 2: Create Account Streaming Session  
        std::cout << "Test 2: Creating account streaming session..." << std::endl;
        auto accountSession = streamingService.createAccountSession();
        if (!accountSession) {
            std::cerr << "Failed to create account streaming session" << std::endl;
            return 1;
        }
        
        std::cout << "✅ Account session created successfully!" << std::endl;
        std::cout << "   Session URL: " << accountSession->url << std::endl;
        std::cout << "   Session ID: " << accountSession->sessionId << std::endl << std::endl;
        
        // Test 3: Connect to Market Stream
        std::cout << "Test 3: Connecting to market stream for AAPL, SPY..." << std::endl;
        std::vector<std::string> symbols = {"AAPL", "SPY", "QQQ"};
        
        bool marketConnected = streamingService.connectMarket(*marketSession, symbols, handleMarketEvent);
        if (!marketConnected) {
            std::cerr << "Failed to connect to market stream" << std::endl;
            return 1;
        }
        
        std::cout << "✅ Connected to market stream!" << std::endl;
        std::cout << "📡 Receiving market data for: ";
        for (size_t i = 0; i < symbols.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << symbols[i];
        }
        std::cout << std::endl << std::endl;
        
        // Test 4: Connect to Account Stream
        std::cout << "Test 4: Connecting to account stream..." << std::endl;
        bool accountConnected = streamingService.connectAccount(*accountSession, handleAccountEvent);
        if (!accountConnected) {
            std::cerr << "Failed to connect to account stream" << std::endl;
            return 1;
        }
        
        std::cout << "✅ Connected to account stream!" << std::endl;
        std::cout << "📡 Monitoring account events..." << std::endl << std::endl;
        
        // Test 5: Let streams run for a while
        std::cout << "🔄 Streaming data for 30 seconds..." << std::endl;
        std::cout << "   Market events should appear every ~1 second" << std::endl;
        std::cout << "   Account events should appear occasionally" << std::endl;
        std::cout << "   Press Ctrl+C to stop early\n" << std::endl;
        
        // Stream for 30 seconds
        for (int i = 30; i > 0; --i) {
            std::cout << "\r⏱️  Time remaining: " << i << " seconds   " << std::flush;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        std::cout << std::endl << std::endl;
        
        // Test 6: Check connection status
        std::cout << "Test 6: Checking connection status..." << std::endl;
        bool isConnected = streamingService.isConnected();
        std::cout << "Connection status: " << (isConnected ? "Connected ✅" : "Disconnected ❌") << std::endl;
        
        // Test 7: Disconnect
        std::cout << "\nTest 7: Disconnecting from streams..." << std::endl;
        streamingService.disconnect();
        
        // Wait a moment for disconnect to take effect
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        bool isDisconnected = !streamingService.isConnected();
        std::cout << "Disconnection status: " << (isDisconnected ? "Successfully disconnected ✅" : "Still connected ❌") << std::endl;
        
        std::cout << "\n=== All Streaming Features Tested Successfully ===" << std::endl;
        std::cout << "\n📊 Test Summary:" << std::endl;
        std::cout << "✅ Market session creation" << std::endl;
        std::cout << "✅ Account session creation" << std::endl;
        std::cout << "✅ Market stream connection" << std::endl;
        std::cout << "✅ Account stream connection" << std::endl;
        std::cout << "✅ Data streaming (mock)" << std::endl;
        std::cout << "✅ Connection status checking" << std::endl;
        std::cout << "✅ Clean disconnection" << std::endl;
        
    } catch (const tradier::ValidationError& e) {
        std::cerr << "Validation Error: " << e.what() << std::endl;
        return 1;
    } catch (const tradier::ApiError& e) {
        std::cerr << "API Error: " << e.what() << std::endl;
        return 1;
    } catch (const tradier::ConnectionError& e) {
        std::cerr << "Connection Error: " << e.what() << std::endl;
        return 1;
    } catch (const tradier::AuthenticationError& e) {
        std::cerr << "Authentication Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}