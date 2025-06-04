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

#include <stdexcept>
#include <string>

namespace tradier {

class TradierException : public std::runtime_error {
public:
    explicit TradierException(const std::string& message) : std::runtime_error(message) {}
};

class AuthenticationError : public TradierException {
public:
    explicit AuthenticationError(const std::string& message) : TradierException("Auth: " + message) {}
};

class ConnectionError : public TradierException {
public:
    explicit ConnectionError(const std::string& message) : TradierException("Connection: " + message) {}
};

class ValidationError : public TradierException {
public:
    explicit ValidationError(const std::string& message) : TradierException("Validation: " + message) {}
};

class TimeoutError : public TradierException {
public:
    explicit TimeoutError(const std::string& message) : TradierException("Timeout: " + message) {}
};

class ApiError : public TradierException {
public:
    int statusCode;
    std::string endpoint;
    
    ApiError(int code, const std::string& message, const std::string& ep = "") 
        : TradierException("API(" + std::to_string(code) + "): " + message), 
          statusCode(code), endpoint(ep) {}
    
    std::string toString() const {
        std::string result = what();
        if (!endpoint.empty()) {
            result += " [" + endpoint + "]";
        }
        return result;
    }
    
    bool isRetryable() const {
        return statusCode >= 500 || statusCode == 429; 
    }
};

}