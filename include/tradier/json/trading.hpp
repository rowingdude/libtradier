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
#include "tradier/trading.hpp"

namespace tradier {
namespace json {

OrderResponse parseOrderResponse(const nlohmann::json& json);
OrderPreview parseOrderPreview(const nlohmann::json& json);

} // namespace json
} // namespace tradier