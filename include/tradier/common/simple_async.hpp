/*
 * libtradier - Tradier API C++ Library v0.1.0
 * Simple and safe async implementation
 */

#pragma once

#include <future>
#include <functional>
#include "tradier/common/api_result.hpp"

namespace tradier {

// Simple async wrapper using std::async
template<typename T>
using SimpleAsyncResult = std::future<ApiResult<T>>;

template<typename T>
using SimpleAsyncCallback = std::function<void(const ApiResult<T>&)>;

// Simple async execution using std::async
template<typename T>
SimpleAsyncResult<T> makeSimpleAsync(std::function<ApiResult<T>()> syncFunc) {
    return std::async(std::launch::async, std::move(syncFunc));
}

// Simple callback-based async execution
template<typename T>
void executeSimpleAsync(std::function<ApiResult<T>()> syncFunc, SimpleAsyncCallback<T> callback) {
    auto future = makeSimpleAsync<T>(std::move(syncFunc));
    
    // Launch another async task to handle the callback
    std::async(std::launch::async, [future = std::move(future), callback = std::move(callback)]() mutable {
        try {
            auto result = future.get();
            callback(result);
        } catch (const std::exception& e) {
            callback(ApiResult<T>::internalError(e.what()));
        }
    });
}

} // namespace tradier