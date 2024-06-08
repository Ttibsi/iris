#include "model.h"

#include <catch2/catch_test_macros.hpp>

#include "constants.h"
#include "editor.h"
#include "filesystem.h"

TEST_CASE("default constructor", "[MODEL]") {
    Editor e = Editor();
    auto m = Model(&e);

    REQUIRE(m.gv.size() == 0);
    REQUIRE(m.filename == "");
}

TEST_CASE("constructor with open file", "[MODEL]") {
    Editor e = Editor();
    auto file = open_file("tests/fixture/test_file_1.txt");
    auto m = Model(&e, Gapvector(file.value().begin(), file.value().end()), "test_file_1.txt");

    REQUIRE(m.gv.line(1) == "This is some text");
    REQUIRE(m.gv.size() == 68);
    REQUIRE(m.filename == "test_file_1.txt");
}

TEST_CASE("render", "[MODEL]") {
    // SKIP("THIS IS RUNNING VERY SLOW");
    Editor e;
    auto v = View(rawterm::Pos {24, 80});

    auto file = open_file("tests/fixture/test_file_1.txt");
    auto m = Model(&e, Gapvector(file.value().begin(), file.value().end()), "test_file_1.txt");

    std::vector<std::string> expected = {
        rawterm::set_foreground(" 1\u2502", COLOR_1) + "This is some text\n",
        rawterm::set_foreground(" 2\u2502", COLOR_1) + "    here is a newline and tab\n",
        rawterm::set_foreground(" 3\u2502", COLOR_1) + "and another newline\n",
    };
    auto actual = m.render(&v);

    // We aren't comparing the statusbar here, that's for the test below
    REQUIRE(actual.size() == 24);
    REQUIRE(actual.at(0) == expected.at(0));
    REQUIRE(actual.at(1) == expected.at(1));
    REQUIRE(actual.at(2) == expected.at(2));
}

TEST_CASE("render_status_bar", "[MODEL]") {
    Editor e;
    auto file = open_file("tests/fixture/test_file_1.txt");
    auto m = Model(&e, Gapvector(file.value().begin(), file.value().end()), "test_file_1.txt");
    REQUIRE(m.filename == "test_file_1.txt");

    auto actual = m.render_status_bar(80);

    REQUIRE(actual.find("READ") != std::string::npos);
    REQUIRE(actual.find(e.git_branch) != std::string::npos);
    REQUIRE(actual.find("1:1") != std::string::npos);
    REQUIRE(rawterm::raw_size(actual) == 80);
}
