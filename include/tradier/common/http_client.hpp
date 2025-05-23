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

#include "tradier/common/types.hpp"
#include "tradier/common/config.hpp"

namespace tradier {

class HttpClient {
private:
    Config config_;
    void* curlHandle_;
    
    std::string buildUrl(const std::string& endpoint) const;
    Headers buildHeaders(const Headers& additional = {}) const;
    std::string urlEncode(const std::string& value) const;
public:
    explicit HttpClient(const Config& config);
    ~HttpClient();
    
    HttpClient(HttpClient&&) noexcept;
    HttpClient& operator=(HttpClient&&) noexcept;
    HttpClient(const HttpClient&) = delete;
    HttpClient& operator=(const HttpClient&) = delete;
    
    Response get(const std::string& endpoint, const QueryParams& params = {});
    Response post(const std::string& endpoint, const FormParams& params = {});
    Response put(const std::string& endpoint, const FormParams& params = {});
    Response del(const std::string& endpoint, const QueryParams& params = {});
};

}