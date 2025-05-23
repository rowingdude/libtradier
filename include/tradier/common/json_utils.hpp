#pragma once

#include <nlohmann/json.hpp>
#include "tradier/common/types.hpp"
#include <functional>

namespace tradier {
namespace json {

TimePoint parseDateTime(const nlohmann::json& json, const std::string& key);
std::string formatDateTime(const TimePoint& time);

template<typename T>
Result<T> parseResponse(const Response& response, std::function<T(const nlohmann::json&)> parser) {
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

}
}