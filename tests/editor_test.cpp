#include "editor.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("init", "[EDITOR]") {
    Editor e;
    e.init("tests/fixture/test_file_1.txt");

    REQUIRE(e.models.size() == 1);
    REQUIRE(e.models.at(0).gv.size() == 68);
}

TEST_CASE("start_controller", "[EDITOR]") {
    SKIP("May not be testable");
}

TEST_CASE("set_mode", "[EDITOR]") {
    Editor e;

    e.set_mode(Mode::Write);
    REQUIRE(e.mode == Mode::Write);

    e.set_mode(Mode::Read);
    REQUIRE(e.mode == Mode::Read);
}

TEST_CASE("get_mode", "[EDITOR]") {
    Editor e;

    REQUIRE(e.get_mode() == "READ");
    e.set_mode(Mode::Write);
    REQUIRE(e.get_mode() == "WRITE");
}

TEST_CASE("get_current_model", "[EDITOR]") {
    Editor e;
    e.init("tests/fixture/test_file_1.txt");

    REQUIRE(e.get_current_model() == 0);
}
