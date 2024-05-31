#include "logger.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "filesystem.h"

TEST_CASE("level_str", "[LOGGER]") {
    REQUIRE(level_str(Level::INFO) == "[INFO]");
    REQUIRE(level_str(Level::WARNING) == "[WARNING]");
}

TEST_CASE("log", "[LOGGER]") {
    SECTION("no specified level") {
        log("hello world");

        auto file_contents = open_file(log_file);
        REQUIRE(file_contents.has_value() == true);

        std::string contents_str =
            std::string(file_contents.value().begin(), file_contents.value().end());

        std::string expected = "[INFO] hello world";
        REQUIRE_THAT(contents_str, Catch::Matchers::ContainsSubstring(expected));
    }

    SECTION("Given level") {
        log(Level::WARNING, "hello world");

        auto file_contents = open_file(log_file);
        REQUIRE(file_contents.has_value() == true);

        std::string contents_str =
            std::string(file_contents.value().begin(), file_contents.value().end());

        std::string expected = "[WARNING] hello world";
        REQUIRE(contents_str.find(expected) != std::string::npos);
    }
}
