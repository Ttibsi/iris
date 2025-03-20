#include "controller.h"

#include <catch2/catch_test_case_info.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Construction", "[controller]") {
    Controller c;
    REQUIRE(c.models.capacity() == 8);
}

TEST_CASE("set_mode", "[controller]") {
    Controller c;

    c.set_mode(Mode::Write);
    REQUIRE(c.mode == Mode::Write);

    c.set_mode(Mode::Read);
    REQUIRE(c.mode == Mode::Read);
}

TEST_CASE("get_mode", "[controller]") {
    Controller c;

    REQUIRE(c.get_mode() == "READ");
    c.set_mode(Mode::Write);
    REQUIRE(c.get_mode() == "WRITE");
}

TEST_CASE("create_view", "[controller]") {
    SECTION("View with file") {
        Controller c;
        c.create_view("tests/fixture/test_file_1.txt");

        REQUIRE(c.models.size() == 1);
        REQUIRE(*c.models.at(0).buf.at(0).begin() == 'T');
    }

    SECTION("Empty view") {
        Controller c;
        c.create_view("");

        REQUIRE(c.models.size() == 1);
        REQUIRE(c.models.at(0).buf.size() == 1);
    }
}

TEST_CASE("start_action_engine", "[controller]") {
    SKIP("TODO");
}
