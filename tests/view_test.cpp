#include "view.h"

#include <utility>

#include <rawterm/text.h>
#include <ut/ut.hpp>

#include "constants.h"
#include "controller.h"
#include "text_io.h"

boost::ut::suite<"View"> view_suite = [] {
    using namespace boost::ut;

    "Constructor"_test = [] {
        Controller c;
        auto v = View(&c, {24, 80});

        expect(v.view_models.capacity() == 8);
        expect(v.view_size.horizontal == 80);
        expect(v.view_size.vertical == 24);
    };

    "add_model"_test = [] {
        Controller c;
        lines_t raw = {"This is some text", "    here is a newline and tab", "and another newline"};

        auto v = View(&c, rawterm::Pos(24, 80));
        auto m = Model(raw, "test_file.txt");
        v.add_model(&m);

        expect(v.active_model == 0);
        expect(v.get_active_model()->filename == "test_file.txt");

        if (LINE_NUMBERS) {
            expect(v.line_number_offset == 2);
        }
    };

    "get_active_model"_test = [] {
        Controller c;
        lines_t raw = {"This is some text", "    here is a newline and tab", "and another newline"};

        auto v = View(&c, rawterm::Pos(24, 80));
        auto m = Model(raw, "test_file.txt");
        v.add_model(&m);

        expect(v.get_active_model() == &m);
    };

    "render_screen"_test = [] {
        should("Render single text file") = [] {
            Controller c;
            auto v = View(&c, rawterm::Pos(24, 80));
            auto m = Model(
                open_file("tests/fixture/test_file_1.txt").value(),
                "tests/fixture/test_file_1.txt");
            v.add_model(&m);

            auto buffer = lines(v.render_screen());

            expect(buffer.size() == 22);
            expect(buffer.at(0).at(5) == 'T');
            expect(buffer.at(1).at(1) == '2');
        };

        should("Truncated line") = [] {
            Controller c;
            auto v = View(&c, rawterm::Pos(24, 80));
            auto m = Model(32, "");
            v.add_model(&m);

            for (int i = 0; i < 80; i++) {
                m.buf.at(0).push_back('_');
            }

            auto buffer = lines(v.render_screen());

            expect(buffer.size() == 22);
            expect(buffer.at(0).substr(buffer.at(0).size() - 2, 2) == "\u00bb");
            expect(buffer.at(0).size() == 83);  // +3 for unicode chars
        };
    };

    "generate_tab_bar"_test = [] {
        Controller c;
        lines_t raw = {"This is some text", "    here is a newline and tab", "and another newline"};

        auto v = View(&c, rawterm::Pos(24, 80));
        auto m = Model(raw, "test_file.txt");

        v.add_model(&m);
        auto m2 = Model(
            open_file("tests/fixture/test_file_1.txt").value(), "tests/fixture/test_file_1.txt");
        v.add_model(&m2);

        std::string ret = v.render_tab_bar();
        std::string expected =
            "| test_file.txt | \x1B[7mtests/fixture/test_file_1.txt\x1B[27m | \n";

        expect(v.view_models.size() == 2);
        expect(v.active_model == 1);
        expect(expected == ret);
    };

    "render_line"_test = [] {
        should("Standard line rendering") = []() {
            Controller c;
            auto m = Model(
                open_file("tests/fixture/test_file_1.txt").value(),
                "tests/fixture/test_file_1.txt");

            auto v = View(&c, rawterm::Pos(24, 80));
            v.add_model(&m);

            const std::string line = rawterm::raw_str(v.render_line());

            std::string expected_text = "This is some text";
            unsigned int expected_size = expected_text.size();

            if (LINE_NUMBERS) {
                const std::string line_prefix = " 1\u2502";
                expected_text = line_prefix + expected_text;
                expected_size += line_prefix.size();
            }

            expect(line == expected_text);
            expect(line.size() == expected_size);
        };

        should("Truncated line") = []() {
            Controller c;
            auto m = Model(32, "");
            auto v = View(&c, rawterm::Pos(24, 80));
            v.add_model(&m);

            for (int i = 0; i <= 80; i++) {
                m.buf.at(0).push_back('_');
            }

            const std::string line = rawterm::raw_str(v.render_line());

            expect(line.substr(line.size() - 2, 2) == "\u00BB");
            expect(line.size() == 83);  // +3 for unicode chars
        };
    };

    "render_status_bar"_test = [] {
        Controller c;
        auto v = View(&c, rawterm::Pos(24, 80));

        auto m = Model(
            open_file("tests/fixture/test_file_1.txt").value(), "tests/fixture/test_file_1.txt");

        v.add_model(&m);
        std::string ret = v.render_status_bar();

        expect(ret.find("READ") != std::string::npos);
        expect(ret.find("test_file_1.txt") != std::string::npos);
        expect(ret.find("1:1") != std::string::npos);
        expect(rawterm::raw_size(ret) == 80);
    };

    "cursor_left"_test = [] {
        should("Already at left-most position") = [&]() {
            Controller c;
            auto v = View(&c, rawterm::Pos(24, 80));
            auto m = Model(
                open_file("tests/fixture/test_file_1.txt").value(),
                "tests/fixture/test_file_1.txt");
            v.add_model(&m);

            expect(v.cur == rawterm::Pos(1, 1));
            v.cursor_left();
            expect(v.cur == rawterm::Pos(1, 1));
        };

        should("Move left") = [&]() {
            Controller c;
            auto v = View(&c, rawterm::Pos(24, 80));
            auto m = Model(
                open_file("tests/fixture/test_file_1.txt").value(),
                "tests/fixture/test_file_1.txt");
            v.add_model(&m);

            for (int i = 1; i < 5; i++) {
                v.cursor_right();
            }
            v.cursor_left();
            expect(v.cur == rawterm::Pos(1, 4));
            expect(m.current_line == 0);
            expect(m.current_char == 3) << m.current_char;
        };
    };

    "cursor_up"_test = [] {
        should("Already at top-most row") = [] {
            Controller c;
            auto v = View(&c, rawterm::Pos(24, 80));
            auto m = Model(
                open_file("tests/fixture/test_file_1.txt").value(),
                "tests/fixture/test_file_1.txt");
            v.add_model(&m);

            expect(v.cur == rawterm::Pos(1, 1));
            v.cursor_up();
            expect(v.cur == rawterm::Pos(1, 1));
        };

        should("Move cursor up") = [] {
            Controller c;
            auto v = View(&c, rawterm::Pos(24, 80));
            auto m = Model(
                open_file("tests/fixture/test_file_1.txt").value(),
                "tests/fixture/test_file_1.txt");
            v.add_model(&m);
            v.cursor_down();
            v.cursor_down();
            expect(v.cur == rawterm::Pos(3, 1));

            std::ignore = v.cursor_up();

            expect(v.cur == rawterm::Pos(2, 1));
            expect(m.current_line == 1);
        };

        should("Move view up (scroll)") = [] {
            Controller c;
            auto v = View(&c, rawterm::Pos(24, 80));
            auto m = Model(
                open_file("tests/fixture/lorem_ipsum.txt").value(),
                "tests/fixture/lorem_ipsum.txt");
            v.add_model(&m);

            // Scroll down below initial view
            for (int i = 1; i < 35; i++) {
                v.cursor_down();
            }

            expect(v.cur == rawterm::Pos(22, 1));
            expect(m.current_line == 34);

            std::vector<std::string> text = lines(v.render_screen());
            const int idx = m.current_line - m.view_offset - 1;
            expect(text.at(idx).find("massa.") != std::string::npos);

            // Move cursor to top row
            for (int i = 1; i < 23; i++) {
                v.cursor_up();
            }

            expect(v.cur == rawterm::Pos(1, 1));
            expect(m.current_line == 12);

            text = lines(v.render_screen());
            expect(text.at(0).find("Lorem ipsum") == std::string::npos);
        };
    };

    "cursor_down"_test = [] {
        should("Move cursor down") = [&] {
            Controller c;
            auto m = Model(
                open_file("tests/fixture/lorem_ipsum.txt").value(),
                "tests/fixture/lorem_ipsum.txt");
            auto v = View(&c, rawterm::Pos(24, 80));
            v.add_model(&m);

            expect(v.cur == rawterm::Pos(1, 1));
            v.cursor_down();
            expect(v.cur == rawterm::Pos(2, 1));
            expect(m.current_line == 1);
        };

        should("Move view down (scroll)") = [&] {
            Controller c;
            auto m = Model(
                open_file("tests/fixture/lorem_ipsum.txt").value(),
                "tests/fixture/lorem_ipsum.txt");
            auto v = View(&c, rawterm::Pos(24, 80));
            v.add_model(&m);

            for (int i = 1; i < 22; i++) {
                v.cursor_down();
            }

            expect(v.cur == rawterm::Pos(22, 1));
            expect(m.current_line == 21);

            v.cursor_down();  // trigger scrolling

            expect(v.cur == rawterm::Pos(22, 1));
            expect(m.current_line == 22);

            v.cursor_down();  // trigger scrolling
            expect(v.cur == rawterm::Pos(22, 1));
            expect(m.current_line == 23);
        };

        should("Already at bottom-most row in file") = [&] {
            Controller c;
            auto m = Model(
                open_file("tests/fixture/lorem_ipsum.txt").value(),
                "tests/fixture/lorem_ipsum.txt");
            auto v = View(&c, rawterm::Pos(24, 80));
            v.add_model(&m);

            for (unsigned int i = 1; i < m.buf.size(); i++) {
                v.cursor_down();
            }

            expect(v.cur == rawterm::Pos(22, 1));
            expect(m.current_line == m.buf.size() - 1);

            v.cursor_down();
            expect(v.cur == rawterm::Pos(22, 1));
            expect(m.current_line == m.buf.size() - 1);
        };

        should("bottom row of file within view (no scrolling required)") = [&] {
            Controller c;
            auto m = Model(
                open_file("tests/fixture/test_file_1.txt").value(),
                "tests/fixture/test_file_1.txt");
            auto v = View(&c, rawterm::Pos(24, 80));
            v.add_model(&m);

            // Move to end of file
            v.cursor_down();
            v.cursor_down();
            expect(v.cur == rawterm::Pos(3, 1)) << v.cur;
            expect(m.current_line == 2);

            // No cursor movement as we're already at end of file
            v.cursor_down();
            expect(v.cur == rawterm::Pos(3, 1)) << v.cur;
            expect(m.current_line == 2);
        };
    };

    "cursor_right"_test = [] {
        should("Already at right-most position in view") = [] {
            Controller c;
            auto v = View(&c, rawterm::Pos(24, 80));
            auto m = Model(
                open_file("tests/fixture/test_file_1.txt").value(),
                "tests/fixture/test_file_1.txt");
            v.add_model(&m);
            v.cur.move({v.cur.vertical, v.line_number_offset + 1});

            for (int i = 1; i <= 80; i++) {
                v.cursor_right();
            }
            expect(v.cur == rawterm::Pos(1, 21));
            v.cursor_right();
            expect(v.cur == rawterm::Pos(1, 21));
        };

        should("Already at right-most position in line") = [] {
            Controller c;
            auto v = View(&c, rawterm::Pos(24, 80));
            auto m = Model(
                open_file("tests/fixture/test_file_1.txt").value(),
                "tests/fixture/test_file_1.txt");
            v.add_model(&m);
            v.cur.move({v.cur.vertical, v.line_number_offset + 1});

            for (int i = 1; i <= 100; i++) {
                v.cursor_right();
            }

            expect(v.cur == rawterm::Pos(1, 21));
            expect(m.current_char == 18);
        };

        should("Move right") = [] {
            Controller c;
            auto v = View(&c, rawterm::Pos(24, 80));
            auto m = Model(
                open_file("tests/fixture/test_file_1.txt").value(),
                "tests/fixture/test_file_1.txt");
            v.add_model(&m);

            v.cursor_right();
            expect(v.cur == rawterm::Pos(1, 2));
        };
    };
};
