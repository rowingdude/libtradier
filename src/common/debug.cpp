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

#include "tradier/common/debug.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <memory>
#include <fstream>

namespace tradier {
namespace debug {

class ScopedFileLogger {
private:
    std::unique_ptr<std::ofstream> file_;
    std::string filename_;
    bool autoDelete_;

public:
    explicit ScopedFileLogger(const std::string& filename, bool autoDelete = false) 
        : filename_(filename), autoDelete_(autoDelete) {
        file_ = std::make_unique<std::ofstream>(filename, std::ios::out | std::ios::app);
        if (!file_->is_open()) {
            throw std::runtime_error("Failed to open log file: " + filename);
        }
    }
    
    ~ScopedFileLogger() {
        if (file_ && file_->is_open()) {
            file_->close();
        }
        if (autoDelete_) {
            std::remove(filename_.c_str());
        }
    }
    
    ScopedFileLogger(const ScopedFileLogger&) = delete;
    ScopedFileLogger& operator=(const ScopedFileLogger&) = delete;
    
    ScopedFileLogger(ScopedFileLogger&&) = default;
    ScopedFileLogger& operator=(ScopedFileLogger&&) = default;
    
    template<typename T>
    ScopedFileLogger& operator<<(const T& value) {
        if (file_ && file_->is_open()) {
            *file_ << value;
            file_->flush();
        }
        return *this;
    }
    
    void log(const std::string& message) {
        if (file_ && file_->is_open()) {
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            
            *file_ << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") 
                   << "] " << message << std::endl;
            file_->flush();
        }
    }
};

class LoggerSession {
private:
    Logger::Level originalLevel_;
    bool wasEnabled_;

public:
    explicit LoggerSession(Logger::Level level) {
        Logger& logger = Logger::getInstance();
        originalLevel_ = logger.getLevel();
        wasEnabled_ = logger.isEnabled();
        
        logger.setLevel(level);
        logger.setEnabled(true);
    }
    
    ~LoggerSession() {
        Logger& logger = Logger::getInstance();
        logger.setLevel(originalLevel_);
        logger.setEnabled(wasEnabled_);
    }
    
    LoggerSession(const LoggerSession&) = delete;
    LoggerSession& operator=(const LoggerSession&) = delete;
    
