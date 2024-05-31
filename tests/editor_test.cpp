#include "editor.h"

#include <catch2/catch_test_macros.hpp>

#include "constants.h"

TEST_CASE("init", "[EDITOR]") {
    Editor e;
    e.init("tests/fixture/test_file_1.txt");

    REQUIRE(e.models.size() == 1);
    REQUIRE(e.models.at(0).size() == 68);
}

TEST_CASE("start_controller", "[EDITOR]") {
    SKIP("May not be testable");
}

TEST_CASE("model_to_view", "[EDITOR]") {
    Editor e;
    e.init("tests/fixture/test_file_1.txt");

    std::vector<std::string> expected = {
        rawterm::set_foreground(" 1\u2502", COLOR_1) + "This is some text\n",
        rawterm::set_foreground(" 2\u2502", COLOR_1) + "    here is a newline and tab\n",
        rawterm::set_foreground(" 3\u2502", COLOR_1) + "and another newline\n",
    };
    auto actual = e.model_to_view();

    REQUIRE(actual == expected);
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
