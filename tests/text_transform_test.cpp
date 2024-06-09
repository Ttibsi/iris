#include "text_transform.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("strip_trailing_whitespace", "[TEXT_TRANSFORM]") {
    SECTION("No trailing whitespace") {
        std::string s = "hello world";
        REQUIRE(strip_trailing_whitespace(s) == s);
    }

    SECTION("Trailing whitespace") {
        std::string s = "hello world   ";
        REQUIRE(strip_trailing_whitespace(s) == "hello world");
    }
}

TEST_CASE("strip_newline", "[TEXT_TRANSFORM]") {
    SECTION("No newline") {
        std::string s = "hello world";
        REQUIRE(strip_newline(s) == s);
    }

    SECTION("Line contains newline at end") {
        std::string s = "hello world\n";
        REQUIRE(strip_newline(s) == "hello world");
    }

    SECTION("Line contains newline in middle") {
        std::string s = "hello\nworld";
        REQUIRE(strip_newline(s) == "hello\nworld");
    }
}
