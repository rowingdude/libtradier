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

#include "tradier/json/streaming.hpp"
#include "tradier/common/json_utils.hpp"

namespace tradier {
namespace json {

StreamSession parseStreamSession(const nlohmann::json& json) {
    StreamSession session;
    
    if (!json.is_null() && json.is_object()) {
        if (json.contains("stream") && !json["stream"].is_null() && json["stream"].is_object()) {
            const auto& stream = json["stream"];
            session.url = stream.value("url", "");
            session.sessionId = stream.value("sessionid", "");
        } else {
            session.url = json.value("url", "");
            session.sessionId = json.value("sessionid", "");
        }
        
        session.isActive = !session.url.empty() && !session.sessionId.empty();
        session.expiresAt = std::chrono::system_clock::now() + std::chrono::hours(8);
    }
    
    return session;
}

} // namespace json
} // namespace tradier