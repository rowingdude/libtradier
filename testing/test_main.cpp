#include "test_framework.hpp"
#include <iostream>

extern int test_config_main();
extern int test_json_utils_main();
extern int test_utils_main();
extern int test_auth_main();
extern int test_trading_main();
extern int test_market_main();
extern int test_watchlist_main();
extern int test_streaming_main();

int main() {
    std::cout << "Running libtradier comprehensive test suite...\n" << std::endl;
    
    int totalFailures = 0;
    
    std::cout << "\n" << std::string(60, '=') << std::endl;
    totalFailures += test_config_main();
    
    std::cout << "\n" << std::string(60, '=') << std::endl;
    totalFailures += test_utils_main();
    
    std::cout << "\n" << std::string(60, '=') << std::endl;
    totalFailures += test_json_utils_main();
    
    std::cout << "\n" << std::string(60, '=') << std::endl;
    totalFailures += test_auth_main();
    
    std::cout << "\n" << std::string(60, '=') << std::endl;
    totalFailures += test_trading_main();
    
    std::cout << "\n" << std::string(60, '=') << std::endl;
    totalFailures += test_market_main();
    
    std::cout << "\n" << std::string(60, '=') << std::endl;
    totalFailures += test_watchlist_main();
    
    std::cout << "\n" << std::string(60, '=') << std::endl;
    totalFailures += test_streaming_main();
    
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "FINAL RESULTS" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    if (totalFailures == 0) {
        std::cout << "🎉 ALL TESTS PASSED! 🎉" << std::endl;
        std::cout << "The libtradier library is working correctly." << std::endl;
    } else {
        std::cout << "❌ " << totalFailures << " TEST(S) FAILED ❌" << std::endl;
        std::cout << "Please review the failures above and fix any issues." << std::endl;
    }
    
    std::cout << std::string(60, '=') << std::endl;
    
    return totalFailures;
}