#include "mock_http_client.h"
#include <regex>
#include <thread>
#include <stdexcept>

namespace tradier {
namespace test {

Response MockHttpClient::get(const std::string& endpoint, const QueryParams& params) {
    return execute_request("GET", endpoint, {}, params);
}

Response MockHttpClient::post(const std::string& endpoint, const FormParams& params) {
    return execute_request("POST", endpoint, params, {});
}

Response MockHttpClient::put(const std::string& endpoint, const FormParams& params) {
    return execute_request("PUT", endpoint, params, {});
}

Response MockHttpClient::del(const std::string& endpoint, const QueryParams& params) {
    return execute_request("DELETE", endpoint, {}, params);
}

void MockHttpClient::set_response(const std::string& method, const std::string& url_pattern, const MockResponse& response) {
    ResponseKey key{method, url_pattern};
    responses_[key] = response;
}

void MockHttpClient::set_default_response(const MockResponse& response) {
    default_response_ = response;
}

void MockHttpClient::clear_responses() {
    responses_.clear();
    default_response_ = MockResponse{};
}

MockHttpClient::RequestRecord MockHttpClient::get_last_request() const {
    if (requests_.empty()) {
        throw std::runtime_error("No requests recorded");
    }
    return requests_.back();
}

bool MockHttpClient::was_called(const std::string& method, const std::string& url_pattern) const {
    for (const auto& request : requests_) {
        if (request.method == method && url_matches_pattern(request.endpoint, url_pattern)) {
            return true;
        }
    }
    return false;
}

void MockHttpClient::clear_request_history() {
    requests_.clear();
}

Response MockHttpClient::execute_request(const std::string& method, const std::string& endpoint, 
                                        const FormParams& form_params, 
                                        const QueryParams& query_params) {
    // Record the request
    record_request(method, endpoint, form_params, query_params);

    // Simulate network conditions
    if (simulate_network_error_) {
        throw std::runtime_error("Simulated network error");
    }

    if (simulate_timeout_) {
        throw std::runtime_error("Simulated timeout");
    }

    // Apply global delay
    if (global_delay_.count() > 0) {
        std::this_thread::sleep_for(global_delay_);
    }

    // Find matching response
    MockResponse mock_response = find_matching_response(method, endpoint);

    // Apply response-specific delay
    if (mock_response.delay.count() > 0) {
        std::this_thread::sleep_for(mock_response.delay);
    }

    // Check if response should fail
    if (mock_response.should_fail) {
        throw std::runtime_error(mock_response.error_message.empty() ? "Simulated request failure" : mock_response.error_message);
    }

    // Create HTTP response
    Response response;
    response.status = mock_response.status;
    response.body = mock_response.body;
    response.headers = mock_response.headers;

    return response;
}

MockHttpClient::MockResponse MockHttpClient::find_matching_response(const std::string& method, const std::string& url) const {
    // Look for exact matches first
    for (const auto& [key, response] : responses_) {
        if (key.method == method && url_matches_pattern(url, key.url_pattern)) {
            return response;
        }
    }

    // Return default response if no match found
    return default_response_;
}

bool MockHttpClient::url_matches_pattern(const std::string& url, const std::string& pattern) const {
    // Simple pattern matching - supports wildcards with *
    std::string regex_pattern = pattern;
    
    // Escape regex special characters except *
    std::regex special_chars{R"([-[\]{}()+?.,\^$|#\s])"};
    regex_pattern = std::regex_replace(regex_pattern, special_chars, R"(\$&)");
    
    // Convert * to .*
    std::regex wildcard{R"(\\\*)"};
    regex_pattern = std::regex_replace(regex_pattern, wildcard, ".*");
    
    // Match the pattern
    std::regex pattern_regex(regex_pattern);
    return std::regex_match(url, pattern_regex);
}

void MockHttpClient::record_request(const std::string& method, const std::string& endpoint, 
                                   const FormParams& form_params, const QueryParams& query_params) {
    RequestRecord record;
    record.method = method;
    record.endpoint = endpoint;
    record.form_params = form_params;
    record.query_params = query_params;
    record.timestamp = std::chrono::system_clock::now();
    
    requests_.push_back(record);
}

// ApiResponseBuilder implementations
MockHttpClient::MockResponse ApiResponseBuilder::success(const std::string& json_body) {
    MockResponse response;
    response.status = 200;
    response.body = json_body;
    response.headers["content-type"] = "application/json";
    return response;
}

MockHttpClient::MockResponse ApiResponseBuilder::error(int status_code, const std::string& error_message) {
    MockResponse response;
    response.status = status_code;
    response.body = R"({"error": ")" + error_message + R"("})";
    response.headers["content-type"] = "application/json";
    return response;
}

MockHttpClient::MockResponse ApiResponseBuilder::rate_limit_exceeded() {
    return error(429, "Rate limit exceeded");
}

MockHttpClient::MockResponse ApiResponseBuilder::unauthorized() {
    return error(401, "Unauthorized");
}

MockHttpClient::MockResponse ApiResponseBuilder::not_found() {
    return error(404, "Not found");
}

MockHttpClient::MockResponse ApiResponseBuilder::server_error() {
    return error(500, "Internal server error");
}

MockHttpClient::MockResponse ApiResponseBuilder::network_timeout() {
    MockResponse response;
    response.should_fail = true;
    response.error_message = "Request timeout";
    return response;
}

MockHttpClient::MockResponse ApiResponseBuilder::account_profile() {
    return success(R"({
        "profile": {
            "account_number": "123456789",
            "classification": "individual",
            "date_created": "2020-01-01T00:00:00.000Z",
            "day_trader": false,
            "option_level": 2,
            "status": "active",
            "type": "margin",
            "last_update_date": "2023-01-01T00:00:00.000Z"
        }
    })");
}

MockHttpClient::MockResponse ApiResponseBuilder::account_balances() {
    return success(R"({
        "balances": {
            "option_short_value": 0,
            "total_equity": 50000.00,
            "account_number": "123456789",
            "account_type": "margin",
            "close_pl": 0,
            "current_requirement": 0,
            "equity": 50000.00,
            "long_market_value": 45000.00,
            "market_value": 45000.00,
            "open_pl": 1000.00,
            "option_long_value": 0,
            "option_requirement": 0,
            "pending_orders_count": 0,
            "short_market_value": 0,
            "stock_long_value": 45000.00,
            "total_cash": 5000.00,
            "uncleared_funds": 0,
            "unsettled_funds": 0,
            "day_trade_buying_power": 100000.00,
            "option_buying_power": 25000.00,
            "stock_buying_power": 25000.00
        }
    })");
}

