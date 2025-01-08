#include "view.h"

#include <utility>

#include <rawterm/text.h>
#include <ut/ut.hpp>

#include "constants.h"
#include "controller.h"
#include "text_io.h"

boost::ut::suite<"View"> view_suite = [] {
    using namespace boost::ut;

    skip / "Constructor"_test = [] {
        Controller c;
        auto v = View(&c, rawterm::Pos(24, 80));

        expect(v.view_models.capacity() == 8);
        expect(v.view_size.horizontal == 80);
        expect(v.view_size.vertical == 24);
    };

    skip / "add_model"_test = [] {
        Controller c;
        lines_t raw = {
            "This is some text\n", "    here is a newline and tab\n", "and another newline\n"};

        auto v = View(&c, rawterm::Pos(24, 80));
        auto m = Model(raw, "test_file.txt");
        v.add_model(&m);

        expect(v.active_model == 1);
        expect(v.get_active_model()->filename == "test_file.txt");

        if (LINE_NUMBERS) {
            expect(v.line_number_offset == 2);
        }
    };

    skip / "get_active_model"_test = [] {
        Controller c;
        lines_t raw = {
            "This is some text\n", "    here is a newline and tab\n", "and another newline\n"};

        auto v = View(&c, rawterm::Pos(24, 80));
        auto m = Model(raw, "test_file.txt");
        v.add_model(&m);

        expect(v.get_active_model() == &m);
    };

    skip / "render_screen"_test = [] {
        Controller c;
        auto v = View(&c, rawterm::Pos(24, 80));

        should("Render single text file") = [=]() mutable {
            auto m = Model(
                open_file("tests/fixture/test_file_1.txt").value(),
                "tests/fixture/test_file_1.txt");
            v.add_model(&m);

            auto buffer = lines(v.render_screen());

            expect(buffer.size() == 23);
            expect(rawterm::raw_at(buffer.at(0), 5) == 'T');
            expect(rawterm::raw_at(buffer.at(1), 1) == '2');
        };

        should("Truncated line") = [=]() mutable {
            auto m = Model(32);
            v.add_model(&m);

            for (int i = 0; i < 80; i++) {
                m.buf.at(0).push_back('_');
            }

            auto buffer = lines(v.render_screen());

            expect(buffer.size() == 23);
            expect(rawterm::raw_size(buffer.at(0)) == 82);
            expect(buffer.at(0).substr(buffer.at(0).size() - 3, 2) == "\u00bb");
        };
    };

    skip / "generate_tab_bar"_test = [] {
        Controller c;
        lines_t raw = {
            "This is some text\n", "    here is a newline and tab\n", "and another newline\n"};

        auto v = View(&c, rawterm::Pos(24, 80));
        auto m = Model(raw, "test_file.txt");

        v.add_model(&m);
        auto m2 = Model(
            open_file("tests/fixture/test_file_1.txt").value(), "tests/fixture/test_file_1.txt");
        v.add_model(&m2);

        std::string ret = v.render_tab_bar();
        std::string expected =
            "| test_file.txt | \x1B[7mtests/fixture/test_file_1.txt\x1B[27m | "
            "\n";

        expect(expected == ret);
    };

    skip / "render_line"_test = [] {
        should("Standard line rendering") = []() {
            Controller c;
            auto m = Model(
                open_file("tests/fixture/test_file_1.txt").value(),
                "tests/fixture/test_file_1.txt");

            auto v = View(&c, rawterm::Pos(24, 80));
            v.add_model(&m);

            const std::string line = rawterm::raw_str(v.render_line());

            std::string expected_text = "This is some text\r\n";
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
            auto m = Model(32);
            auto v = View(&c, rawterm::Pos(24, 80));
            v.add_model(&m);

            for (int i = 0; i <= 80; i++) {
                m.buf.at(0).push_back('_');
            }

            const std::string line = rawterm::raw_str(v.render_line());

            expect(line.substr(line.size() - 2, 2) == "\u00BB");
            expect(line.size() == 83);  // adding len of unicode chars, removing \r\n
        };
    };

    skip / "render_status_bar"_test = [] {
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

    skip / "cursor_left"_test = [] {
        Controller c;
        auto v = View(&c, rawterm::Pos(24, 80));
        auto m = Model(
            open_file("tests/fixture/test_file_1.txt").value(), "tests/fixture/test_file_1.txt");
        v.add_model(&m);

        should("Already at left-most position") = [&]() {
            expect(v.cur == rawterm::Pos(1, 1));
            v.cursor_left();
            expect(v.cur == rawterm::Pos(1, 1));
        };

        should("Move left") = [&]() {
            for (int i = 1; i < 5; i++) {
                v.cursor_right();
            }
            v.cursor_left();
            expect(v.cur == rawterm::Pos(1, 4));
            expect(m.current_line == 3);
        };
    };

    skip / "cursor_up"_test = [] {
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

            // Move cursor to top row
            for (int i = 1; i < 22; i++) {
                v.cursor_up();
            }

            expect(v.cur == rawterm::Pos(1, 1));
            expect(m.current_line == 10);

            std::vector<std::string> text = lines(v.render_screen());
            expect(text.at(0).find("Lorem ipsum dolor sit amet,") == std::string::npos);
        };
    };

    skip / "cursor_down"_test = [] {
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

            for (int i = 1; i < m.buf.size(); i++) {
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
            expect(v.cur == rawterm::Pos(3, 1));
            expect(m.current_line == 2);

            // No cursor movement as we're already at end of file
            v.cursor_down();
            expect(v.cur == rawterm::Pos(3, 1));
            expect(m.current_line == 2);
        };
    };

    skip / "cursor_right"_test = [] {
        Controller c;
        auto v = View(&c, rawterm::Pos(24, 80));
        auto m = Model(
            open_file("tests/fixture/test_file_1.txt").value(), "tests/fixture/test_file_1.txt");
        v.add_model(&m);

        should("Already at right-most position in view") = [&] {
            v.cur.move({1, 80});
            expect(v.cur == rawterm::Pos(1, 80));
            v.cursor_right();
            expect(v.cur == rawterm::Pos(1, 80));
        };

        should("Already at right-most position in line") = [&] {
            v.cur.move({1, 1});
            for (int i = 1; i <= 100; i++) {
                v.cursor_right();
            }

            expect(v.cur == rawterm::Pos(1, 18));
            expect(m.current_char == 17);
        };

        should("Move right") = [&] {
            v.cur.move({1, 1});
            v.cursor_right();
            expect(v.cur == rawterm::Pos(1, 2));
        };
    };
};
