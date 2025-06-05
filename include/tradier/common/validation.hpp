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

#pragma once

#include "tradier/common/errors.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <regex>
#include <limits>

namespace tradier {
namespace validation {

// Parameter validation utilities
class Validator {
public:
    // String validation
    static void requireNonEmpty(std::string_view value, std::string_view paramName) {
        if (value.empty()) {
            throw ValidationError(std::string(paramName) + " cannot be empty");
        }
    }
    
    static void requireValidSymbol(std::string_view symbol) {
        requireNonEmpty(symbol, "symbol");
        if (symbol.length() > 10) {
            throw ValidationError("Symbol cannot exceed 10 characters");
        }
        
        // Basic symbol validation - alphanumeric with optional dots/dashes
        static const std::regex symbolPattern("^[A-Za-z0-9.-]+$");
        if (!std::regex_match(std::string(symbol), symbolPattern)) {
            throw ValidationError("Symbol contains invalid characters");
        }
    }
    
    static void requireValidAccountNumber(std::string_view accountNumber) {
        requireNonEmpty(accountNumber, "account number");
        if (accountNumber.length() < 8 || accountNumber.length() > 20) {
            throw ValidationError("Account number must be between 8 and 20 characters");
        }
    }
    
    // Numeric validation
    template<typename T>
    static void requirePositive(T value, std::string_view paramName) {
        if (value <= 0) {
            throw ValidationError(std::string(paramName) + " must be positive");
        }
    }
    
    template<typename T>
    static void requireNonNegative(T value, std::string_view paramName) {
        if (value < 0) {
            throw ValidationError(std::string(paramName) + " cannot be negative");
        }
    }
    
    template<typename T>
    static void requireRange(T value, T min, T max, std::string_view paramName) {
        if (value < min || value > max) {
            throw ValidationError(std::string(paramName) + " must be between " + 
                                std::to_string(min) + " and " + std::to_string(max));
        }
    }
    
    // Order validation
    static void requireValidOrderId(long orderId) {
        if (orderId <= 0) {
            throw ValidationError("Order ID must be positive");
        }
        if (orderId > std::numeric_limits<int>::max()) {
            throw ValidationError("Order ID exceeds maximum value");
        }
    }
    
    static void requireValidQuantity(double quantity) {
        if (quantity <= 0) {
            throw ValidationError("Quantity must be positive");
        }
        if (quantity > 1000000) {
            throw ValidationError("Quantity exceeds maximum allowed value");
        }
    }
    
    static void requireValidPrice(double price) {
        if (price < 0) {
            throw ValidationError("Price cannot be negative");
        }
        if (price > 1000000) {
            throw ValidationError("Price exceeds maximum allowed value");
        }
    }
    
    // Collection validation
    template<typename Container>
    static void requireNonEmpty(const Container& container, std::string_view paramName) {
        if (container.empty()) {
            throw ValidationError(std::string(paramName) + " cannot be empty");
        }
    }
    
    template<typename Container>
    static void requireMaxSize(const Container& container, size_t maxSize, std::string_view paramName) {
        if (container.size() > maxSize) {
            throw ValidationError(std::string(paramName) + " exceeds maximum size of " + 
                                std::to_string(maxSize));
        }
    }
    
    // Pointer validation
    template<typename T>
    static void requireNonNull(const T* ptr, std::string_view paramName) {
        if (ptr == nullptr) {
            throw ValidationError(std::string(paramName) + " cannot be null");
        }
    }
    
    // URL/endpoint validation
    static void requireValidEndpoint(std::string_view endpoint) {
        requireNonEmpty(endpoint, "endpoint");
        if (endpoint.length() > 500) {
            throw ValidationError("Endpoint URL too long");
        }
        
        // Basic URL validation
        if (endpoint.find_first_of(" \t\n\r") != std::string_view::npos) {
            throw ValidationError("Endpoint contains invalid characters");
        }
    }
    
    // Token validation
    static void requireValidAuthToken(std::string_view token) {
        requireNonEmpty(token, "auth token");
        if (token.length() < 10) {
            throw ValidationError("Auth token appears to be invalid (too short)");
        }
        if (token.length() > 500) {
            throw ValidationError("Auth token too long");
        }
    }
    
    // Date validation helpers
    static void requireValidDateString(std::string_view date, std::string_view paramName) {
        requireNonEmpty(date, paramName);
        
        // Basic ISO date format validation (YYYY-MM-DD)
        static const std::regex datePattern("^\\d{4}-\\d{2}-\\d{2}$");
        if (!std::regex_match(std::string(date), datePattern)) {
            throw ValidationError(std::string(paramName) + " must be in YYYY-MM-DD format");
        }
    }
};

// RAII validation context for better error messages
class ValidationContext {
private:
    std::string context_;
    
public:
    explicit ValidationContext(std::string context) : context_(std::move(context)) {}
    
    template<typename F>
    void validate(F&& validationFunction) const {
        try {
            validationFunction();
        } catch (const ValidationError& e) {
            throw ValidationError(context_ + ": " + e.what());
        }
    }
};

// Macro for easy validation context
#define VALIDATE_IN_CONTEXT(context, expr) \
    do { \
        try { \
            expr; \
        } catch (const ValidationError& e) { \
            throw ValidationError(std::string(context) + ": " + e.what()); \
        } \
    } while(0)

} // namespace validation
} // namespace tradier