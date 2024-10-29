#include "view.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <rawterm/text.h>

#include "constants.h"
#include "controller.h"
#include "file_io.h"
#include "model.h"

// TODO: Create a test util function that takes in a lambda to wrap the stdout
// redirection

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
        if (line.find("~") != std::string::npos) {
            continue;
        }
        result.push_back(line);
    }

    result.pop_back();  // Remove the string of escape codes that gets the cursor to the right place
    return result;
}

TEST_CASE("render_screen", "[VIEW]") {
    Controller c;
    auto v = View(&c, rawterm::Pos(24, 80));

    SECTION("Render single text file") {
        auto m = Model(
            open_file("tests/fixture/test_file_1.txt").value(), "tests/fixture/test_file_1.txt");
        v.add_model(&m);

        // https://truong.io/posts/capturing_stdout_for_c++_unit_testing.html
        // capture stdout
        std::stringstream buffer;
        std::streambuf* prevcoutbuf = std::cout.rdbuf(buffer.rdbuf());

        v.render_screen();
        std::vector<std::string> text = splitStringOnNewlines(buffer.str());

        // restore stdout
        std::cout.rdbuf(prevcoutbuf);

        REQUIRE(text.size() == 4);
        REQUIRE(rawterm::raw_at(text.at(0), 5) == 'T');
        REQUIRE(rawterm::raw_at(text.at(1), 1) == '2');
    }

    SECTION("Truncated line") {
        auto m = Model();
        v.add_model(&m);

        for (int i = 0; i < 80; i++) {
            m.insert_char('_');
        }

        // capture stdout
        std::stringstream buffer;
        std::streambuf* prevcoutbuf = std::cout.rdbuf(buffer.rdbuf());

        v.render_screen();
        std::vector<std::string> text = splitStringOnNewlines(buffer.str());

        // restore stdout
        std::cout.rdbuf(prevcoutbuf);

        REQUIRE(text.size() == 3);
        // REQUIRE(rawterm::raw_str(text.at(0)).size() == 83); // TODO: rawterm 55
        REQUIRE(text.at(0).substr(text.at(0).size() - 3, 2) == "\u00bb");
    }
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
    Controller c;
    auto v = View(&c, rawterm::Pos(24, 80));

    SECTION("Standard line rendering") {
        auto m = Model(
            open_file("tests/fixture/test_file_1.txt").value(), "tests/fixture/test_file_1.txt");
        v.add_model(&m);

        // capture stdout
        std::stringstream buffer;
        std::streambuf* prevcoutbuf = std::cout.rdbuf(buffer.rdbuf());

        v.render_line();
        std::string text = buffer.str();

        // restore stdout
        std::cout.rdbuf(prevcoutbuf);

        int expected_size = 17;
        if (LINE_NUMBERS) {
            expected_size = 23;
        }

        // TODO: expected size is 22 if we only run the one test, but 23 when running all tests?
        REQUIRE(rawterm::raw_size(text) == expected_size);
        // REQUIRE(text == " 1\u2502This is some text"); // TODO: raw string - rawterm #55
    }

    SECTION("Truncated line") {
        auto m = Model();
        v.add_model(&m);

        for (int i = 0; i < 80; i++) {
            m.insert_char('_');
        }

        // capture stdout
        std::stringstream buffer;
        std::streambuf* prevcoutbuf = std::cout.rdbuf(buffer.rdbuf());

        v.render_line();
        std::string text = rawterm::raw_str(buffer.str());

        // restore stdout
        std::cout.rdbuf(prevcoutbuf);

        REQUIRE(text.substr(text.size() - 2, 2) == "\u00BB");
        // size is +3 because "\u2502".size() == 3 is "\u00bb".size() == 2
        REQUIRE(text.size() == 84);
    }
}

TEST_CASE("set_status", "[VIEW]") {
    SKIP("Not Tested Yet");
};

TEST_CASE("cursor_left", "[VIEW]") {
    Controller c;
    auto v = View(&c, rawterm::Pos(24, 80));
    auto m =
        Model(open_file("tests/fixture/test_file_1.txt").value(), "tests/fixture/test_file_1.txt");
    v.add_model(&m);

    std::stringstream buffer;
    std::streambuf* prevcoutbuf = std::cout.rdbuf(buffer.rdbuf());

    SECTION("Already at left-most position") {
        REQUIRE(v.cur == rawterm::Pos(1, 1));
        v.cursor_left();
        REQUIRE(v.cur == rawterm::Pos(1, 1));
    }

    SECTION("Move left") {
        for (int i = 1; i < 5; i++) {
            v.cursor_right();
        }
        v.cursor_left();
        REQUIRE(v.cur == rawterm::Pos(1, 4));
        REQUIRE(m.current_char_in_line == 4);
    }

    std::cout.rdbuf(prevcoutbuf);
};