MockHttpClient::MockResponse ApiResponseBuilder::account_positions() {
    return success(R"({
        "positions": {
            "position": [
                {
                    "cost_basis": 10000.00,
                    "date_acquired": "2023-01-01T00:00:00.000Z",
                    "id": 1,
                    "quantity": 100,
                    "symbol": "AAPL"
                },
                {
                    "cost_basis": 5000.00,
                    "date_acquired": "2023-01-15T00:00:00.000Z",
                    "id": 2,
                    "quantity": 50,
                    "symbol": "GOOGL"
                }
            ]
        }
    })");
}

MockHttpClient::MockResponse ApiResponseBuilder::quotes(const std::vector<std::string>& symbols) {
    std::string quotes_json = R"({"quotes": {"quote": [)";
    
    for (size_t i = 0; i < symbols.size(); ++i) {
        if (i > 0) quotes_json += ",";
        quotes_json += R"({
            "symbol": ")" + symbols[i] + R"(",
            "description": "Test Company",
            "exch": "NASDAQ",
            "type": "stock",
            "last": 150.00,
            "change": 2.50,
            "volume": 1000000,
            "open": 148.00,
            "high": 152.00,
            "low": 147.50,
            "close": 147.50,
            "bid": 149.90,
            "ask": 150.10,
            "change_percentage": 1.69,
            "average_volume": 950000,
            "last_volume": 100,
            "trade_date": 1640995200000,
            "prevclose": 147.50,
            "week_52_high": 180.00,
            "week_52_low": 120.00,
            "bidsize": 10,
            "bidexch": "NASDAQ",
            "bid_date": 1640995200000,
            "asksize": 12,
            "askexch": "NASDAQ",
            "ask_date": 1640995200000,
            "root_symbols": ")" + symbols[i] + R"("
        })";
    }
    
    quotes_json += "]}}";
    return success(quotes_json);
}

