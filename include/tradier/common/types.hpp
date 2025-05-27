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
#include <map>
#include <vector>
#include <chrono>
#include <optional>
#include "tradier/common/api_result.hpp"

namespace tradier {

using QueryParams = std::map<std::string, std::string>;
using FormParams = std::map<std::string, std::string>;
using Headers = std::map<std::string, std::string>;
using TimePoint = std::chrono::system_clock::time_point;

struct Response {
    int status;
    std::string body;
    Headers headers;
    
    bool success() const { return status >= 200 && status < 300; }
};

template<typename T>
class ApiResult;

template<typename T>
using Result = ApiResult<T>;


}