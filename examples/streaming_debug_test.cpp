#include "tradier/client.hpp"
#include "tradier/streaming.hpp"
#include "tradier/common/errors.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

int main() {
    try {
        auto config = tradier::Config::fromEnvironment();
        tradier::TradierClient client(config);
        
        std::cout << "=== Streaming Session Debug Test ===" << std::endl;
        std::cout << "Using " << (config.sandboxMode ? "SANDBOX" : "PRODUCTION") << " environment\n\n";

        std::cout << "Testing market session creation..." << std::endl;
        auto response = client.post("/markets/events/session");
        
        std::cout << "HTTP Status: " << response.status << std::endl;
        
        if (response.status == 200) {
            std::cout << "✅ HTTP Request successful" << std::endl;
            std::cout << "Response length: " << response.body.length() << " characters" << std::endl;
            
            try {
                auto json = nlohmann::json::parse(response.body);
                std::cout << "✅ JSON parsing successful" << std::endl;

                std::string formatted = json.dump(2);
                std::cout << "JSON response:\n" << formatted << std::endl;
                
            } catch (const nlohmann::json::exception& e) {
                std::cout << "❌ JSON parsing failed: " << e.what() << std::endl;
                std::cout << "Raw response:\n" << response.body << std::endl;
            }
        } else {
            std::cout << "❌ HTTP Error " << response.status << std::endl;
            std::cout << "Response: " << response.body << std::endl;
        }
        
        std::cout << "\n" << std::string(50, '=') << std::endl;

        std::cout << "Testing account session creation..." << std::endl;
        auto accountResponse = client.post("/accounts/events/session");
        
        std::cout << "HTTP Status: " << accountResponse.status << std::endl;
        
        if (accountResponse.status == 200) {
            std::cout << "✅ HTTP Request successful" << std::endl;
            
            try {
                auto json = nlohmann::json::parse(accountResponse.body);
                std::cout << "✅ JSON parsing successful" << std::endl;
                
                std::string formatted = json.dump(2);
                std::cout << "JSON response:\n" << formatted << std::endl;
                
            } catch (const nlohmann::json::exception& e) {
                std::cout << "❌ JSON parsing failed: " << e.what() << std::endl;
                std::cout << "Raw response:\n" << accountResponse.body << std::endl;
            }
        } else {
            std::cout << "❌ HTTP Error " << accountResponse.status << std::endl;
            std::cout << "Response: " << accountResponse.body << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}