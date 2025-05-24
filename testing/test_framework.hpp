#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <stdexcept>
#include <sstream>
#include <exception>
#include <memory>

namespace test {

class TestFailure : public std::runtime_error {
public:
    explicit TestFailure(const std::string& message) : std::runtime_error(message) {}
};

class TestSuite {
private:
    std::string name_;
    std::vector<std::function<void()>> tests_;
    std::vector<std::string> test_names_;
    
public:
    explicit TestSuite(const std::string& name) : name_(name) {}
    
    void add_test(const std::string& test_name, std::function<void()> test_func) {
        test_names_.push_back(test_name);
        tests_.push_back(test_func);
    }
    
    int run() {
        int passed = 0;
        int failed = 0;
        
        std::cout << "Running test suite: " << name_ << std::endl;
        
        for (size_t i = 0; i < tests_.size(); ++i) {
            std::cout << "  " << test_names_[i] << "... ";
            
            try {
                tests_[i]();
                std::cout << "PASS" << std::endl;
                passed++;
            } catch (const TestFailure& e) {
                std::cout << "FAIL: " << e.what() << std::endl;
                failed++;
            } catch (const std::exception& e) {
                std::cout << "ERROR: " << e.what() << std::endl;
                failed++;
            } catch (...) {
                std::cout << "ERROR: Unknown exception" << std::endl;
                failed++;
            }
        }
        
        std::cout << "\nResults: " << passed << " passed, " << failed << " failed" << std::endl;
        return failed;
    }
};

#define ASSERT_EQ(expected, actual) \
    do { \
        auto exp_val = (expected); \
        auto act_val = (actual); \
        if constexpr (std::is_same_v<decltype(exp_val), std::string> || \
                      std::is_same_v<decltype(act_val), std::string>) { \
            if (std::string(exp_val) != std::string(act_val)) { \
                throw TestFailure("Expected '" + std::string(exp_val) + "', got '" + std::string(act_val) + "'"); \
            } \
        } else { \
            if (exp_val != act_val) { \
                throw TestFailure("Expected '" + std::to_string(exp_val) + "', got '" + std::to_string(act_val) + "'"); \
            } \
        } \
    } while(0)

#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            throw TestFailure("Expected true, got false: " #condition); \
        } \
    } while(0)

#define ASSERT_FALSE(condition) \
    do { \
        if (condition) { \
            throw TestFailure("Expected false, got true: " #condition); \
        } \
    } while(0)

#define ASSERT_THROW(expression, exception_type) \
    do { \
        bool threw = false; \
        try { \
            (expression); \
        } catch (const exception_type&) { \
            threw = true; \
        } catch (...) { \
            throw TestFailure("Expected " #exception_type ", got different exception"); \
        } \
        if (!threw) { \
            throw TestFailure("Expected " #exception_type ", but no exception was thrown"); \
        } \
    } while(0)

#define ASSERT_NO_THROW(expression) \
    do { \
        try { \
            (expression); \
        } catch (const std::exception& e) { \
            throw TestFailure("Expected no exception, but got: " + std::string(e.what())); \
        } catch (...) { \
            throw TestFailure("Expected no exception, but got unknown exception"); \
        } \
    } while(0)
    
}