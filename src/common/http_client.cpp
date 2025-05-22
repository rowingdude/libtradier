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

namespace tradier {

static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
    data->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

static size_t headerCallback(char* buffer, size_t size, size_t nitems, Headers* headers) {
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
    return size * nitems;
}

HttpClient::HttpClient(const Config& config) : config_(config) {
    curlHandle_ = curl_easy_init();
    if (!curlHandle_) {
        throw ConnectionError("Failed to initialize CURL");
    }
}

HttpClient::~HttpClient() {
    if (curlHandle_) {
        curl_easy_cleanup(curlHandle_);
    }
}

HttpClient::HttpClient(HttpClient&& other) noexcept 
    : config_(std::move(other.config_)), curlHandle_(other.curlHandle_) {
    other.curlHandle_ = nullptr;
}

HttpClient& HttpClient::operator=(HttpClient&& other) noexcept {
    if (this != &other) {
        if (curlHandle_) {
            curl_easy_cleanup(curlHandle_);
        }
        config_ = std::move(other.config_);
        curlHandle_ = other.curlHandle_;
        other.curlHandle_ = nullptr;
    }
    return *this;
}

std::string HttpClient::buildUrl(const std::string& endpoint) const {
    std::string url = config_.baseUrl();
    if (!endpoint.empty() && endpoint[0] != '/') {
        url += '/';
    }
    url += endpoint;
    return url;
}

Headers HttpClient::buildHeaders(const Headers& additional) const {
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

Response HttpClient::get(const std::string& endpoint, const QueryParams& params) {
    if (!curlHandle_) {
        throw ConnectionError("CURL handle not initialized");
    }
    
    curl_easy_reset(curlHandle_);
    
    std::string url = buildUrl(endpoint);
    if (!params.empty()) {
        url += "?";
        bool first = true;
        for (const auto& [key, value] : params) {
            if (!first) url += "&";
            url += utils::urlEncode(key) + "=" + utils::urlEncode(value);
            first = false;
        }
    }
    
    std::string responseBody;
    Headers responseHeaders;
    
    curl_easy_setopt(curlHandle_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curlHandle_, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curlHandle_, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curlHandle_, CURLOPT_WRITEDATA, &responseBody);
    curl_easy_setopt(curlHandle_, CURLOPT_HEADERFUNCTION, headerCallback);
    curl_easy_setopt(curlHandle_, CURLOPT_HEADERDATA, &responseHeaders);
    curl_easy_setopt(curlHandle_, CURLOPT_TIMEOUT, config_.timeoutSeconds);
    
    struct curl_slist* headerList = nullptr;
    auto headers = buildHeaders();
    for (const auto& [key, value] : headers) {
        std::string header = key + ": " + value;
        headerList = curl_slist_append(headerList, header.c_str());
    }
    curl_easy_setopt(curlHandle_, CURLOPT_HTTPHEADER, headerList);
    
    CURLcode res = curl_easy_perform(curlHandle_);
    curl_slist_free_all(headerList);
    
    if (res != CURLE_OK) {
        throw ConnectionError(std::string("CURL error: ") + curl_easy_strerror(res));
    }
    
    long statusCode;
    curl_easy_getinfo(curlHandle_, CURLINFO_RESPONSE_CODE, &statusCode);
    
    return {static_cast<int>(statusCode), responseBody, responseHeaders};
}

Response HttpClient::post(const std::string& endpoint, const FormParams& params) {
    if (!curlHandle_) {
        throw ConnectionError("CURL handle not initialized");
    }
    
    curl_easy_reset(curlHandle_);
    
    std::string url = buildUrl(endpoint);
    std::string postData;
    
    if (!params.empty()) {
        bool first = true;
        for (const auto& [key, value] : params) {
            if (!first) postData += "&";
            postData += utils::urlEncode(key) + "=" + utils::urlEncode(value);
            first = false;
        }
    }
    
    std::string responseBody;
    Headers responseHeaders;
    
    curl_easy_setopt(curlHandle_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curlHandle_, CURLOPT_POST, 1L);
    curl_easy_setopt(curlHandle_, CURLOPT_POSTFIELDS, postData.c_str());
    curl_easy_setopt(curlHandle_, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curlHandle_, CURLOPT_WRITEDATA, &responseBody);
    curl_easy_setopt(curlHandle_, CURLOPT_HEADERFUNCTION, headerCallback);
    curl_easy_setopt(curlHandle_, CURLOPT_HEADERDATA, &responseHeaders);
    curl_easy_setopt(curlHandle_, CURLOPT_TIMEOUT, config_.timeoutSeconds);
    
    struct curl_slist* headerList = nullptr;
    auto headers = buildHeaders({{"Content-Type", "application/x-www-form-urlencoded"}});
    for (const auto& [key, value] : headers) {
        std::string header = key + ": " + value;
        headerList = curl_slist_append(headerList, header.c_str());
    }
    curl_easy_setopt(curlHandle_, CURLOPT_HTTPHEADER, headerList);
    
    CURLcode res = curl_easy_perform(curlHandle_);
    curl_slist_free_all(headerList);
    
    if (res != CURLE_OK) {
        throw ConnectionError(std::string("CURL error: ") + curl_easy_strerror(res));
    }
    
    long statusCode;
    curl_easy_getinfo(curlHandle_, CURLINFO_RESPONSE_CODE, &statusCode);
    
    return {static_cast<int>(statusCode), responseBody, responseHeaders};
}

Response HttpClient::put(const std::string& endpoint, const FormParams& params) {
    if (!curlHandle_) {
        throw ConnectionError("CURL handle not initialized");
    }
    
    curl_easy_reset(curlHandle_);
    
    std::string url = buildUrl(endpoint);
    std::string putData;
    
    if (!params.empty()) {
        bool first = true;
        for (const auto& [key, value] : params) {
            if (!first) putData += "&";
            putData += utils::urlEncode(key) + "=" + utils::urlEncode(value);
            first = false;
        }
    }
    
    std::string responseBody;
    Headers responseHeaders;
    
    curl_easy_setopt(curlHandle_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curlHandle_, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curlHandle_, CURLOPT_POSTFIELDS, putData.c_str());
    curl_easy_setopt(curlHandle_, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curlHandle_, CURLOPT_WRITEDATA, &responseBody);
    curl_easy_setopt(curlHandle_, CURLOPT_HEADERFUNCTION, headerCallback);
    curl_easy_setopt(curlHandle_, CURLOPT_HEADERDATA, &responseHeaders);
    curl_easy_setopt(curlHandle_, CURLOPT_TIMEOUT, config_.timeoutSeconds);
    
    struct curl_slist* headerList = nullptr;
    auto headers = buildHeaders({{"Content-Type", "application/x-www-form-urlencoded"}});
    for (const auto& [key, value] : headers) {
        std::string header = key + ": " + value;
        headerList = curl_slist_append(headerList, header.c_str());
    }
    curl_easy_setopt(curlHandle_, CURLOPT_HTTPHEADER, headerList);
    
    CURLcode res = curl_easy_perform(curlHandle_);
    curl_slist_free_all(headerList);
    
    if (res != CURLE_OK) {
        throw ConnectionError(std::string("CURL error: ") + curl_easy_strerror(res));
    }
    
    long statusCode;
    curl_easy_getinfo(curlHandle_, CURLINFO_RESPONSE_CODE, &statusCode);
    
    return {static_cast<int>(statusCode), responseBody, responseHeaders};
}

Response HttpClient::del(const std::string& endpoint, const QueryParams& params) {
    if (!curlHandle_) {
        throw ConnectionError("CURL handle not initialized");
    }
    
    curl_easy_reset(curlHandle_);
    
    std::string url = buildUrl(endpoint);
    if (!params.empty()) {
        url += "?";
        bool first = true;
        for (const auto& [key, value] : params) {
            if (!first) url += "&";
            url += utils::urlEncode(key) + "=" + utils::urlEncode(value);
            first = false;
        }
    }
    
    std::string responseBody;
    Headers responseHeaders;
    
    curl_easy_setopt(curlHandle_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curlHandle_, CURLOPT_CUSTOMREQUEST, "DELETE");
    curl_easy_setopt(curlHandle_, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curlHandle_, CURLOPT_WRITEDATA, &responseBody);
    curl_easy_setopt(curlHandle_, CURLOPT_HEADERFUNCTION, headerCallback);
    curl_easy_setopt(curlHandle_, CURLOPT_HEADERDATA, &responseHeaders);
    curl_easy_setopt(curlHandle_, CURLOPT_TIMEOUT, config_.timeoutSeconds);
    
    struct curl_slist* headerList = nullptr;
    auto headers = buildHeaders();
    for (const auto& [key, value] : headers) {
        std::string header = key + ": " + value;
        headerList = curl_slist_append(headerList, header.c_str());
    }
    curl_easy_setopt(curlHandle_, CURLOPT_HTTPHEADER, headerList);
    
    CURLcode res = curl_easy_perform(curlHandle_);
    curl_slist_free_all(headerList);
    
    if (res != CURLE_OK) {
        throw ConnectionError(std::string("CURL error: ") + curl_easy_strerror(res));
    }
    
    long statusCode;
    curl_easy_getinfo(curlHandle_, CURLINFO_RESPONSE_CODE, &statusCode);
    
    return {static_cast<int>(statusCode), responseBody, responseHeaders};
}

}