#include "model.h"

#include "text_io.h"
#include "ut/ut.hpp"

boost::ut::suite<"Model"> model_suite = [] {
    using namespace boost::ut;

    "Constructor"_test = [] {
        auto m = Model(32);

        expect(m.filename == "");
        expect(m.buf.capacity() == 32);
    };

    "Constructor with params"_test = [] {
        lines_t expected_buf = {"foo", "bar", "baz"};
        auto m = Model(expected_buf, "");

        expect(m.filename == "");
        expect(m.buf.at(0) == "foo");
    };

    "backspace"_test = [] {
        should("backspace a newline") = [] {
            lines_t v = {"foo", "bar", "baz"};
            auto m = Model(v, "");
            m.current_line = 1;
            std::ignore = m.backspace();

            expect(m.buf.size() == 2);
            expect(m.buf.at(0) == "foobar");
        };

        should("backspace a char") = [] {
            lines_t v = {"foo", "bar", "baz"};
            auto m = Model(v, "");
            m.current_line = 1;
            m.current_char = 2;
            std::ignore = m.backspace();

            expect(m.buf.size() == 3);
            expect(m.buf.at(1) == "br") << m.buf.at(1);
            expect(m.buf.at(1).size() == 2);
        };

        should("backspace the last char") = [] {
            lines_t v = {"foo", "bar", "baz"};
            auto m = Model(v, "");
            m.current_line = 1;
            m.current_char = 3;
            std::ignore = m.backspace();

            expect(m.buf.size() == 3);
            expect(m.buf.at(1) == "ba");
            expect(m.buf.at(1).size() == 2);
        };
    };

    "newline"_test = [] {
        should("At end of line") = [] {
            lines_t v = {"foo", "bar", "baz"};
            auto m = Model(v, "");

            m.current_char = 3;
            const unsigned int prev_line_len = m.newline();

            expect(prev_line_len == v.at(0).size());
            expect(m.buf.size() == 4);
            expect(m.buf.at(0).size() == 3);
            expect(m.buf.at(1).size() == 0);
        };

        should("At mid of line") = [] {
            lines_t v = {"foo", "bar", "baz"};
            auto m = Model(v, "");

            m.current_char = 1;
            const int prev_line_len = m.newline();

            expect(prev_line_len == 1);
            expect(m.buf.size() == 4);
            expect(m.buf.at(0).size() == 1);
            expect(m.buf.at(0) == "f");
            expect(m.buf.at(1).size() == 2);
            expect(m.buf.at(1) == "oo");
        };

        should("At start of line") = [] {
            lines_t v = {"foo", "bar", "baz"};
            auto m = Model(v, "");

            const unsigned int prev_line_len = m.newline();

            expect(prev_line_len == 0);
            expect(m.buf.size() == 4);

            expect(m.buf.at(0).size() == 0);
            expect(m.buf.at(0) == "");

            expect(m.buf.at(1).size() == v.at(0).size());
            expect(m.buf.at(1) == "foo");
        };
    };

    "insert"_test = [] {
        should("Insert at start of line") = [] {
            lines_t v = {"foo", "bar", "baz"};
            auto m = Model(v, "");

            m.insert('x');
            expect(m.buf.at(0) == "xfoo");
            expect(m.buf.at(0).size() == 4);
            expect(m.buf.size() == 3);

            m.insert('y');
            expect(m.buf.at(0) == "xyfoo");
            expect(m.buf.at(0).size() == 5);
            expect(m.buf.size() == 3);
        };

        should("Insert in mid of line") = [] {
            lines_t v = {"foo", "bar", "baz"};
            auto m = Model(v, "");

            m.current_char++;
            m.insert('x');

            expect(m.buf.at(0) == "fxoo");
            expect(m.buf.at(0).size() == 4);
            expect(m.buf.size() == 3);
        };

        should("Insert at end of line") = [] {
            lines_t v = {"foo", "bar", "baz"};
            auto m = Model(v, "");

            m.current_char = v.at(0).size();
            m.insert('x');

            expect(m.buf.at(0) == "foox");
            expect(m.buf.at(0).size() == 4);
            expect(m.buf.size() == 3);
        };
    };
};
