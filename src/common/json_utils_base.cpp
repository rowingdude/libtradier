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

#include "tradier/common/json_utils.hpp"
#include "tradier/common/utils.hpp"

namespace tradier {
namespace json {

SafeJsonParser::SafeJsonParser(const std::string& jsonString) : valid_(false) {
    try {
        json_ = nlohmann::json::parse(jsonString);
        valid_ = true;
    } catch (const nlohmann::json::exception& e) {
        json_ = nlohmann::json::object();
        error_ = e.what();
    }
}

SafeJsonParser::SafeJsonParser(const Response& response) : valid_(false) {
    if (!response.success()) {
        json_ = nlohmann::json::object();
        error_ = "HTTP error: " + std::to_string(response.status);
        return;
    }
    
    try {
        json_ = nlohmann::json::parse(response.body);
        valid_ = true;
    } catch (const nlohmann::json::exception& e) {
        json_ = nlohmann::json::object();
        error_ = e.what();
    }
}

std::string SafeJsonParser::value(const std::string& key, const std::string& defaultValue) const noexcept {
    if (!valid_ || !json_.contains(key) || json_[key].is_null()) {
        return defaultValue;
    }
    try {
        return json_[key].get<std::string>();
    } catch (const nlohmann::json::exception&) {
        return defaultValue;
    }
}

double SafeJsonParser::value(const std::string& key, double defaultValue) const noexcept {
    if (!valid_ || !json_.contains(key) || json_[key].is_null()) {
        return defaultValue;
    }
    try {
        return json_[key].get<double>();
    } catch (const nlohmann::json::exception&) {
        return defaultValue;
    }
}

int SafeJsonParser::value(const std::string& key, int defaultValue) const noexcept {
    if (!valid_ || !json_.contains(key) || json_[key].is_null()) {
        return defaultValue;
    }
    try {
        return json_[key].get<int>();
    } catch (const nlohmann::json::exception&) {
        return defaultValue;
    }
}

bool SafeJsonParser::value(const std::string& key, bool defaultValue) const noexcept {
    if (!valid_ || !json_.contains(key) || json_[key].is_null()) {
        return defaultValue;
    }
    try {
        return json_[key].get<bool>();
    } catch (const nlohmann::json::exception&) {
        return defaultValue;
    }
}

long SafeJsonParser::value(const std::string& key, long defaultValue) const noexcept {
    if (!valid_ || !json_.contains(key) || json_[key].is_null()) {
        return defaultValue;
    }
    try {
        return json_[key].get<long>();
    } catch (const nlohmann::json::exception&) {
        return defaultValue;
    }
}

bool SafeJsonParser::contains(const std::string& key) const noexcept {
    return valid_ && json_.contains(key);
}

SafeJsonParser::operator double() const noexcept {
    if (!valid_ || json_.is_null()) {
        return 0.0;
    }
    try {
        return json_.get<double>();
    } catch (const nlohmann::json::exception&) {
        return 0.0;
    }
}

SafeJsonParser::operator std::string() const noexcept {
    if (!valid_ || json_.is_null()) {
        return "";
    }
    try {
        return json_.get<std::string>();
    } catch (const nlohmann::json::exception&) {
        return "";
    }
}

SafeJsonParser::operator int() const noexcept {
    if (!valid_ || json_.is_null()) {
        return 0;
    }
    try {
        return json_.get<int>();
    } catch (const nlohmann::json::exception&) {
        return 0;
    }
}

SafeJsonParser::operator bool() const noexcept {
    if (!valid_ || json_.is_null()) {
        return false;
    }
    try {
        return json_.get<bool>();
    } catch (const nlohmann::json::exception&) {
        return false;
    }
}

SafeJsonParser::operator long() const noexcept {
    if (!valid_ || json_.is_null()) {
        return 0L;
    }
    try {
        return json_.get<long>();
    } catch (const nlohmann::json::exception&) {
        return 0L;
    }
}

bool JsonValidator::validateObject(const nlohmann::json& json, const std::string& objectName, bool required) {
    if (!json.contains(objectName)) {
        if (required) {
            errors_.push_back("Missing required object: " + objectName);
            return false;
        }
        return true;
    }
    
    if (!json[objectName].is_object()) {
        errors_.push_back("Field is not an object: " + objectName);
        return false;
    }
    
    return true;
}

bool JsonValidator::validateArray(const nlohmann::json& json, const std::string& arrayName, bool required) {
    if (!json.contains(arrayName)) {
        if (required) {
            errors_.push_back("Missing required array: " + arrayName);
            return false;
        }
        return true;
    }
    
    if (!json[arrayName].is_array()) {
        errors_.push_back("Field is not an array: " + arrayName);
        return false;
    }
    
    return true;
}

std::string JsonValidator::getErrorString() const {
    if (errors_.empty()) return "";
    
    std::string result = "Validation errors: ";
    for (size_t i = 0; i < errors_.size(); ++i) {
        if (i > 0) result += ", ";
        result += errors_[i];
    }
    return result;
}

TimePoint parseDateTime(const nlohmann::json& json, const std::string& key) {
    if (json.contains(key) && !json[key].is_null()) {
        try {
            std::string dateStr = json[key];
            return utils::parseISODateTime(dateStr);
        } catch (const std::exception&) {
            return TimePoint{};
        }
    }
    return TimePoint{};
}

std::string formatDateTime(const TimePoint& time) {
    try {
        return utils::formatISODateTime(time);
    } catch (const std::exception&) {
        return "";
    }
}

}
}