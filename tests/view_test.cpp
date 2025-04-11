#include "view.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <rawterm/text.h>

#include "constants.h"
#include "controller.h"
#include "text_io.h"

TEST_CASE("Constructor", "[view]") {
    Controller c;
    auto v = View(&c, {24, 80});

    REQUIRE(v.view_models.capacity() == 8);
    REQUIRE(v.view_size.horizontal == 80);
    REQUIRE(v.view_size.vertical == 24);
}

TEST_CASE("add_model", "[view]") {
    Controller c;
    lines_t raw = {"This is some text", "    here is a newline and tab", "and another newline"};

    auto v = View(&c, rawterm::Pos(24, 80));
    auto m = Model(raw, "test_file.txt");
    v.add_model(&m);

    REQUIRE(v.active_model == 0);
    REQUIRE(v.get_active_model()->filename == "test_file.txt");

    if (LINE_NUMBERS) {
        REQUIRE(v.line_number_offset == 2);
    }
}

TEST_CASE("get_active_model", "[view]") {
    Controller c;
    lines_t raw = {"This is some text", "    here is a newline and tab", "and another newline"};

    auto v = View(&c, rawterm::Pos(24, 80));
    auto m = Model(raw, "test_file.txt");
    v.add_model(&m);

    REQUIRE(v.get_active_model() == &m);
}

TEST_CASE("render_screen", "[view]") {
    SECTION("Render single text file") {
        Controller c;
        auto v = View(&c, rawterm::Pos(24, 80));
        auto m = Model(
            open_file("tests/fixture/test_file_1.txt").value(), "tests/fixture/test_file_1.txt");
        v.add_model(&m);

        auto buffer = lines(v.render_screen());

        REQUIRE(buffer.size() == 22);
        REQUIRE(buffer.at(0).at(5) == 'T');
        REQUIRE(buffer.at(1).at(1) == '2');
    }

    SECTION("Truncated line") {
        Controller c;
        auto v = View(&c, rawterm::Pos(24, 80));
        auto m = Model(32, "");
        v.add_model(&m);

        for (int i = 0; i < 80; i++) {
            m.buf.at(0).push_back('_');
        }

        auto buffer = lines(v.render_screen());

        REQUIRE(buffer.size() == 22);
        REQUIRE(buffer.at(0).substr(buffer.at(0).size() - 2, 2) == "\u00bb");
        REQUIRE(buffer.at(0).size() == 83);  // +3 for unicode chars
    }
}

TEST_CASE("generate_tab_bar", "[view]") {
    Controller c;
    lines_t raw = {"This is some text", "    here is a newline and tab", "and another newline"};

    auto v = View(&c, rawterm::Pos(24, 80));
    auto m = Model(raw, "test_file.txt");

    v.add_model(&m);
    auto m2 =
        Model(open_file("tests/fixture/test_file_1.txt").value(), "tests/fixture/test_file_1.txt");
    v.add_model(&m2);

    std::string ret = v.render_tab_bar();
    std::string expected = "| test_file.txt | \x1B[7mtests/fixture/test_file_1.txt\x1B[27m | \n";

    REQUIRE(v.view_models.size() == 2);
    REQUIRE(v.active_model == 1);
    REQUIRE(expected == ret);
}

TEST_CASE("render_line", "[view]") {
    SECTION("Standard line rendering") {
        Controller c;
        auto m = Model(
            open_file("tests/fixture/test_file_1.txt").value(), "tests/fixture/test_file_1.txt");

        auto v = View(&c, rawterm::Pos(24, 80));
        v.add_model(&m);

        const std::string line = rawterm::raw_str(v.render_line(0));

        std::string expected_text = "This is some text";
        std::size_t expected_size = expected_text.size();

        if (LINE_NUMBERS) {
            const std::string line_prefix = " 1\u2502";
            expected_text = line_prefix + expected_text;
            expected_size += line_prefix.size();
        }

        REQUIRE(line == expected_text);
        REQUIRE(line.size() == expected_size);
    }

    SECTION("Truncated line") {
        Controller c;
        auto m = Model(32, "");
        auto v = View(&c, rawterm::Pos(24, 80));
        v.add_model(&m);

        for (int i = 0; i <= 80; i++) {
            m.buf.at(0).push_back('_');
        }

        const std::string line = rawterm::raw_str(v.render_line(0));

        REQUIRE(line.substr(line.size() - 2, 2) == "\u00BB");
        REQUIRE(line.size() == 83);  // +3 for unicode chars
    }
}

