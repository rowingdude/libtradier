#include "test_framework.hpp"
#include "tradier/common/utils.hpp"
#include <chrono>

namespace {

void test_url_encoding() {
    ASSERT_EQ("hello%20world", tradier::utils::urlEncode("hello world"));
    ASSERT_EQ("test%40example.com", tradier::utils::urlEncode("test@example.com"));
    ASSERT_EQ("simple", tradier::utils::urlEncode("simple"));
    ASSERT_EQ("", tradier::utils::urlEncode(""));
    ASSERT_EQ("a%2Bb%3Dc%26d%3De", tradier::utils::urlEncode("a+b=c&d=e"));
}

void test_datetime_parsing() {
    std::string isoDateTime = "2024-01-15T14:30:45Z";
    auto parsed = tradier::utils::parseISODateTime(isoDateTime);
    
    ASSERT_TRUE(parsed != std::chrono::system_clock::time_point{});
    
    auto formatted = tradier::utils::formatISODateTime(parsed);
    ASSERT_FALSE(formatted.empty());
    ASSERT_TRUE(formatted.find("2024-01-15") != std::string::npos);
    ASSERT_TRUE(formatted.find("14:30:45") != std::string::npos);
    
    std::string badDateTime = "not-a-date";
    auto badParsed = tradier::utils::parseISODateTime(badDateTime);
    ASSERT_TRUE(badParsed == std::chrono::system_clock::time_point{});
}

void test_date_formatting() {
    auto now = std::chrono::system_clock::now();
    auto formatted = tradier::utils::formatDate(now);
    
    ASSERT_FALSE(formatted.empty());
    ASSERT_TRUE(formatted.find("-") != std::string::npos);
    ASSERT_EQ(10, formatted.length()); // YYYY-MM-DD format
}

void test_to_string_conversions() {
    ASSERT_EQ("42", tradier::utils::toString(42));
    ASSERT_EQ("3.14", tradier::utils::toString(3.14));
    ASSERT_EQ("hello", tradier::utils::toString(std::string("hello")));
    ASSERT_EQ("123", tradier::utils::toString(123L));
    ASSERT_EQ("456", tradier::utils::toString(456.0f));
}

void test_iso_datetime_edge_cases() {
    std::string withMillis = "2024-01-15T14:30:45.123Z";
    auto parsed1 = tradier::utils::parseISODateTime(withMillis);
    ASSERT_TRUE(parsed1 != std::chrono::system_clock::time_point{});
    
    std::string withTimezone = "2024-01-15T14:30:45+05:00";
    auto parsed2 = tradier::utils::parseISODateTime(withTimezone);
    ASSERT_TRUE(parsed2 != std::chrono::system_clock::time_point{});
    
    std::string simpleFormat = "2024-01-15T14:30:45";
    auto parsed3 = tradier::utils::parseISODateTime(simpleFormat);
    ASSERT_TRUE(parsed3 != std::chrono::system_clock::time_point{});
    
    std::string emptyString = "";
    auto parsed4 = tradier::utils::parseISODateTime(emptyString);
    ASSERT_TRUE(parsed4 == std::chrono::system_clock::time_point{});
}

void test_roundtrip_datetime() {
    auto original = std::chrono::system_clock::now();
    auto formatted = tradier::utils::formatISODateTime(original);
    auto parsed = tradier::utils::parseISODateTime(formatted);
    
    auto originalSeconds = std::chrono::duration_cast<std::chrono::seconds>(original.time_since_epoch()).count();
    auto parsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(parsed.time_since_epoch()).count();
    
    ASSERT_TRUE(std::abs(originalSeconds - parsedSeconds) <= 1);
}

}

int test_utils_main() {
    test::TestSuite suite("Utilities Tests");
    
    suite.add_test("URL Encoding", test_url_encoding);
    suite.add_test("DateTime Parsing", test_datetime_parsing);
    suite.add_test("Date Formatting", test_date_formatting);
    suite.add_test("ToString Conversions", test_to_string_conversions);
    suite.add_test("ISO DateTime Edge Cases", test_iso_datetime_edge_cases);
    suite.add_test("Roundtrip DateTime", test_roundtrip_datetime);
    
    return suite.run();
}