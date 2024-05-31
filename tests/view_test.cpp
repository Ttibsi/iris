#include "view.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("constructor", "[VIEW]") {
    auto v = View(rawterm::Pos {10, 10});

    REQUIRE(v.pane_manager.count() == 1);
    REQUIRE(v.pane_manager.active() == 1);
    REQUIRE(v.pane_manager.get_size() == rawterm::Pos {10, 10});
}
