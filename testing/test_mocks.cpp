#pragma once

#include "tradier/common/types.hpp"
#include "tradier/common/http_client.hpp"
#include <string>
#include <map>

namespace test {

class MockHttpClient {
private:
    std::map<std::string, tradier::Response> responses_;
    
public:
    void setResponse(const std::string& endpoint, const tradier::Response& response) {
        responses_[endpoint] = response;
    }
    
    tradier::Response get(const std::string& endpoint, const tradier::QueryParams& params = {}) {
        auto it = responses_.find(endpoint);
        if (it != responses_.end()) {
            return it->second;
        }
        
        tradier::Response defaultResponse;
        defaultResponse.status = 404;
        defaultResponse.body = "Not Found";
        return defaultResponse;
    }
    
    tradier::Response post(const std::string& endpoint, const tradier::FormParams& params = {}) {
        return get(endpoint);
    }
    
    tradier::Response put(const std::string& endpoint, const tradier::FormParams& params = {}) {
        return get(endpoint);
    }
    
    tradier::Response del(const std::string& endpoint, const tradier::QueryParams& params = {}) {
        return get(endpoint);
    }
};

class MockTradierClient {
private:
    MockHttpClient httpClient_;
    tradier::Config config_;
    
public:
    explicit MockTradierClient(const tradier::Config& config) : config_(config) {}
    
    MockHttpClient& mockHttp() { return httpClient_; }
    
    const tradier::Config& config() const { return config_; }
    
    tradier::Response get(const std::string& endpoint, const tradier::QueryParams& params = {}) {
        return httpClient_.get(endpoint, params);
    }
    
    tradier::Response post(const std::string& endpoint, const tradier::FormParams& params = {}) {
        return httpClient_.post(endpoint, params);
    }
    
    tradier::Response put(const std::string& endpoint, const tradier::FormParams& params = {}) {
        return httpClient_.put(endpoint, params);
    }
    
    tradier::Response del(const std::string& endpoint, const tradier::QueryParams& params = {}) {
        return httpClient_.del(endpoint, params);
    }
};

tradier::Response createSuccessResponse(const std::string& body) {
    tradier::Response response;
    response.status = 200;
    response.body = body;
    response.headers["Content-Type"] = "application/json";
    return response;
}

tradier::Response createErrorResponse(int status, const std::string& body) {
    tradier::Response response;
    response.status = status;
    response.body = body;
    return response;
}

}