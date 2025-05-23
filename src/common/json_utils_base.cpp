#include "tradier/common/json_utils.hpp"
#include "tradier/common/utils.hpp"

namespace tradier {
namespace json {

TimePoint parseDateTime(const nlohmann::json& json, const std::string& key) {
    if (json.contains(key) && !json[key].is_null()) {
        std::string dateStr = json[key];
        return utils::parseISODateTime(dateStr);
    }
    return TimePoint{};
}

std::string formatDateTime(const TimePoint& time) {
    return utils::formatISODateTime(time);
}

}
}