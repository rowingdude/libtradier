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

#include <optional>
#include <string>
#include <functional>
#include <type_traits>
#include "tradier/common/errors.hpp"

namespace tradier {

enum class ErrorCategory {
    NONE = 0,
    NETWORK,
    AUTHENTICATION,
    VALIDATION,
    API_ERROR,
    PARSING,
    INTERNAL
};

template<typename T>
class ApiResult {
private:
    std::optional<T> value_;
    std::optional<ApiError> error_;
    
public:
    using value_type = T;
    
    ApiResult(T&& val) : value_(std::move(val)) {}
    ApiResult(const T& val) : value_(val) {}
    
    ApiResult(ApiError err) : error_(std::move(err)) {}
    ApiResult(ErrorCategory category, const std::string& message) {
        if (category == ErrorCategory::API_ERROR) {
            error_ = ApiError(500, message);
        } else {
            error_ = ApiError(0, categoryToString(category) + ": " + message);
        }
    }
    ApiResult(ErrorCategory /* category */, int httpStatus, const std::string& message) {
        error_ = ApiError(httpStatus, message);
    }
    
    static ApiResult networkError(const std::string& message) {
        return ApiResult(ErrorCategory::NETWORK, message);
    }
    
    static ApiResult authError(const std::string& message) {
        return ApiResult(ErrorCategory::AUTHENTICATION, message);
    }
    
    static ApiResult validationError(const std::string& message) {
        return ApiResult(ErrorCategory::VALIDATION, message);
    }
    
    static ApiResult apiError(int httpStatus, const std::string& message) {
        return ApiResult(ErrorCategory::API_ERROR, httpStatus, message);
    }
    
    static ApiResult parseError(const std::string& message) {
        return ApiResult(ErrorCategory::PARSING, message);
    }
    
    static ApiResult internalError(const std::string& message) {
        return ApiResult(ErrorCategory::INTERNAL, message);
    }
    
    bool isSuccess() const { return value_.has_value(); }
    bool isError() const { return error_.has_value(); }
    bool isRetryable() const { 
        return error_ && error_->statusCode >= 500;
    }
    
    T& value() & { 
        if (!value_) throw ValidationError("Accessing value of failed ApiResult");
        return *value_; 
    }
    
    const T& value() const & { 
        if (!value_) throw ValidationError("Accessing value of failed ApiResult");
        return *value_; 
    }
    
    T&& value() && { 
        if (!value_) throw ValidationError("Accessing value of failed ApiResult");
        return std::move(*value_); 
    }
    
    const ApiError& error() const { 
        if (!error_) throw ValidationError("Accessing error of successful ApiResult");
        return *error_; 
    }
    
    T valueOr(const T& defaultValue) const {
        return value_ ? *value_ : defaultValue;
    }
    
    T valueOr(T&& defaultValue) const {
        return value_ ? *value_ : std::move(defaultValue);
    }
    
    template<typename F>
    auto andThen(F&& func) -> ApiResult<decltype(func(value()))> {
        using ReturnType = decltype(func(value()));
        
        if (isError()) {
            return ApiResult<ReturnType>(error());
        }
        
        try {
            return ApiResult<ReturnType>(func(value()));
        } catch (const ValidationError& e) {
            return ApiResult<ReturnType>::validationError(e.what());
        } catch (const AuthenticationError& e) {
            return ApiResult<ReturnType>::authError(e.what());
        } catch (const ConnectionError& e) {
            return ApiResult<ReturnType>::networkError(e.what());
        } catch (const ::tradier::ApiError& e) {
            return ApiResult<ReturnType>::apiError(e.statusCode, e.what());
        } catch (const std::exception& e) {
            return ApiResult<ReturnType>::internalError(e.what());
        }
    }
    
    template<typename F>
    auto map(F&& func) -> ApiResult<decltype(func(value()))> {
        using ReturnType = decltype(func(value()));
        
        if (isError()) {
            return ApiResult<ReturnType>(error());
        }
        
        try {
            return ApiResult<ReturnType>(func(value()));
        } catch (const ValidationError& e) {
            return ApiResult<ReturnType>::validationError(e.what());
        } catch (const AuthenticationError& e) {
            return ApiResult<ReturnType>::authError(e.what());
        } catch (const ConnectionError& e) {
            return ApiResult<ReturnType>::networkError(e.what());
        } catch (const ::tradier::ApiError& e) {
            return ApiResult<ReturnType>::apiError(e.statusCode, e.what());
        } catch (const std::exception& e) {
            return ApiResult<ReturnType>::internalError(e.what());
        }
    }
    
    template<typename F>
    ApiResult& orElse(F&& func) {
        if (isError()) {
            try {
                *this = func(error());
            } catch (...) {
            }
        }
        return *this;
    }
    
    explicit operator bool() const { return isSuccess(); }
    
    T* operator->() { 
        if (!value_) return nullptr;
        return &(*value_); 
    }
    
    const T* operator->() const { 
        if (!value_) return nullptr;
        return &(*value_); 
    }
    
    T& operator*() & { return value(); }
    const T& operator*() const & { return value(); }
    T&& operator*() && { return std::move(value()); }

private:
    std::string categoryToString(ErrorCategory cat) const {
        switch (cat) {
            case ErrorCategory::NETWORK: return "Network Error";
            case ErrorCategory::AUTHENTICATION: return "Authentication Error";
            case ErrorCategory::VALIDATION: return "Validation Error";
            case ErrorCategory::API_ERROR: return "API Error";
            case ErrorCategory::PARSING: return "Parsing Error";
            case ErrorCategory::INTERNAL: return "Internal Error";
            default: return "Unknown Error";
        }
    }
};

template<typename T, typename F>
ApiResult<T> tryExecute(F&& func, const std::string& operation = "") {
    try {
        if constexpr (std::is_void_v<T>) {
            func();
            return ApiResult<bool>(true); 
        } else {
            return ApiResult<T>(func());
        }
    } catch (const ValidationError& e) {
        return ApiResult<T>::validationError(operation + ": " + e.what());
    } catch (const AuthenticationError& e) {
        return ApiResult<T>::authError(operation + ": " + e.what());
    } catch (const ConnectionError& e) {
        return ApiResult<T>::networkError(operation + ": " + e.what());
    } catch (const ::tradier::ApiError& e) {
        return ApiResult<T>::apiError(e.statusCode, operation + ": " + e.what());
    } catch (const std::exception& e) {
        return ApiResult<T>::internalError(operation + ": " + e.what());
    }
}

using VoidResult = ApiResult<bool>;

}