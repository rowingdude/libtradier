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

#include <future>
#include <functional>
#include <thread>
#include <atomic>
#include <chrono>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <memory>
#include <cmath>
#include "tradier/common/api_result.hpp"
#include "tradier/common/errors.hpp"

namespace tradier {

template<typename T>
using AsyncResult = std::future<ApiResult<T>>;

template<typename T>
using AsyncCallback = std::function<void(const ApiResult<T>&)>;

// Thread pool for async operations
class ThreadPool {
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    mutable std::mutex queueMutex_;
    std::condition_variable condition_;
    std::atomic<bool> stopped_{false};
    std::atomic<bool> initialized_{false};
    
    // Prevent copying and assignment
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    
public:
    explicit ThreadPool(size_t numThreads = std::thread::hardware_concurrency()) {
        if (numThreads == 0) {
            numThreads = 1; // Fallback to at least 1 thread
        }
        
        workers_.reserve(numThreads);
        
        for (size_t i = 0; i < numThreads; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    
                    {
                        std::unique_lock<std::mutex> lock(queueMutex_);
                        condition_.wait(lock, [this] { 
                            return stopped_.load(std::memory_order_acquire) || !tasks_.empty(); 
                        });
                        
                        if (stopped_.load(std::memory_order_acquire) && tasks_.empty()) {
                            return;
                        }
                        
                        if (!tasks_.empty()) {
                            task = std::move(tasks_.front());
                            tasks_.pop();
                        }
                    }
                    
                    // Execute task outside of lock to prevent deadlocks
                    if (task) {
                        try {
                            task();
                        } catch (...) {
                            // Prevent uncaught exceptions from terminating worker threads
                        }
                    }
                }
            });
        }
        
        initialized_.store(true, std::memory_order_release);
    }
    
    ~ThreadPool() {
        shutdown();
    }
    
    void shutdown() {
        if (!initialized_.load(std::memory_order_acquire)) {
            return;
        }
        
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            stopped_.store(true, std::memory_order_release);
        }
        
        condition_.notify_all();
        
        for (std::thread& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        
        // Clear any remaining tasks to prevent memory leaks
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            while (!tasks_.empty()) {
                tasks_.pop();
            }
        }
    }
    
    template<typename F, typename... Args>
    auto enqueue(F&& func, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type> {
        using ReturnType = typename std::invoke_result<F, Args...>::type;
        
        if (stopped_.load(std::memory_order_acquire)) {
            throw std::runtime_error("ThreadPool is stopped");
        }
        
        // Use unique_ptr instead of shared_ptr to avoid circular references
        auto taskPtr = std::make_unique<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(func), std::forward<Args>(args)...)
        );
        
        std::future<ReturnType> result = taskPtr->get_future();
        
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            
            if (stopped_.load(std::memory_order_acquire)) {
                throw std::runtime_error("ThreadPool is stopped");
            }
            
            // Create a shared_ptr for the lambda capture to make it copyable
            auto sharedTask = std::shared_ptr<std::packaged_task<ReturnType()>>(taskPtr.release());
            tasks_.emplace([sharedTask]() {
                try {
                    (*sharedTask)();
                } catch (...) {
                    // Task exceptions are propagated through the future
                }
            });
        }
        
        condition_.notify_one();
        return result;
    }
    
    // Get current queue size for monitoring
    size_t queueSize() const {
        std::unique_lock<std::mutex> lock(queueMutex_);
        return tasks_.size();
    }
    
    // Check if thread pool is active
    bool isActive() const noexcept {
        return initialized_.load(std::memory_order_acquire) && 
               !stopped_.load(std::memory_order_acquire);
    }
    
    static ThreadPool& getInstance() {
        static ThreadPool instance;
        return instance;
    }
};

// Async execution utilities
template<typename T>
AsyncResult<T> makeAsync(std::function<ApiResult<T>()> syncFunc) {
    return ThreadPool::getInstance().enqueue(std::move(syncFunc));
}

template<typename T>
void executeAsync(std::function<ApiResult<T>()> syncFunc, AsyncCallback<T> callback) {
    // Execute the function directly in the thread pool to avoid future chaining issues
    ThreadPool::getInstance().enqueue([syncFunc = std::move(syncFunc), callback = std::move(callback)]() mutable {
        try {
            auto result = syncFunc();
            callback(result);
        } catch (const std::exception& e) {
            callback(ApiResult<T>::internalError(e.what()));
        }
    });
}

// Timeout wrapper for async operations
template<typename T>
ApiResult<T> waitWithTimeout(AsyncResult<T>& future, std::chrono::milliseconds timeout) {
    auto status = future.wait_for(timeout);
    
    if (status == std::future_status::timeout) {
        return ApiResult<T>::internalError("Operation timed out");
    }
    
    return future.get();
}

// Promise-based API for better composability
template<typename T>
class Promise {
private:
    std::shared_ptr<std::promise<ApiResult<T>>> promise_;
    std::shared_future<ApiResult<T>> future_;
    std::weak_ptr<std::promise<ApiResult<T>>> weakPromise_;
    
public:
    Promise() : promise_(std::make_shared<std::promise<ApiResult<T>>>()) {
        future_ = promise_->get_future().share();
        weakPromise_ = promise_;
    }
    
    void resolve(const T& value) {
        if (auto p = weakPromise_.lock()) {
            try {
                p->set_value(ApiResult<T>(value));
            } catch (const std::future_error&) {
                // Promise already set, ignore
            }
        }
    }
    
