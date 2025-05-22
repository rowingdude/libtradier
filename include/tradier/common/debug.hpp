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
#include <fstream>
#include <iostream>
#include "tradier/common/types.hpp"

namespace tradier {
namespace debug {

class Logger {
public:
    enum class Level {
        NONE = 0,
        ERROR = 1,
        WARN = 2,
        INFO = 3,
        DEBUG = 4,
        TRACE = 5
    };

    static Logger& getInstance();
    
    void setLevel(Level level);
    Level getLevel() const;
    void setEnabled(bool enabled);
    bool isEnabled() const;
    
    void log(Level level, const std::string& message);
    void error(const std::string& message);
    void warn(const std::string& message); 
    void info(const std::string& message);
    void debug(const std::string& message);
    void trace(const std::string& message);

private:
    Logger() = default;
    Level level_ = Level::NONE;
    bool enabled_ = false;
    
    std::string levelToString(Level level) const;
};

void enableDebugLogging(Logger::Level level = Logger::Level::DEBUG);
void disableDebugLogging();

void logHttpRequest(
    const std::string& method,
    const std::string& url, 
    const Headers& headers,
    const std::string& body = ""
);

void logHttpResponse(
    const std::string& endpoint,
    const Response& response,
    bool saveToFile = true
);

void saveResponseToFile(
    const std::string& response,
    const std::string& filename
);

void logApiError(
    const std::string& operation,
    const std::string& error,
    const Response* response = nullptr
);

void logJsonParseError(
    const std::string& operation,
    const std::string& jsonError,
    const std::string& responseBody
);

#define DEBUG_LOG(message) \
    if (tradier::debug::Logger::getInstance().isEnabled()) { \
        tradier::debug::Logger::getInstance().debug(message); \
    }

#define TRACE_LOG(message) \
    if (tradier::debug::Logger::getInstance().isEnabled()) { \
        tradier::debug::Logger::getInstance().trace(message); \
    }

#define ERROR_LOG(message) \
    if (tradier::debug::Logger::getInstance().isEnabled()) { \
        tradier::debug::Logger::getInstance().error(message); \
    }

} // namespace debug
} // namespace tradier