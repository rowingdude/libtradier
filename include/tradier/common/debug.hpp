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
#include <thread>
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
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

    void enableAsyncLogging(bool enable = true);
    size_t getDroppedMessageCount() const;
    void resetDroppedMessageCount();
    size_t getQueueSize() const;

private:
    Logger() = default;
    ~Logger();
    Level level_ = Level::NONE;
    bool enabled_ = false;
    
    std::atomic<bool> asyncLogging_{false};
    std::unique_ptr<std::thread> logThread_;
    std::queue<std::string> logQueue_;
    mutable std::mutex queueMutex_;
    std::condition_variable queueCv_;
    std::atomic<bool> stopLogging_{false};
    std::atomic<size_t> droppedMessages_{0};
    static constexpr size_t MAX_QUEUE_SIZE = 10000;
    
    std::string levelToString(Level level) const;
    void processLogQueue();
    std::string getCurrentThreadId() const;
};

void logThreadInfo(const std::string& operation, const std::string& details = "");
void logPerformanceMetric(const std::string& operation, std::chrono::milliseconds duration);

class PerformanceTimer {
    std::string operation_;
    std::chrono::steady_clock::time_point start_;
    
public:
    explicit PerformanceTimer(const std::string& operation);
    ~PerformanceTimer();
};

void configureProductionLogging();
void configureDebugLogging();

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

#define PERF_TIMER(operation) tradier::debug::PerformanceTimer _timer(operation)

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

}
}