    void reject(const ApiError& error) {
        if (auto p = weakPromise_.lock()) {
            try {
                p->set_value(ApiResult<T>(error));
            } catch (const std::future_error&) {
                // Promise already set, ignore
            }
        }
    }
    
    std::shared_future<ApiResult<T>> getFuture() const {
        return future_;
    }
    
    template<typename U>
    Promise<U> then(std::function<ApiResult<U>(const T&)> func) {
        Promise<U> nextPromise;
        auto nextWeakPromise = nextPromise.weakPromise_;
        auto currentFuture = future_;
        
        ThreadPool::getInstance().enqueue([currentFuture, func = std::move(func), nextWeakPromise]() mutable {
            try {
                auto result = currentFuture.get();
                if (auto nextPromise = nextWeakPromise.lock()) {
                    if (result.isSuccess()) {
                        auto nextResult = func(result.value());
                        if (nextResult.isSuccess()) {
                            try {
                                nextPromise->set_value(ApiResult<U>(nextResult.value()));
                            } catch (const std::future_error&) {}
                        } else {
                            try {
                                nextPromise->set_value(ApiResult<U>(nextResult.error()));
                            } catch (const std::future_error&) {}
                        }
                    } else {
                        try {
                            nextPromise->set_value(ApiResult<U>(result.error()));
                        } catch (const std::future_error&) {}
                    }
                }
            } catch (const std::exception& e) {
                if (auto nextPromise = nextWeakPromise.lock()) {
                    try {
                        nextPromise->set_value(ApiResult<U>(ApiError(0, e.what())));
                    } catch (const std::future_error&) {}
                }
            }
        });
        
        return nextPromise;
    }
    
    Promise<T> catch_(std::function<void(const ApiError&)> errorHandler) {
        Promise<T> nextPromise;
        auto nextWeakPromise = nextPromise.weakPromise_;
        auto currentFuture = future_;
        
        ThreadPool::getInstance().enqueue([currentFuture, errorHandler = std::move(errorHandler), nextWeakPromise]() mutable {
            try {
                auto result = currentFuture.get();
                if (auto nextPromise = nextWeakPromise.lock()) {
                    if (result.isSuccess()) {
                        try {
                            nextPromise->set_value(result);
                        } catch (const std::future_error&) {}
                    } else {
                        errorHandler(result.error());
                        try {
                            nextPromise->set_value(result);
                        } catch (const std::future_error&) {}
                    }
                }
            } catch (const std::exception& e) {
                auto error = ApiError(0, e.what());
                errorHandler(error);
                if (auto nextPromise = nextWeakPromise.lock()) {
                    try {
                        nextPromise->set_value(ApiResult<T>(error));
                    } catch (const std::future_error&) {}
                }
            }
        });
        
        return nextPromise;
    }
};

// Rate limiter for async operations
class RateLimiter {
private:
    std::chrono::steady_clock::time_point windowStart_;
    std::atomic<int> requestCount_{0};
    const int maxRequests_;
    const std::chrono::milliseconds windowDuration_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    
public:
    RateLimiter(int maxRequests, std::chrono::milliseconds windowDuration)
        : maxRequests_(maxRequests), windowDuration_(windowDuration) {
        windowStart_ = std::chrono::steady_clock::now();
    }
    
    bool tryAcquire() {
        std::unique_lock<std::mutex> lock(mutex_);
        auto now = std::chrono::steady_clock::now();
        
        if (now - windowStart_ > windowDuration_) {
            windowStart_ = now;
            requestCount_ = 0;
        }
        
        if (requestCount_ < maxRequests_) {
            ++requestCount_;
            return true;
        }
        
        return false;
    }
    
    void waitForSlot() {
        std::unique_lock<std::mutex> lock(mutex_);
        
        while (!tryAcquireInternal()) {
            auto timeToWait = windowDuration_ - (std::chrono::steady_clock::now() - windowStart_);
            if (timeToWait > std::chrono::milliseconds(0)) {
                condition_.wait_for(lock, timeToWait);
            }
        }
    }
    
private:
    bool tryAcquireInternal() {
        auto now = std::chrono::steady_clock::now();
        
        if (now - windowStart_ > windowDuration_) {
            windowStart_ = now;
            requestCount_ = 0;
        }
        
        if (requestCount_ < maxRequests_) {
            ++requestCount_;
            return true;
        }
        
        return false;
    }
};

// Retry mechanism with exponential backoff
template<typename T>
ApiResult<T> retryWithBackoff(
    std::function<ApiResult<T>()> operation,
    int maxRetries = 3,
    std::chrono::milliseconds initialDelay = std::chrono::milliseconds(1000),
    double backoffMultiplier = 2.0
) {
    ApiResult<T> lastResult = ApiResult<T>::internalError("No attempts made");
    
    for (int attempt = 0; attempt < maxRetries; ++attempt) {
        lastResult = operation();
        
        if (lastResult.isSuccess()) {
            return lastResult;
        }
        
        // Don't retry on client errors (4xx), only server errors (5xx) and network issues
        if (lastResult.error().statusCode >= 400 && lastResult.error().statusCode < 500) {
            break;
        }
        
        if (attempt < maxRetries - 1) {
            auto delay = std::chrono::duration_cast<std::chrono::milliseconds>(
                initialDelay * std::pow(backoffMultiplier, attempt)
            );
            std::this_thread::sleep_for(delay);
        }
    }
    
    return lastResult;
}

} // namespace tradier