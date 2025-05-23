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

#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace tradier::utils {

using TimePoint = std::chrono::system_clock::time_point;

inline TimePoint parseISODateTime(const std::string& dateTime) {
    std::tm tm = {};
    std::istringstream ss(dateTime);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    if (ss.fail()) return TimePoint{};
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

inline std::string formatISODateTime(const TimePoint& timePoint) {
    auto time = std::chrono::system_clock::to_time_t(timePoint);
    std::ostringstream ss;
    ss << std::put_time(std::gmtime(&time), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

inline std::string formatDate(const TimePoint& timePoint) {
    auto time = std::chrono::system_clock::to_time_t(timePoint);
    std::ostringstream ss;
    ss << std::put_time(std::gmtime(&time), "%Y-%m-%d");
    return ss.str();
}

inline std::string urlEncode(const std::string& value) {
    std::ostringstream encoded;
    encoded.fill('0');
    encoded << std::hex;
    
    for (unsigned char c : value) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded << c;
        } else {
            encoded << '%' << std::setw(2) << static_cast<int>(c);
        }
    }
    
    return encoded.str();
}

template<typename T>
std::string toString(const T& value) {
    return std::to_string(value);
}

template<>
inline std::string toString<std::string>(const std::string& value) {
    return value;
}

}