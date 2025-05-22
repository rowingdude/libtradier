#pragma once

#include <nlohmann/json.hpp>

namespace tradier {

struct OrderResponse;
struct OrderPreview;

namespace json {

OrderResponse parseOrderResponse(const nlohmann::json& json);
OrderPreview parseOrderPreview(const nlohmann::json& json);

} // namespace json
} // namespace tradier