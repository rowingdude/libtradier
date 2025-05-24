// testing/test_config.cpp
#include "test_framework.hpp"
#include "tradier/common/config.hpp"
#include <cstdlib>
#include <memory>

namespace {

class EnvGuard {
private:
    std::string var_;
    std::string old_value_;
    bool had_value_;
    
public:
    EnvGuard(const std::string& var, const std::string& value) : var_(var) {
        const char* old = std::getenv(var_.c_str());
        had_value_ = (old != nullptr);
        if (had_value_) {
            old_value_ = old;
        }
        setenv(var_.c_str(), value.c_str(), 1);
    }
    
    ~EnvGuard() {
        if (had_value_) {
            setenv(var_.c_str(), old_value_.c_str(), 1);
        } else {
            unsetenv(var_.c_str());
        }
    }
};

void test_default_config() {
    unsetenv("TRADIER_SBX_ENABLE");
    unsetenv("TRADIER_SBX_TOKEN");
    unsetenv("TRADIER_SBX_ACCNUM");
    unsetenv("TRADIER_PROD_TOKEN");
    unsetenv("TRADIER_API_TIMEOUT");
    
    auto config = tradier::Config::fromEnvironment();
    
    ASSERT_TRUE(config.sandboxMode);
    ASSERT_TRUE(config.accessToken.empty());
    ASSERT_TRUE(config.accountNumber.empty());
    ASSERT_EQ(30, config.timeoutSeconds);
}

void test_sandbox_mode() {
    EnvGuard sbx_enable("TRADIER_SBX_ENABLE", "1");
    EnvGuard sbx_token("TRADIER_SBX_TOKEN", "test_sandbox_token");
    EnvGuard sbx_accnum("TRADIER_SBX_ACCNUM", "SB123456");
    
    auto config = tradier::Config::fromEnvironment();
    
    ASSERT_TRUE(config.sandboxMode);
    ASSERT_EQ("test_sandbox_token", config.accessToken);
    ASSERT_EQ("SB123456", config.accountNumber);
}

void test_production_mode() {
    EnvGuard sbx_enable("TRADIER_SBX_ENABLE", "false");
    EnvGuard prod_token("TRADIER_PROD_TOKEN", "test_prod_token");
    
    auto config = tradier::Config::fromEnvironment();
    
    ASSERT_FALSE(config.sandboxMode);
    ASSERT_EQ("test_prod_token", config.accessToken);
}

void test_custom_timeout() {
    EnvGuard timeout("TRADIER_API_TIMEOUT", "60");
    
    auto config = tradier::Config::fromEnvironment();
    
    ASSERT_EQ(60, config.timeoutSeconds);
}

void test_invalid_timeout() {
    EnvGuard timeout("TRADIER_API_TIMEOUT", "invalid");
    
    auto config = tradier::Config::fromEnvironment();
    
    ASSERT_EQ(30, config.timeoutSeconds);
}

void test_base_urls() {
    tradier::Config config;
    
    config.sandboxMode = true;
    ASSERT_EQ("https://sandbox.tradier.com/v1", config.baseUrl());
    ASSERT_EQ("wss://sandbox.tradier.com/v1", config.wsUrl());
    
    config.sandboxMode = false;
    ASSERT_EQ("https://api.tradier.com/v1", config.baseUrl());
    ASSERT_EQ("wss://api.tradier.com/v1", config.wsUrl());
}

}

int test_config_main() {
    test::TestSuite suite("Configuration Tests");
    
    suite.add_test("Default Configuration", test_default_config);
    suite.add_test("Sandbox Mode", test_sandbox_mode);
    suite.add_test("Production Mode", test_production_mode);
    suite.add_test("Custom Timeout", test_custom_timeout);
    suite.add_test("Invalid Timeout", test_invalid_timeout);
    suite.add_test("Base URLs", test_base_urls);
    
    return suite.run();
}