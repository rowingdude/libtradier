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

#include "tradier/common/config.hpp"
#include <cstdlib>

namespace tradier {

Config Config::fromEnvironment() {
    Config config;
    
    const char* sandboxEnv = std::getenv("TRADIER_SBX_ENABLE");
    if (sandboxEnv) {
        std::string sandboxStr(sandboxEnv);
        if (sandboxStr == "0" || 
            sandboxStr == "false" || 
            sandboxStr == "no" || 
            sandboxStr == "off") {
            config.sandboxMode = false;
        }
    }
    
    if (config.sandboxMode) {
        const char* sbxToken = std::getenv("TRADIER_SBX_TOKEN");
        const char* sbxAccNum = std::getenv("TRADIER_SBX_ACCNUM");
        
        if (sbxToken) {
            config.accessToken = sbxToken;
        }
        
        if (sbxAccNum) {
            config.accountNumber = sbxAccNum;
        }
    } else {
        const char* prodToken = std::getenv("TRADIER_PROD_TOKEN");
        
        if (prodToken) {
            config.accessToken = prodToken;
        }
    }
    
    const char* timeoutEnv = std::getenv("TRADIER_API_TIMEOUT");
    if (timeoutEnv) {
        try {
            config.timeoutSeconds = std::stoi(timeoutEnv);
        } catch (...) {  /* Keep default if invalid */  }
    }
    
    return config;
}

}