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

#include "tradier/common/http_client.hpp"
#include "tradier/common/errors.hpp"
#include "tradier/common/utils.hpp"
#include <curl/curl.h>
#include <sstream>
#include <memory>

namespace tradier {

namespace {
    size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
        if (!data) return 0;
        try {
            data->append(static_cast<char*>(contents), size * nmemb);
            return size * nmemb;
        } catch (...) {
            return 0;
        }
    }

    size_t headerCallback(char* buffer, size_t size, size_t nitems, Headers* headers) {
        if (!headers || !buffer) return size * nitems;
        
        try {
            std::string header(buffer, size * nitems);
            auto pos = header.find(':');
            if (pos != std::string::npos) {
                std::string key = header.substr(0, pos);
                std::string value = header.substr(pos + 1);
                
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t\r\n") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t\r\n") + 1);
                
                (*headers)[key] = value;
            }
        } catch (...) {
        }
        return size * nitems;
    }
}

class CurlHandle {
private:
    void* handle_;

public:
    CurlHandle() : handle_(curl_easy_init()) {
        if (!handle_) {
            throw ConnectionError("Failed to initialize CURL");
        }
    }
    
    ~CurlHandle() {
        if (handle_) {
            curl_easy_cleanup(handle_);
        }
    }
    
    CurlHandle(const CurlHandle&) = delete;
    CurlHandle& operator=(const CurlHandle&) = delete;
    
    CurlHandle(CurlHandle&& other) noexcept : handle_(other.handle_) {
        other.handle_ = nullptr;
    }
    
    CurlHandle& operator=(CurlHandle&& other) noexcept {
        if (this != &other) {
            if (handle_) {
                curl_easy_cleanup(handle_);
            }
            handle_ = other.handle_;
            other.handle_ = nullptr;
        }
        return *this;
    }
    
    void* get() const noexcept { return handle_; }
    explicit operator bool() const noexcept { return handle_ != nullptr; }
    void reset() {
        if (handle_) {
            curl_easy_reset(handle_);
        }
    }
};

class CurlSlist {
private:
    struct curl_slist* list_;

public:
    CurlSlist() : list_(nullptr) {}
    
    ~CurlSlist() {
        if (list_) {
            curl_slist_free_all(list_);
        }
    }
    
    CurlSlist(const CurlSlist&) = delete;
    CurlSlist& operator=(const CurlSlist&) = delete;
    
    CurlSlist(CurlSlist&& other) noexcept : list_(other.list_) {
        other.list_ = nullptr;
    }
    
    CurlSlist& operator=(CurlSlist&& other) noexcept {
        if (this != &other) {
            if (list_) {
                curl_slist_free_all(list_);
            }
            list_ = other.list_;
            other.list_ = nullptr;
        }
        return *this;
    }
    
    void append(const std::string& header) {
        list_ = curl_slist_append(list_, header.c_str());
        if (!list_) {
            throw ConnectionError("Failed to append CURL header");
        }
    }
    
    struct curl_slist* get() const noexcept { return list_; }
};

class HttpClient::Impl {
private:
    Config config_;
    CurlHandle curlHandle_;
    
public:
    explicit Impl(const Config& config) : config_(config) {}
    
    std::string buildUrl(const std::string& endpoint) const {
        std::string url = config_.baseUrl();
        if (!endpoint.empty() && endpoint[0] != '/') {
            url += '/';
        }
        url += endpoint;
        return url;
    }
    
    Headers buildHeaders(const Headers& additional = {}) const {
        Headers headers = additional;
        
        if (!config_.accessToken.empty()) {
            headers["Authorization"] = "Bearer " + config_.accessToken;
        }
        
        if (config_.sandboxMode && !config_.accountNumber.empty()) {
            headers["Tradier-Account"] = config_.accountNumber;
        }
    
        if (headers.find("Accept") == headers.end()) {
            headers["Accept"] = "application/json";
        }
        
        return headers;
    }
    
    std::string urlEncode(const std::string& value) const {
        CurlHandle encoder;
        if (!encoder) {
            throw ConnectionError("Failed to initialize CURL for URL encoding");
        }
        
        std::unique_ptr<char, decltype(&curl_free)> encoded(
            curl_easy_escape(encoder.get(), value.c_str(), static_cast<int>(value.length())),
            &curl_free
        );
        
        if (!encoded) {
            throw ConnectionError("Failed to URL encode value");
        }
        
        return std::string(encoded.get());
    }
    
