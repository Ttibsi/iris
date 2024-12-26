#include "constants.h"
#include "controller.h"
#include "file_io.h"
#include "ut/ut.hpp"
#include "utils.h"
#include "view.h"
#include <rawterm/text.h>

boost::ut::suite<"View"> view_suite = [] {
    using namespace boost::ut;

    "Constructor"_test = [] {
        Controller c;
        auto v = View(&c, rawterm::Pos(24, 80));

        expect(v.viewable_models.capacity() == 8);
        expect(v.view_size.horizontal == 80);
        expect(v.view_size.vertical == 24);
    };

    "add_model"_test = [] {
        Controller c;
        std::string raw = "This is some text\n"
                          "    here is a newline and tab\n"
                          "and another newline\n";

        auto v = View(&c, rawterm::Pos(24, 80));
        auto m = Model(TwinArray<char>(raw), "test_file.txt");
        v.add_model(&m);

        expect(v.active_model == 1);
        expect(v.get_active_model()->filename == "test_file.txt");

        if (LINE_NUMBERS) {
            expect(v.line_number_offset == 2);
        }
    };

    "get_active_model"_test = [] {
        Controller c;
        std::string raw = "This is some text\n"
                          "    here is a newline and tab\n"
                          "and another newline\n";

        auto v = View(&c, rawterm::Pos(24, 80));
        auto m = Model(TwinArray<char>(raw), "test_file.txt");
        v.add_model(&m);

        expect(v.get_active_model() == &m);
    };

    "render_screen"_test = [] {
        Controller c;
        auto v = View(&c, rawterm::Pos(24, 80));

        should("Render single text file") = [=]() mutable {
            auto m = Model(open_file("tests/fixture/test_file_1.txt").value(),
                           "tests/fixture/test_file_1.txt");
            v.add_model(&m);

            auto buffer = lines(v.render_screen());

            expect(buffer.size() == 23);
            expect(rawterm::raw_at(buffer.at(0), 5) == 'T');
            expect(rawterm::raw_at(buffer.at(1), 1) == '2');
        };

        should("Truncated line") = [=]() mutable {
            auto m = Model();
            v.add_model(&m);

            for (int i = 0; i < 80; i++) {
                m.buf.push('_');
            }

            auto buffer = lines(v.render_screen());

            expect(buffer.size() == 23);
            expect(rawterm::raw_size(buffer.at(0)) == 83);
            expect(buffer.at(0).substr(buffer.at(0).size() - 3, 2) == "\u00bb");
        };
    };

    "generate_tab_bar"_test = [] {
        Controller c;
        std::string raw = "This is some text\n"
                          "    here is a newline and tab\n"
                          "and another newline\n";

        auto v = View(&c, rawterm::Pos(24, 80));
        auto m = Model(TwinArray<char>(raw), "test_file.txt");

        v.add_model(&m);
        auto m2 = Model(open_file("tests/fixture/test_file_1.txt").value(),
                        "tests/fixture/test_file_1.txt");
        v.add_model(&m2);

        std::string ret = v.render_tab_bar();
        std::string expected =
            "| test_file.txt | \x1B[7mtests/fixture/test_file_1.txt\x1B[27m | "
            "\n";

        expect(expected == ret);
    };

    "render_status_bar"_test = [] {
        Controller c;
        auto v = View(&c, rawterm::Pos(24, 80));

        auto m = Model(open_file("tests/fixture/test_file_1.txt").value(),
                       "tests/fixture/test_file_1.txt");

        v.add_model(&m);
        std::string ret = v.render_status_bar();

        expect(ret.find("READ") != std::string::npos);
        expect(ret.find("test_file_1.txt") != std::string::npos);
        expect(ret.find("1:1") != std::string::npos);
        expect(rawterm::raw_size(ret) == 80);
    };
};
