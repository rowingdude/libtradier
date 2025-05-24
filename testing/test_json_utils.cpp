#include "test_framework.hpp"
#include "tradier/common/json_utils.hpp"
#include "tradier/common/types.hpp"

namespace {

void test_safe_json_parser_valid() {
    std::string validJson = R"({"name": "test", "value": 42.5, "active": true, "count": 100})";
    
    tradier::json::SafeJsonParser parser(validJson);
    
    ASSERT_TRUE(parser.isValid());
    ASSERT_EQ("test", parser.value("name", std::string("")));
    ASSERT_EQ(42.5, parser.value("value", 0.0));
    ASSERT_TRUE(parser.value("active", false));
    ASSERT_EQ(100, parser.value("count", 0));
}

void test_safe_json_parser_invalid() {
    std::string invalidJson = R"({"name": "test", "value": )";
    
    tradier::json::SafeJsonParser parser(invalidJson);
    
    ASSERT_FALSE(parser.isValid());
    ASSERT_FALSE(parser.error().empty());
}

void test_safe_json_parser_defaults() {
    std::string json = R"({"existing": "value"})";
    
    tradier::json::SafeJsonParser parser(json);
    
    ASSERT_TRUE(parser.isValid());
    ASSERT_EQ("value", parser.value("existing", std::string("default")));
    ASSERT_EQ("default", parser.value("missing", std::string("default")));
    ASSERT_EQ(42, parser.value("missing", 42));
    ASSERT_TRUE(parser.value("missing", true));
}

void test_safe_json_parser_null_values() {
    std::string json = R"({"nullValue": null, "emptyString": ""})";
    
    tradier::json::SafeJsonParser parser(json);
    
    ASSERT_TRUE(parser.isValid());
    ASSERT_EQ("default", parser.value("nullValue", std::string("default")));
    ASSERT_EQ("", parser.value("emptyString", std::string("default")));
}

void test_safe_json_parser_nested() {
    std::string json = R"({"user": {"name": "John", "settings": {"theme": "dark"}}})";
    
    tradier::json::SafeJsonParser parser(json);
    
    ASSERT_TRUE(parser.isValid());
    ASSERT_TRUE(parser.contains("user"));
    
    auto userParser = parser["user"];
    ASSERT_TRUE(userParser.isValid());
    ASSERT_EQ("John", userParser.value("name", std::string("")));
    
    auto settingsParser = userParser["settings"];
    ASSERT_TRUE(settingsParser.isValid());
    ASSERT_EQ("dark", settingsParser.value("theme", std::string("")));
}

void test_safe_json_parser_response() {
    tradier::Response response;
    response.status = 200;
    response.body = R"({"message": "success", "data": 123})";
    
    tradier::json::SafeJsonParser parser(response);
    
    ASSERT_TRUE(parser.isValid());
    ASSERT_EQ("success", parser.value("message", std::string("")));
    ASSERT_EQ(123, parser.value("data", 0));
}

void test_safe_json_parser_bad_response() {
    tradier::Response response;
    response.status = 500;
    response.body = "Internal Server Error";
    
    tradier::json::SafeJsonParser parser(response);
    
    ASSERT_FALSE(parser.isValid());
    ASSERT_FALSE(parser.error().empty());
}

void test_json_validator() {
    tradier::json::JsonValidator validator;
    nlohmann::json validJson = nlohmann::json::parse(R"({
        "user": {"name": "test"},
        "items": [1, 2, 3],
        "count": 42
    })");
    
    ASSERT_TRUE(validator.validateField<std::string>(validJson, "nonexistent", false));
    ASSERT_FALSE(validator.validateField<std::string>(validJson, "nonexistent", true));
    ASSERT_TRUE(validator.validateField<int>(validJson, "count"));
    ASSERT_TRUE(validator.validateObject(validJson, "user"));
    ASSERT_TRUE(validator.validateArray(validJson, "items"));
    
    ASSERT_FALSE(validator.validateObject(validJson, "count"));
    ASSERT_TRUE(validator.hasErrors());
    ASSERT_FALSE(validator.getErrorString().empty());
}

void test_datetime_parsing() {
    nlohmann::json json = nlohmann::json::parse(R"({
        "timestamp": "2024-01-15T14:30:45",
        "date": "2024-01-15"
    })");
    
    auto timestamp = tradier::json::parseDateTime(json, "timestamp");
    ASSERT_TRUE(timestamp != tradier::TimePoint{});
    
    auto formatted = tradier::json::formatDateTime(timestamp);
    ASSERT_FALSE(formatted.empty());
}

void test_parse_response_helper() {
    tradier::Response goodResponse;
    goodResponse.status = 200;
    goodResponse.body = R"({"value": 42})";
    
    auto result = tradier::json::parseResponse<int>(goodResponse, [](const nlohmann::json& json) {
        return json.value("value", 0);
    });
    
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(42, result.value());
    
    tradier::Response badResponse;
    badResponse.status = 500;
    badResponse.body = "Error";
    
    auto badResult = tradier::json::parseResponse<int>(badResponse, [](const nlohmann::json& json) {
        return json.value("value", 0);
    });
    
    ASSERT_FALSE(badResult.has_value());
}

void test_parse_response_safe_helper() {
    tradier::Response response;
    response.status = 200;
    response.body = R"({"value": 42})";
    
    auto result = tradier::json::parseResponseSafe<int>(response, [](const tradier::json::SafeJsonParser& parser) {
        return parser.value("value", 0);
    });
    
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(42, result.value());
}

}

int test_json_utils_main() {
    test::TestSuite suite("JSON Utils Tests");
    
    suite.add_test("SafeJsonParser Valid JSON", test_safe_json_parser_valid);
    suite.add_test("SafeJsonParser Invalid JSON", test_safe_json_parser_invalid);
    suite.add_test("SafeJsonParser Default Values", test_safe_json_parser_defaults);
    suite.add_test("SafeJsonParser Null Values", test_safe_json_parser_null_values);
    suite.add_test("SafeJsonParser Nested Access", test_safe_json_parser_nested);
    suite.add_test("SafeJsonParser Response Constructor", test_safe_json_parser_response);
    suite.add_test("SafeJsonParser Bad Response", test_safe_json_parser_bad_response);
    suite.add_test("JsonValidator", test_json_validator);
    suite.add_test("DateTime Parsing", test_datetime_parsing);
    suite.add_test("Parse Response Helper", test_parse_response_helper);
    suite.add_test("Parse Response Safe Helper", test_parse_response_safe_helper);
    
    return suite.run();
}