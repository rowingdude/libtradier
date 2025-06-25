#pragma once

#include <string>
#include <map>
#include <functional>
#include <memory>
#include <vector>
#include "tradier/common/types.hpp"

namespace tradier {
namespace test {

/**
 * Mock HTTP client for testing purposes
 * Allows predefined responses and request verification
 */
class MockHttpClient {
public:
    struct MockResponse {
        int status = 200;
        std::string body;
        Headers headers;
        std::chrono::milliseconds delay = std::chrono::milliseconds(0);
        bool should_fail = false;
        std::string error_message;
    };

    struct RequestRecord {
        std::string method;
        std::string endpoint;
        QueryParams query_params;
        FormParams form_params;
        std::chrono::system_clock::time_point timestamp;
    };

    MockHttpClient() = default;
    ~MockHttpClient() = default;

    // Mock HTTP client interface
    Response get(const std::string& endpoint, const QueryParams& params = {});
    Response post(const std::string& endpoint, const FormParams& params = {});
    Response put(const std::string& endpoint, const FormParams& params = {});
    Response del(const std::string& endpoint, const QueryParams& params = {});

    // Mock configuration methods
    void set_response(const std::string& method, const std::string& url_pattern, const MockResponse& response);
    void set_default_response(const MockResponse& response);
    void clear_responses();

    // Request verification methods
    std::vector<RequestRecord> get_requests() const { return requests_; }
    RequestRecord get_last_request() const;
    size_t get_request_count() const { return requests_.size(); }
    bool was_called(const std::string& method, const std::string& url_pattern) const;
    void clear_request_history();

    // Response simulation
    void simulate_network_error(bool enable = true) { simulate_network_error_ = enable; }
    void simulate_timeout(bool enable = true) { simulate_timeout_ = enable; }
    void set_global_delay(std::chrono::milliseconds delay) { global_delay_ = delay; }

private:
    struct ResponseKey {
        std::string method;
        std::string url_pattern;
        
        bool operator<(const ResponseKey& other) const {
            if (method != other.method) return method < other.method;
            return url_pattern < other.url_pattern;
        }
    };

    Response execute_request(const std::string& method, const std::string& endpoint, 
                           const FormParams& form_params, 
                           const QueryParams& query_params);
    
    MockResponse find_matching_response(const std::string& method, const std::string& endpoint) const;
    bool url_matches_pattern(const std::string& endpoint, const std::string& pattern) const;
    void record_request(const std::string& method, const std::string& endpoint, 
                       const FormParams& form_params, const QueryParams& query_params);

    std::map<ResponseKey, MockResponse> responses_;
    MockResponse default_response_;
    std::vector<RequestRecord> requests_;
    
    bool simulate_network_error_ = false;
    bool simulate_timeout_ = false;
    std::chrono::milliseconds global_delay_ = std::chrono::milliseconds(0);
};

/**
 * Helper class for setting up common API responses
 */
class ApiResponseBuilder {
public:
    static MockHttpClient::MockResponse success(const std::string& json_body);
    static MockHttpClient::MockResponse error(int status_code, const std::string& error_message);
    static MockHttpClient::MockResponse rate_limit_exceeded();
    static MockHttpClient::MockResponse unauthorized();
    static MockHttpClient::MockResponse not_found();
    static MockHttpClient::MockResponse server_error();
    static MockHttpClient::MockResponse network_timeout();

    // Account API responses
    static MockHttpClient::MockResponse account_profile();
    static MockHttpClient::MockResponse account_balances();
    static MockHttpClient::MockResponse account_positions();

    // Market API responses
    static MockHttpClient::MockResponse quotes(const std::vector<std::string>& symbols);
    static MockHttpClient::MockResponse options_chain(const std::string& symbol);
    static MockHttpClient::MockResponse market_calendar();

    // Trading API responses
    static MockHttpClient::MockResponse order_preview();
    static MockHttpClient::MockResponse order_submitted(const std::string& order_id);
    static MockHttpClient::MockResponse order_status(const std::string& order_id, const std::string& status);
};

} // namespace test
} // namespace tradier