    Response performRequest(const std::string& method, const std::string& endpoint, 
                          const std::map<std::string, std::string>& params) {
        if (!curlHandle_) {
            throw ConnectionError("CURL handle not initialized");
        }
        
        curlHandle_.reset();
        
        std::string url = buildUrl(endpoint);
        std::string postData;
        
        if (method == "GET" || method == "DELETE") {
            if (!params.empty()) {
                url += "?";
                bool first = true;
                for (const auto& [key, value] : params) {
                    if (!first) url += "&";
                    url += utils::urlEncode(key) + "=" + utils::urlEncode(value);
                    first = false;
                }
            }
        } else {
            if (!params.empty()) {
                bool first = true;
                for (const auto& [key, value] : params) {
                    if (!first) postData += "&";
                    postData += urlEncode(key) + "=" + urlEncode(value);
                    first = false;
                }
            }
        }
        
        std::string responseBody;
        Headers responseHeaders;
        
        curl_easy_setopt(curlHandle_.get(), CURLOPT_URL, url.c_str());
        curl_easy_setopt(curlHandle_.get(), CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curlHandle_.get(), CURLOPT_WRITEDATA, &responseBody);
        curl_easy_setopt(curlHandle_.get(), CURLOPT_HEADERFUNCTION, headerCallback);
        curl_easy_setopt(curlHandle_.get(), CURLOPT_HEADERDATA, &responseHeaders);
        curl_easy_setopt(curlHandle_.get(), CURLOPT_TIMEOUT, config_.timeoutSeconds);
        
        if (method == "POST") {
            curl_easy_setopt(curlHandle_.get(), CURLOPT_POST, 1L);
            curl_easy_setopt(curlHandle_.get(), CURLOPT_POSTFIELDS, postData.c_str());
        } else if (method == "PUT") {
            curl_easy_setopt(curlHandle_.get(), CURLOPT_CUSTOMREQUEST, "PUT");
            curl_easy_setopt(curlHandle_.get(), CURLOPT_POSTFIELDS, postData.c_str());
        } else if (method == "DELETE") {
            curl_easy_setopt(curlHandle_.get(), CURLOPT_CUSTOMREQUEST, "DELETE");
        } else {
            curl_easy_setopt(curlHandle_.get(), CURLOPT_HTTPGET, 1L);
        }
        
        CurlSlist headerList;
        auto headers = buildHeaders(method == "POST" || method == "PUT" ? 
            Headers{{"Content-Type", "application/x-www-form-urlencoded"}} : Headers{});
        
        for (const auto& [key, value] : headers) {
            headerList.append(key + ": " + value);
        }
        curl_easy_setopt(curlHandle_.get(), CURLOPT_HTTPHEADER, headerList.get());
        
        CURLcode res = curl_easy_perform(curlHandle_.get());
        if (res != CURLE_OK) {
            throw ConnectionError(std::string("CURL error: ") + curl_easy_strerror(res));
        }
        
        long statusCode;
        curl_easy_getinfo(curlHandle_.get(), CURLINFO_RESPONSE_CODE, &statusCode);
        
        return {static_cast<int>(statusCode), std::move(responseBody), std::move(responseHeaders)};
    }
};

HttpClient::HttpClient(const Config& config) : impl_(std::make_unique<Impl>(config)) {}

HttpClient::~HttpClient() = default;

HttpClient::HttpClient(HttpClient&& other) noexcept = default;
HttpClient& HttpClient::operator=(HttpClient&& other) noexcept = default;

Response HttpClient::get(const std::string& endpoint, const QueryParams& params) {
    return impl_->performRequest("GET", endpoint, params);
}

Response HttpClient::post(const std::string& endpoint, const FormParams& params) {
    return impl_->performRequest("POST", endpoint, params);
}

Response HttpClient::put(const std::string& endpoint, const FormParams& params) {
    return impl_->performRequest("PUT", endpoint, params);
}

Response HttpClient::del(const std::string& endpoint, const QueryParams& params) {
    return impl_->performRequest("DELETE", endpoint, params);
}

}
