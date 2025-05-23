/*
 * libtradier - Tradier API C++ Library v0.1.0
 *
 * Author: Benjamin Cance (kc8bws@kc8bws.com)
 * Date: 2025-05-22
 *
 * This software is provided free of charge under the MIT License.
 * By using it, you agree to absolve the author of all liability.
 * See LICENSE file for full terms and conditions.
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <csignal>
#include <iomanip>
#include "tradier/client.hpp"
#include "tradier/streaming.hpp"
#include "tradier/common/errors.hpp"

std::atomic<bool> running{true};

void signalHandler(int) {
    running = false;
    std::cout << "\nShutting down..." << std::endl;
}

int main() {
    std::signal(SIGINT, signalHandler);
    
    try {
        auto config = tradier::Config::fromEnvironment();
        tradier::TradierClient client(config);
        
        auto streamingService = client.streaming();
        
        std::cout << "Creating market streaming session..." << std::endl;
        auto session = streamingService.createMarketSession();
        if (!session) {
            std::cerr << "Failed to create streaming session" << std::endl;
            return 1;
        }
        
        std::cout << "Session ID: " << session->sessionId << std::endl;
        std::cout << "Connecting to market stream for SPY..." << std::endl;
        
        int messageCount = 0;
        bool connected = streamingService.connectMarket(*session, {"SPY"}, 
            [&messageCount](const tradier::MarketEvent& event) {
                std::cout << "[" << ++messageCount << "] " << event.type 
                          << " - " << event.symbol 
                          << " @ $" << std::fixed << std::setprecision(2) << event.price 
                          << " (size: " << event.size << ")" << std::endl;
            });
        
        if (connected) {
            std::cout << "Connected! Streaming market data..." << std::endl;
            std::cout << "Press Ctrl+C to stop" << std::endl;
            
            while (running && streamingService.isConnected()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            
            streamingService.disconnect();
            std::cout << "Received " << messageCount << " messages" << std::endl;
        } else {
            std::cout << "Failed to connect to stream" << std::endl;
        }
        
    } catch (const tradier::TradierException& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}