#include <catch2/catch_test_macros.hpp>

#include "editor.h"

TEST_CASE("set_mode", "[Editor]") {
    Editor e{ "" };

    e.set_mode(Mode::Write);
    REQUIRE(e.mode == Mode::Write);

    e.set_mode(Mode::Read);
    REQUIRE(e.mode == Mode::Read);

    e.set_mode(Mode::Command);
    REQUIRE(e.mode == Mode::Command);
}

TEST_CASE("get_mode", "[Editor]") {
    Editor e{ "" };

    REQUIRE(e.get_mode() == "READ");
    e.set_mode(Mode::Command);
    REQUIRE(e.get_mode() == "COMMAND");
}
