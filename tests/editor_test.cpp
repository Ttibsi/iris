#include "editor.h"

#include <catch2/catch_test_macros.hpp>

#include "filesystem.h"

TEST_CASE("constructor", "[EDITOR]") {
    Editor e;

    REQUIRE(e.models.capacity() == 8);
    REQUIRE(e.views.capacity() == 8);
    REQUIRE(e.models.size() == 0);
    REQUIRE(e.views.size() == 1);
}

TEST_CASE("init", "[EDITOR]") {
    Editor e;
    auto v = View(rawterm::Pos {24, 80});
    e.views.push_back(v);
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
