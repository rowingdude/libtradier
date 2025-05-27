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

namespace tradier {

struct Config {
    std::string accessToken;
    std::string accountNumber;
    bool sandboxMode = true;
    int timeoutSeconds = 30;
    
    static Config fromEnvironment();
    
    std::string baseUrl() const {
        return sandboxMode ? "https://sandbox.tradier.com/v1" : "https://api.tradier.com/v1";
    }
    
    std::string wsUrl() const {
        return sandboxMode ? "wss://sandbox.tradier.com/v1" : "wss://api.tradier.com/v1";
    }
};

}