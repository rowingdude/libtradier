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
    std::mutex queueMutex_;
    std::condition_variable condition_;
    std::atomic<bool> stopped_{false};
    
public:
    explicit ThreadPool(size_t numThreads = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    
                    {
                        std::unique_lock<std::mutex> lock(queueMutex_);
                        condition_.wait(lock, [this] { return stopped_ || !tasks_.empty(); });
                        
                        if (stopped_ && tasks_.empty()) {
                            return;
                        }
                        
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    
                    task();
                }
            });
        }
    }
    
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            stopped_ = true;
        }
        
        condition_.notify_all();
        
        for (std::thread& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
    
    template<typename F, typename... Args>
    auto enqueue(F&& func, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
        using ReturnType = typename std::result_of<F(Args...)>::type;
        
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(func), std::forward<Args>(args)...)
        );
        
        std::future<ReturnType> result = task->get_future();
        
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            
            if (stopped_) {
                throw std::runtime_error("ThreadPool is stopped");
            }
            
            tasks_.emplace([task] { (*task)(); });
        }
        
        condition_.notify_one();
        return result;
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
    auto future = makeAsync<T>(std::move(syncFunc));
    
    ThreadPool::getInstance().enqueue([future = std::move(future), callback = std::move(callback)]() mutable {
        try {
            auto result = future.get();
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
    
public:
    Promise() : promise_(std::make_shared<std::promise<ApiResult<T>>>()) {
        future_ = promise_->get_future().share();
    }
    
    void resolve(const T& value) {
        promise_->set_value(ApiResult<T>(value));
    }
    
    void reject(const ApiError& error) {
        promise_->set_value(ApiResult<T>(error));
    }
    
    std::shared_future<ApiResult<T>> getFuture() const {
        return future_;
    }
    
    template<typename U>
    Promise<U> then(std::function<ApiResult<U>(const T&)> func) {
        Promise<U> nextPromise;
        
        ThreadPool::getInstance().enqueue([this, func, nextPromise]() mutable {
            try {
                auto result = future_.get();
                if (result.isSuccess()) {
                    auto nextResult = func(result.value());
                    if (nextResult.isSuccess()) {
                        nextPromise.resolve(nextResult.value());
                    } else {
                        nextPromise.reject(nextResult.error());
                    }
                } else {
                    nextPromise.reject(result.error());
                }
            } catch (const std::exception& e) {
                nextPromise.reject(ApiError(0, e.what()));
            }
        });
        
        return nextPromise;
    }
    
    Promise<T> catch_(std::function<void(const ApiError&)> errorHandler) {
        Promise<T> nextPromise;
        
        ThreadPool::getInstance().enqueue([this, errorHandler, nextPromise]() mutable {
            try {
                auto result = future_.get();
                if (result.isSuccess()) {
                    nextPromise.resolve(result.value());
                } else {
                    errorHandler(result.error());
                    nextPromise.reject(result.error());
                }
            } catch (const std::exception& e) {
                auto error = ApiError(0, e.what());
                errorHandler(error);
                nextPromise.reject(error);
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