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
    
    if (json.contains("stream")) {
        const auto& stream = json["stream"];
        session.url = stream.value("url", "");
        session.sessionId = stream.value("sessionid", "");
    }
    
    return session;
}

} // namespace json
} // namespace tradier