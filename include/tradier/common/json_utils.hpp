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

#include <nlohmann/json.hpp>
#include "tradier/common/types.hpp"
#include <functional>
#include <memory>
#include <optional>

namespace tradier {
namespace json {

class SafeJsonParser {
private:
    nlohmann::json json_;
    bool valid_;
    std::string error_;

public:
    explicit SafeJsonParser(const std::string& jsonString);
    explicit SafeJsonParser(const Response& response);
    explicit SafeJsonParser(const nlohmann::json& json) : json_(json), valid_(true) {}
    
    bool isValid() const noexcept { return valid_; }
    const nlohmann::json& get() const noexcept { return json_; }
    const std::string& error() const noexcept { return error_; }
    
    std::string value(const std::string& key, const std::string& defaultValue) const noexcept;
    double value(const std::string& key, double defaultValue) const noexcept;
    int value(const std::string& key, int defaultValue) const noexcept;
    bool value(const std::string& key, bool defaultValue) const noexcept;
    long value(const std::string& key, long defaultValue) const noexcept;
    
    template<typename T>
    T value(const std::string& key, const T& defaultValue) const noexcept {
        if (!valid_ || !json_.contains(key)) {
            return defaultValue;
        }
        try {
            return json_[key].get<T>();
        } catch (const nlohmann::json::exception&) {
            return defaultValue;
        }
    }
    
    bool contains(const std::string& key) const noexcept;
    
    SafeJsonParser operator[](const std::string& key) const {
        if (!valid_ || !json_.contains(key)) {
            return SafeJsonParser(nlohmann::json::object());
        }
        return SafeJsonParser(json_[key]);
    }
    
    SafeJsonParser operator[](const char* key) const {
        return operator[](std::string(key));
    }
    
    SafeJsonParser operator[](size_t index) const {
        if (!valid_ || !json_.is_array() || index >= json_.size()) {
            return SafeJsonParser(nlohmann::json::object());
        }
        return SafeJsonParser(json_[index]);
    }
    
    bool is_array() const noexcept {
        return valid_ && json_.is_array();
    }
    
    bool is_object() const noexcept {
        return valid_ && json_.is_object();
    }
    
    size_t size() const noexcept {
        return valid_ ? json_.size() : 0;
    }
    
    bool empty() const noexcept {
        return !valid_ || json_.empty();
    }
    
    operator double() const noexcept;
    operator std::string() const noexcept;
    operator int() const noexcept;
    operator bool() const noexcept;
    operator long() const noexcept;
};

class JsonValidator {
private:
    std::vector<std::string> errors_;
    
public:
    JsonValidator() = default;
    
    template<typename T>
    bool validateField(const nlohmann::json& json, const std::string& fieldName, bool required = true) {
        if (!json.contains(fieldName)) {
            if (required) {
                errors_.push_back("Missing required field: " + fieldName);
                return false;
            }
            return true;
        }
        
        try {
            json[fieldName].get<T>();
            return true;
        } catch (const nlohmann::json::exception&) {
            errors_.push_back("Invalid type for field: " + fieldName);
            return false;
        }
    }
    
    bool validateObject(const nlohmann::json& json, const std::string& objectName, bool required = true);
    bool validateArray(const nlohmann::json& json, const std::string& arrayName, bool required = true);
    
    bool hasErrors() const { return !errors_.empty(); }
    const std::vector<std::string>& getErrors() const { return errors_; }
    void clearErrors() { errors_.clear(); }
    std::string getErrorString() const;
};

TimePoint parseDateTime(const nlohmann::json& json, const std::string& key);
std::string formatDateTime(const TimePoint& time);

template<typename T>
std::optional<T> parseResponse(const Response& response, std::function<T(const nlohmann::json&)> parser) {
    if (!response.success()) {
        return std::nullopt;
    }
    
    try {
        auto json = nlohmann::json::parse(response.body);
        return parser(json);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

template<typename T>
std::optional<T> parseResponseSafe(const Response& response, std::function<T(const SafeJsonParser&)> parser) {
    SafeJsonParser jsonParser(response);
    if (!jsonParser.isValid()) {
        return std::nullopt;
    }
    
    try {
        return parser(jsonParser);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

} // namespace json
} // namespace tradier