    LoggerSession(LoggerSession&&) = delete;
    LoggerSession& operator=(LoggerSession&&) = delete;
};

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::setLevel(Level level) {
    level_ = level;
}

Logger::Level Logger::getLevel() const {
    return level_;
}

void Logger::setEnabled(bool enabled) {
    enabled_ = enabled;
}

bool Logger::isEnabled() const {
    return enabled_;
}

void Logger::log(Level level, const std::string& message) {
    if (!enabled_ || level > level_) {
        return;
    }
    
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::tm tm = *std::localtime(&time_t);
    
    std::cerr << "[" << std::put_time(&tm, "%H:%M:%S") 
              << "." << std::setfill('0') << std::setw(3) << ms.count()
              << "] [" << levelToString(level) << "] " << message << std::endl;
}

void Logger::error(const std::string& message) {
    log(Level::ERROR, message);
}

void Logger::warn(const std::string& message) {
    log(Level::WARN, message);
}

void Logger::info(const std::string& message) {
    log(Level::INFO, message);
}

void Logger::debug(const std::string& message) {
    log(Level::DEBUG, message);
}

void Logger::trace(const std::string& message) {
    log(Level::TRACE, message);
}

std::string Logger::levelToString(Level level) const {
    switch (level) {
        case Level::ERROR: return "ERROR";
        case Level::WARN:  return "WARN ";
        case Level::INFO:  return "INFO ";
        case Level::DEBUG: return "DEBUG";
        case Level::TRACE: return "TRACE";
        default:           return "NONE ";
    }
}

void enableDebugLogging(Logger::Level level) {
    Logger::getInstance().setEnabled(true);
    Logger::getInstance().setLevel(level);
}

void disableDebugLogging() {
    Logger::getInstance().setEnabled(false);
}

void logHttpRequest(
    const std::string& method,
    const std::string& url, 
    const Headers& headers,
    const std::string& body) {
    
    Logger& logger = Logger::getInstance();
    
    logger.debug("=== HTTP REQUEST ===");
    logger.debug("Method: " + method);
    logger.debug("URL: " + url);
    
    logger.debug("Headers:");
    for (const auto& [key, value] : headers) {
        if (key == "Authorization") {
            std::string maskedValue = value.substr(0, 12) + "...";
            logger.debug("  " + key + ": " + maskedValue);
        } else {
            logger.debug("  " + key + ": " + value);
        }
    }
    
    if (!body.empty()) {
        logger.debug("Body: " + body);
    }
    
    logger.debug("=== END REQUEST ===");
}

void logHttpResponse(
    const std::string& endpoint,
    const Response& response,
    bool saveToFile) {
    
    Logger& logger = Logger::getInstance();
    
    logger.debug("=== HTTP RESPONSE ===");
    logger.debug("Endpoint: " + endpoint);
    logger.debug("Status: " + std::to_string(response.status));
    
    logger.debug("Response Headers:");
    for (const auto& [key, value] : response.headers) {
        logger.debug("  " + key + ": " + value);
    }
    
    std::string bodyPreview = response.body.substr(0, std::min(size_t(200), response.body.size()));
    if (response.body.size() > 200) {
        bodyPreview += "...";
    }
    logger.debug("Body Preview: " + bodyPreview);
    
    if (saveToFile && response.status != 200) {
        std::string filename = "debug_response_" + std::to_string(response.status) + "_" + 
                               std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
                                   std::chrono::system_clock::now().time_since_epoch()).count()) + 
                               ".html";
        try {
            saveResponseToFile(response.body, filename);
            logger.debug("Full response saved to: " + filename);
        } catch (const std::exception& e) {
            logger.debug("Failed to save response to file: " + std::string(e.what()));
        }
    }
    
    logger.debug("=== END RESPONSE ===");
}

void saveResponseToFile(
    const std::string& response,
    const std::string& filename) {
    
    try {
        ScopedFileLogger fileLogger(filename);
        fileLogger.log(response);
    } catch (const std::exception&) {
        // Silent failure - logging shouldn't break the application
    }
}

void logApiError(
    const std::string& operation,
    const std::string& error,
    const Response* response) {
    
    Logger& logger = Logger::getInstance();
    
    logger.error("=== API ERROR ===");
    logger.error("Operation: " + operation);
    logger.error("Error: " + error);
    
    if (response) {
        logger.error("HTTP Status: " + std::to_string(response->status));
        logger.error("Response Body Preview: " + 
                    response->body.substr(0, std::min(size_t(300), response->body.size())));
    }
    
    logger.error("=== END API ERROR ===");
}

void logJsonParseError(
    const std::string& operation,
    const std::string& jsonError,
    const std::string& responseBody) {
    
    Logger& logger = Logger::getInstance();
    
    logger.error("=== JSON PARSE ERROR ===");
    logger.error("Operation: " + operation);
    logger.error("JSON Error: " + jsonError);
    logger.error("Response Body (first 500 chars):");
    logger.error(responseBody.substr(0, std::min(size_t(500), responseBody.size())));
    
    if (responseBody.find("<html") != std::string::npos || 
        responseBody.find("<!DOCTYPE") != std::string::npos) {
        logger.error("*** Response appears to be HTML instead of JSON! ***");
        logger.error("*** This usually indicates authentication failure ***");
        
        try {
            saveResponseToFile(responseBody, "json_parse_error_response.html");
            logger.error("Full HTML response saved to: json_parse_error_response.html");
        } catch (const std::exception&) {
            logger.error("Failed to save HTML response to file");
        }
    }
    
    logger.error("=== END JSON PARSE ERROR ===");
}

} // namespace debug
} // namespace tradier