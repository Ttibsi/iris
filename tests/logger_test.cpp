#include <string>

#include <catch2/catch_test_macros.hpp>

#include "logger.h"

// TODO: Logger tests
TEST_CASE("Log without specifying a level", "[Logger]") {
    const std::string log_file = "test1.log";
}

TEST_CASE("Log with a level", "[Logger]") {
    const std::string log_file = "test2.log";
}

TEST_CASE("Log with no message", "[Logger]") {
    const std::string log_file = "test3.log";
}

TEST_CASE("Log without defining a log file", "[Logger]") {}