TEST_CASE("cursor_up", "[VIEW]") {
    Controller c;
    auto v = View(&c, rawterm::Pos(24, 80));
    std::stringstream buffer;
    std::streambuf* prevcoutbuf = std::cout.rdbuf(buffer.rdbuf());

    SECTION("Already at top-most row") {
        auto m = Model(
            open_file("tests/fixture/test_file_1.txt").value(), "tests/fixture/test_file_1.txt");
        v.add_model(&m);

        REQUIRE(v.cur == rawterm::Pos(1, 1));
        v.cursor_up();
        REQUIRE(v.cur == rawterm::Pos(1, 1));
    }

    SECTION("Move cursor up") {
        auto m = Model(
            open_file("tests/fixture/test_file_1.txt").value(), "tests/fixture/test_file_1.txt");
        v.add_model(&m);

        m.current_line = 3;
        m.current_char_in_line = 3;
        v.cur = rawterm::Pos(3, 3);
        v.cursor_up();

        REQUIRE(v.cur == rawterm::Pos(2, 3));
        REQUIRE(m.current_line == 2);
    }

    SECTION("Move view up (scroll)") {
        auto m = Model(
            open_file("tests/fixture/test_file_2.txt").value(), "tests/fixture/test_file_2.txt");
        v.add_model(&m);

        // Scroll down below initial view
        for (int i = 1; i < 27; i++) {
            v.cursor_down();
        }
        // Move cursor to top row
        for (int i = 1; i < 23; i++) {
            v.cursor_up();
        }
        REQUIRE(v.cur == rawterm::Pos(1, 5));
        REQUIRE(m.current_line == 5);

        buffer.str(std::string());  // Empty stringstream buffer

        v.render_screen();
        std::vector<std::string> screenshot = splitStringOnNewlines(buffer.str());
        REQUIRE(screenshot.at(0).find("lectus tempor.") != std::string::npos);
    }

    std::cout.rdbuf(prevcoutbuf);
};

TEST_CASE("cursor_down", "[VIEW]") {
    Controller c;
    auto v = View(&c, rawterm::Pos(24, 80));

    std::stringstream buffer;
    std::streambuf* prevcoutbuf = std::cout.rdbuf(buffer.rdbuf());

    SECTION("Move cursor down") {
        auto m = Model(
            open_file("tests/fixture/test_file_2.txt").value(), "tests/fixture/test_file_2.txt");
        v.add_model(&m);

        v.cursor_down();
        REQUIRE(v.cur == rawterm::Pos(2, 1));
        REQUIRE(m.current_line == 2);
    }

    SECTION("Move view down (scroll)") {
        auto m = Model(
            open_file("tests/fixture/test_file_2.txt").value(), "tests/fixture/test_file_2.txt");
        v.add_model(&m);

        for (int i = 1; i < 22; i++) {
            v.cursor_down();
        }
        REQUIRE(v.cur == rawterm::Pos(22, 1));
        REQUIRE(m.current_line == 22);

        v.cursor_down();  // trigger scrolling
        REQUIRE(v.cur == rawterm::Pos(22, 5));
        REQUIRE(m.current_line == 23);

        v.cursor_down();  // trigger scrolling
        REQUIRE(v.cur == rawterm::Pos(22, 5));
        REQUIRE(m.current_line == 24);
    }

    SECTION("Already at bottom-most row in file") {
        auto m = Model(
            open_file("tests/fixture/test_file_2.txt").value(), "tests/fixture/test_file_2.txt");
        v.add_model(&m);

        for (int i = 1; i < m.line_count; i++) {
            v.cursor_down();
        }
        REQUIRE(v.cur == rawterm::Pos(22, 5));
        REQUIRE(m.current_line == m.line_count);

        v.cursor_down();
        REQUIRE(v.cur == rawterm::Pos(22, 5));
        REQUIRE(m.current_line == m.line_count);
    }

    SECTION("bottom row of file within view (no scrolling required)") {
        auto m = Model(
            open_file("tests/fixture/test_file_1.txt").value(), "tests/fixture/test_file_1.txt");
        v.add_model(&m);

        // Move to end of file
        v.cursor_down();
        v.cursor_down();
        REQUIRE(v.cur == rawterm::Pos(2, 1));
        REQUIRE(m.current_line == 2);

        // No cursor movement as we're already at end of file
        v.cursor_down();
        REQUIRE(v.cur == rawterm::Pos(2, 1));
        REQUIRE(m.current_line == 2);
    }

    std::cout.rdbuf(prevcoutbuf);
};

TEST_CASE("cursor_right", "[VIEW]") {
    Controller c;
    auto v = View(&c, rawterm::Pos(24, 80));
    auto m =
        Model(open_file("tests/fixture/test_file_1.txt").value(), "tests/fixture/test_file_1.txt");
    v.add_model(&m);

    std::stringstream buffer;
    std::streambuf* prevcoutbuf = std::cout.rdbuf(buffer.rdbuf());

    SECTION("Already at right-most position in view") {
        v.cur.move({1, 80});
        REQUIRE(v.cur == rawterm::Pos(1, 80));
        v.cursor_right();
        REQUIRE(v.cur == rawterm::Pos(1, 80));
    }

    SECTION("Already at right-most position in line") {
        for (int i = 1; i < 17; i++) {
            v.cursor_right();
        }
        REQUIRE(v.cur == rawterm::Pos(1, 17));
        REQUIRE(m.current_char_in_line == 17);
    }

    SECTION("Move right") {
        v.cursor_right();
        REQUIRE(v.cur == rawterm::Pos(1, 2));
    }

    std::cout.rdbuf(prevcoutbuf);
};
