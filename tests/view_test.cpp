#include "view.h"

#include <catch2/catch_test_macros.hpp>
#include <rawterm/text.h>

#include <sstream>
#include <string>
#include <vector>
#include "constants.h"
#include "controller.h"
#include "filesystem.h"
#include "model.h"

TEST_CASE("Constructor", "[VIEW]") {
    Controller c;
    auto v = View(&c, rawterm::Pos(24, 80));

    REQUIRE(v.open_files.capacity() == 8);
    REQUIRE(v.view_size.horizontal == 80);
    REQUIRE(v.view_size.vertical == 24);
}

View setup() {
    Controller c;
    auto v = View(&c, rawterm::Pos(24, 80));

    std::vector<char> expected = {
        'T',  'h', 'i', 's', ' ', 'i', 's', ' ', 's', 'o', 'm', 'e', ' ', 't',  'e', 'x', 't',
        '\n', ' ', ' ', ' ', ' ', 'h', 'e', 'r', 'e', ' ', 'i', 's', ' ', 'a',  ' ', 'n', 'e',
        'w',  'l', 'i', 'n', 'e', ' ', 'a', 'n', 'd', ' ', 't', 'a', 'b', '\n', 'a', 'n', 'd',
        ' ',  'a', 'n', 'o', 't', 'h', 'e', 'r', ' ', 'n', 'e', 'w', 'l', 'i',  'n', 'e', '\n'};

    auto m = Model(expected, "test_file.txt");
    v.add_model(&m);
    return v;
}

TEST_CASE("add_model", "[VIEW]") {
    View v = setup();

    REQUIRE(v.active_model == 1);
    REQUIRE(v.viewable_models.at(v.active_model - 1)->file_name == "test_file.txt");

    if (LINE_NUMBERS) {
        REQUIRE(v.line_number_offset == 2);
    }
}

TEST_CASE("close_model", "[VIEW]") {
    SKIP("Not implemented yet");
}

TEST_CASE("change_view_forward", "[VIEW]") {
    SKIP("Not implemented yet");
}

TEST_CASE("change_view_backward", "[VIEW]") {
    SKIP("Not implemented yet");
}

// TODO: Find better place to put this
std::vector<std::string> splitStringOnNewlines(const std::string& input) {
    std::vector<std::string> result;
    std::istringstream stream(input);
    std::string line;

    while (std::getline(stream, line)) {
        result.push_back(line);
    }

    return result;
}

TEST_CASE("render_screen", "[VIEW]") {
    Controller c;
    auto v = View(&c, rawterm::Pos(24, 80));

    auto m =
        Model(open_file("tests/fixture/test_file_1.txt").value(), "tests/fixture/test_file_1.txt");
    v.add_model(&m);

    // https://truong.io/posts/capturing_stdout_for_c++_unit_testing.html
    // capture stdout
    std::stringstream buffer;
    std::streambuf* prevcoutbuf = std::cout.rdbuf(buffer.rdbuf());

    v.render_screen();
    std::vector<std::string> text = splitStringOnNewlines(buffer.str());

    // restore stdout
    std::cout.rdbuf(prevcoutbuf);

    REQUIRE(text.size() == 3);
    REQUIRE(rawterm::raw_at(text.at(0), 5) == 'T');
    REQUIRE(rawterm::raw_at(text.at(1), 1) == '2');
}

TEST_CASE("generate_tab_bar", "[VIEW]") {
    auto v = setup();
    auto m2 =
        Model(open_file("tests/fixture/test_file_1.txt").value(), "tests/fixture/test_file_1.txt");
    v.add_model(&m2);

    std::string ret = v.generate_tab_bar();
    std::string expected = "| test_file.txt | \x1B[7mtests/fixture/test_file_1.txt\x1B[27m | \n";

    REQUIRE(expected == ret);
}

TEST_CASE("draw_status_bar", "[VIEW]") {
    Controller c;
    auto v = View(&c, rawterm::Pos(24, 80));

    auto m =
        Model(open_file("tests/fixture/test_file_1.txt").value(), "tests/fixture/test_file_1.txt");
    v.add_model(&m);
    rawterm::Pos cur_pos = v.cur;

    // capture stdout
    std::stringstream buffer;
    std::streambuf* prevcoutbuf = std::cout.rdbuf(buffer.rdbuf());

    v.draw_status_bar();

    std::string text = buffer.str();

    // restore stdout
    std::cout.rdbuf(prevcoutbuf);

    REQUIRE(v.cur == cur_pos);
}

TEST_CASE("render_status_bar", "[VIEW]") {
    Controller c;
    auto v = View(&c, rawterm::Pos(24, 80));

    auto m =
        Model(open_file("tests/fixture/test_file_1.txt").value(), "tests/fixture/test_file_1.txt");
    v.add_model(&m);
    std::string ret = v.render_status_bar();

    REQUIRE(ret.find("READ") != std::string::npos);
    REQUIRE(ret.find("test_file_1.txt") != std::string::npos);
    REQUIRE(ret.find("1:1") != std::string::npos);
    REQUIRE(rawterm::raw_size(ret) == 80);
}

TEST_CASE("render_line", "[VIEW]") {
    SKIP("Not implemented yet");
}
TEST_CASE("set_status", "[VIEW]") {
    SKIP("Not Tested Yet");
};

TEST_CASE("cursor_left", "[VIEW]") {
    SKIP("Not Tested Yet");
};

TEST_CASE("cursor_up", "[VIEW]") {
    SKIP("Not Tested Yet");
};

TEST_CASE("cursor_down", "[VIEW]") {
    SKIP("Not Tested Yet");
};

TEST_CASE("cursor_right", "[VIEW]") {
    SKIP("Not Tested Yet");
};