TEST_CASE("render_status_bar", "[view]") {
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

TEST_CASE("clamp_horizontal_movement", "[view]") {
    Controller c;
    auto v = View(&c, rawterm::Pos(24, 80));

    auto m =
        Model(open_file("tests/fixture/lorem_ipsum.txt").value(), "tests/fixture/lorem_ipsum.txt");

    v.add_model(&m);
    m.current_line = 7;
    m.current_char = 7;

    auto ret = v.clamp_horizontal_movement(1);
    REQUIRE(ret.has_value());
    REQUIRE(v.prev_cur_hor_pos == 1);
    REQUIRE(ret.value() == 0);

    m.current_line++;
    ret = v.clamp_horizontal_movement(1);
    REQUIRE(ret.has_value());
    REQUIRE(ret.value() == 1);
    REQUIRE(v.prev_cur_hor_pos == -1);
}

TEST_CASE("cursor_left", "[view]") {
    SECTION("Already at left-most position") {
        Controller c;
        auto v = View(&c, rawterm::Pos(24, 80));
        auto m = Model(
            open_file("tests/fixture/test_file_1.txt").value(), "tests/fixture/test_file_1.txt");
        v.add_model(&m);

        REQUIRE(v.cur == rawterm::Pos(1, 1));
        v.cursor_left();
        REQUIRE(v.cur == rawterm::Pos(1, 1));
    }

    SECTION("Move left") {
        Controller c;
        auto v = View(&c, rawterm::Pos(24, 80));
        auto m = Model(
            open_file("tests/fixture/test_file_1.txt").value(), "tests/fixture/test_file_1.txt");
        v.add_model(&m);

        for (int i = 1; i < 5; i++) {
            v.cursor_right();
        }
        v.cursor_left();
        REQUIRE(v.cur == rawterm::Pos(1, 4));
        REQUIRE(m.current_line == 0);
        REQUIRE(m.current_char == 3);
    }
}

TEST_CASE("cursor_up", "[view]") {
    SECTION("Already at top-most row") {
        Controller c;
        auto v = View(&c, rawterm::Pos(24, 80));
        auto m = Model(
            open_file("tests/fixture/test_file_1.txt").value(), "tests/fixture/test_file_1.txt");
        v.add_model(&m);

        REQUIRE(v.cur == rawterm::Pos(1, 1));
        v.cursor_up();
        REQUIRE(v.cur == rawterm::Pos(1, 1));
    }

    SECTION("Move cursor up") {
        Controller c;
        auto v = View(&c, rawterm::Pos(24, 80));
        auto m = Model(
            open_file("tests/fixture/test_file_1.txt").value(), "tests/fixture/test_file_1.txt");
        v.add_model(&m);
        v.cursor_down();
        v.cursor_down();
        REQUIRE(v.cur == rawterm::Pos(3, 1));

        std::ignore = v.cursor_up();

        REQUIRE(v.cur == rawterm::Pos(2, 1));
        REQUIRE(m.current_line == 1);
    }

    SECTION("Move view up (scroll)") {
        Controller c;
        auto v = View(&c, rawterm::Pos(24, 80));
        auto m = Model(
            open_file("tests/fixture/lorem_ipsum.txt").value(), "tests/fixture/lorem_ipsum.txt");
        v.add_model(&m);

        // Scroll down below initial view
        for (int i = 1; i < 35; i++) {
            v.cursor_down();
        }

        REQUIRE(v.cur == rawterm::Pos(22, 1));
        REQUIRE(m.current_line == 34);

        std::vector<std::string> text = lines(v.render_screen());
        const uint_t idx = m.current_line - m.view_offset - 1;
        REQUIRE(text.at(idx).find("massa.") != std::string::npos);

        // Move cursor to top row
        for (int i = 1; i < 23; i++) {
            v.cursor_up();
        }

        REQUIRE(v.cur == rawterm::Pos(1, 1));
        REQUIRE(m.current_line == 12);

        text = lines(v.render_screen());
        REQUIRE(text.at(0).find("Lorem ipsum") == std::string::npos);
    }
}

TEST_CASE("cursor_down", "[view]") {
    SECTION("Move cursor down") {
        Controller c;
        auto m = Model(
            open_file("tests/fixture/lorem_ipsum.txt").value(), "tests/fixture/lorem_ipsum.txt");
        auto v = View(&c, rawterm::Pos(24, 80));
        v.add_model(&m);

        REQUIRE(v.cur == rawterm::Pos(1, 1));
        v.cursor_down();
        REQUIRE(v.cur == rawterm::Pos(2, 1));
        REQUIRE(m.current_line == 1);
    }

    SECTION("Move view down (scroll)") {
        Controller c;
        auto m = Model(
            open_file("tests/fixture/lorem_ipsum.txt").value(), "tests/fixture/lorem_ipsum.txt");
        auto v = View(&c, rawterm::Pos(24, 80));
        v.add_model(&m);

        for (int i = 1; i < 22; i++) {
            v.cursor_down();
        }

        REQUIRE(v.cur == rawterm::Pos(22, 1));
        REQUIRE(m.current_line == 21);

        v.cursor_down();  // trigger scrolling

        REQUIRE(v.cur == rawterm::Pos(22, 1));
        REQUIRE(m.current_line == 22);

        v.cursor_down();  // trigger scrolling
        REQUIRE(v.cur == rawterm::Pos(22, 1));
        REQUIRE(m.current_line == 23);
    }

    SECTION("Already at bottom-most row in file") {
        Controller c;
        auto m = Model(
            open_file("tests/fixture/lorem_ipsum.txt").value(), "tests/fixture/lorem_ipsum.txt");
        auto v = View(&c, rawterm::Pos(24, 80));
        v.add_model(&m);

        for (unsigned int i = 1; i < m.buf.size(); i++) {
            v.cursor_down();
        }

        REQUIRE(v.cur == rawterm::Pos(22, 1));
        REQUIRE(m.current_line == m.buf.size() - 1);

        v.cursor_down();
        REQUIRE(v.cur == rawterm::Pos(22, 1));
        REQUIRE(m.current_line == m.buf.size() - 1);
    }

    SECTION("bottom row of file within view (no scrolling required)") {
        Controller c;
        auto m = Model(
            open_file("tests/fixture/test_file_1.txt").value(), "tests/fixture/test_file_1.txt");
        auto v = View(&c, rawterm::Pos(24, 80));
        v.add_model(&m);

        // Move to end of file
        v.cursor_down();
        v.cursor_down();
        REQUIRE(v.cur == rawterm::Pos(3, 1));
        REQUIRE(m.current_line == 2);

        // No cursor movement as we're already at end of file
        v.cursor_down();
        REQUIRE(v.cur == rawterm::Pos(3, 1));
        REQUIRE(m.current_line == 2);
    }
}

TEST_CASE("cursor_right", "[view]") {
    SECTION("Already at right-most position in view") {
        Controller c;
        auto v = View(&c, rawterm::Pos(24, 80));
        auto m = Model(
            open_file("tests/fixture/test_file_1.txt").value(), "tests/fixture/test_file_1.txt");
        v.add_model(&m);
        v.cur.move({v.cur.vertical, v.line_number_offset + 1});

        for (int i = 1; i <= 80; i++) {
            v.cursor_right();
        }
        REQUIRE(v.cur == rawterm::Pos(1, 21));
        v.cursor_right();
        REQUIRE(v.cur == rawterm::Pos(1, 21));
    }

    SECTION("Already at right-most position in line") {
        Controller c;
        auto v = View(&c, rawterm::Pos(24, 80));
        auto m = Model(
            open_file("tests/fixture/test_file_1.txt").value(), "tests/fixture/test_file_1.txt");
        v.add_model(&m);
        v.cur.move({v.cur.vertical, v.line_number_offset + 1});

        for (int i = 1; i <= 100; i++) {
            v.cursor_right();
        }

        REQUIRE(v.cur == rawterm::Pos(1, 21));
        REQUIRE(m.current_char == 18);
    }

    SECTION("Move right") {
        Controller c;
        auto v = View(&c, rawterm::Pos(24, 80));
        auto m = Model(
            open_file("tests/fixture/test_file_1.txt").value(), "tests/fixture/test_file_1.txt");
        v.add_model(&m);

        v.cursor_right();
        REQUIRE(v.cur == rawterm::Pos(1, 2));
    }
}

TEST_CASE("cursor_end_of_line", "[view]") {
    Controller c;
    auto v = View(&c, rawterm::Pos(24, 80));
    auto m =
        Model(open_file("tests/fixture/test_file_1.txt").value(), "tests/fixture/test_file_1.txt");
    v.add_model(&m);

    REQUIRE(v.cur == rawterm::Pos(1, 1));
    v.cursor_end_of_line();
    REQUIRE(v.cur == rawterm::Pos(1, 18));
}

TEST_CASE("center_current_line", "[view]") {
    SECTION("Do nothing if current line is already near the top") {
        Controller c;
        auto v = View(&c, rawterm::Pos(24, 80));
        auto m = Model(
            open_file("tests/fixture/lorem_ipsum.txt").value(), "tests/fixture/lorem_ipsum.txt");
        v.add_model(&m);

        // Initial state
        REQUIRE(m.current_line == 0);
        REQUIRE(m.view_offset == 0);
        REQUIRE(v.cur == rawterm::Pos(1, 1));

        v.center_current_line();

        // Should not change anything
        REQUIRE(m.current_line == 0);
        REQUIRE(m.view_offset == 0);
        REQUIRE(v.cur == rawterm::Pos(1, 5));
    }

    SECTION("Center the view when current line is far down") {
        Controller c;
        auto v = View(&c, rawterm::Pos(24, 80));
        auto m = Model(
            open_file("tests/fixture/lorem_ipsum.txt").value(), "tests/fixture/lorem_ipsum.txt");
        v.add_model(&m);

        // Move cursor far down
        for (int i = 0; i < 30; i++) {
            v.cursor_down();
        }

        // Check state before centering
        REQUIRE(m.current_line == 30);
        REQUIRE(m.view_offset > 0);

        v.center_current_line();

        // View should be centered on line 30
        REQUIRE(m.current_line == 30);
        REQUIRE(m.view_offset == 30 - std::floor(v.view_size.vertical / 2));
        REQUIRE(
            v.cur == rawterm::Pos(
                         static_cast<int>(std::floor(v.view_size.vertical / 2)) + 1,
                         v.line_number_offset + 2));
    }
}
