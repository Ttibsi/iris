#include "logger.h"

#include <cstdlib>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "file_io.h"

TEST_CASE("level_str", "[LOGGER]") {
    REQUIRE(level_str(Level::INFO) == "[INFO]");
    REQUIRE(level_str(Level::WARNING) == "[WARNING]");
}

TEST_CASE("formatted_time", "[LOGGER]") {
    const std::string given_time = formatted_time(system_clock::now());

    REQUIRE(given_time.size() == 19);

    REQUIRE(given_time.at(0) == '[');
    REQUIRE(given_time.at(18) == ']');
    REQUIRE_THAT(std::stof(given_time.substr(1, 2)), Catch::Matchers::WithinAbs(0.0, 31.0));
    REQUIRE_THAT(std::stof(given_time.substr(3, 2)), Catch::Matchers::WithinAbs(0.0, 12.0));
    REQUIRE_THAT(std::stof(given_time.substr(10, 2)), Catch::Matchers::WithinAbs(0.0, 23.0));
    REQUIRE_THAT(std::stof(given_time.substr(10, 2)), Catch::Matchers::WithinAbs(0.0, 59.0));
}

TEST_CASE("log", "[LOGGER]") {
    unsetenv("RAWTERM_DEBUG");

    SECTION("no specified level") {
        log("hello world");

        auto file_contents = open_file(log_file);
        REQUIRE(file_contents.has_value() == true);

        std::string contents_str = file_contents.value().to_str();

        REQUIRE_THAT(contents_str, Catch::Matchers::ContainsSubstring("INFO"));
        REQUIRE_THAT(contents_str, Catch::Matchers::ContainsSubstring("hello world"));
    }

    SECTION("Given level") {
        log(Level::WARNING, "hello world");

        auto file_contents = open_file(log_file);
        REQUIRE(file_contents.has_value() == true);

        std::string contents_str = file_contents.value().to_str();

        REQUIRE_THAT(contents_str, Catch::Matchers::ContainsSubstring("WARNING"));
        REQUIRE_THAT(contents_str, Catch::Matchers::ContainsSubstring("hello world"));
    }

    setenv("RAWTERM_DEBUG", "True", 1);
}