MockHttpClient::MockResponse ApiResponseBuilder::options_chain(const std::string& symbol) {
    return success(R"({
        "options": {
            "option": [
                {
                    "symbol": ")" + symbol + R"(230120C00150000",
                    "description": ")" + symbol + R"( Jan 20 2023 $150.00 Call",
                    "exch": "OPRA",
                    "type": "option",
                    "last": 5.50,
                    "change": 0.25,
                    "volume": 500,
                    "open": 5.25,
                    "high": 5.75,
                    "low": 5.20,
                    "close": 5.25,
                    "bid": 5.45,
                    "ask": 5.55,
                    "underlying": ")" + symbol + R"(",
                    "strike": 150.00,
                    "option_type": "call",
                    "expiration_date": "2023-01-20",
                    "expiration_type": "standard",
                    "contract_size": 100,
                    "greeks": {
                        "delta": 0.55,
                        "gamma": 0.025,
                        "theta": -0.045,
                        "vega": 0.18,
                        "rho": 0.12,
                        "phi": -0.025,
                        "bid_iv": 0.28,
                        "mid_iv": 0.29,
                        "ask_iv": 0.30,
                        "smv_vol": 0.29,
                        "updated_at": "2023-01-01T16:00:00.000Z"
                    }
                }
            ]
        }
    })");
}

MockHttpClient::MockResponse ApiResponseBuilder::market_calendar() {
    return success(R"({
        "calendar": {
            "days": {
                "day": [
                    {
                        "date": "2023-01-01",
                        "status": "closed",
                        "description": "New Year's Day",
                        "premarket": {
                            "start": "00:00",
                            "end": "00:00"
                        },
                        "open": {
                            "start": "00:00",
                            "end": "00:00"
                        },
                        "postmarket": {
                            "start": "00:00",
                            "end": "00:00"
                        }
                    },
                    {
                        "date": "2023-01-02",
                        "status": "open",
                        "description": "Regular Trading Day",
                        "premarket": {
                            "start": "04:00",
                            "end": "09:30"
                        },
                        "open": {
                            "start": "09:30",
                            "end": "16:00"
                        },
                        "postmarket": {
                            "start": "16:00",
                            "end": "20:00"
                        }
                    }
                ]
            }
        }
    })");
}

MockHttpClient::MockResponse ApiResponseBuilder::order_preview() {
    return success(R"({
        "order": {
            "status": "ok",
            "commission": 0.00,
            "cost": 15000.00,
            "fees": 0.00,
            "symbol": "AAPL",
            "quantity": 100,
            "side": "buy",
            "type": "market",
            "duration": "day",
            "result": true,
            "order_cost": 15000.00,
            "margin_change": 7500.00,
            "request_date": "2023-01-01T12:00:00.000Z",
            "extended_hours": false,
            "class": "equity",
            "strategy": "equity"
        }
    })");
}

MockHttpClient::MockResponse ApiResponseBuilder::order_submitted(const std::string& order_id) {
    return success(R"({
        "order": {
            "id": )" + order_id + R"(,
            "status": "submitted",
            "partner_id": "test-partner"
        }
    })");
}

MockHttpClient::MockResponse ApiResponseBuilder::order_status(const std::string& order_id, const std::string& status) {
    return success(R"({
        "order": {
            "id": )" + order_id + R"(,
            "type": "market",
            "symbol": "AAPL",
            "side": "buy",
            "quantity": 100.00000000,
            "status": ")" + status + R"(",
            "duration": "day",
            "price": 0.00000000,
            "avg_fill_price": 150.00000000,
            "exec_quantity": 100.00000000,
            "last_fill_price": 150.00000000,
            "last_fill_quantity": 100.00000000,
            "remaining_quantity": 0.00000000,
            "create_date": "2023-01-01T12:00:00.000Z",
            "transaction_date": "2023-01-01T12:00:05.000Z",
            "class": "equity"
        }
    })");
}

} // namespace test
} // namespace tradier