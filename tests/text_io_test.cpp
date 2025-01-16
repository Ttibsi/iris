#include "text_io.h"

#include "ut/ut.hpp"

boost::ut::suite<"Text IO"> file_io_suite = [] {
    using namespace boost::ut;

    "open_file"_test = [] {
        lines_t expected = {
            "This is some text", "    here is a newline and a tab", "and another newline"};

        opt_lines_t actual = open_file("tests/fixture/test_file_1.txt");

        expect(actual.has_value() == true);
        expect(actual.value() == expected);
        expect(actual.value().at(1) == expected.at(1));
    };

    "lines"_test = [] {
        std::string s = "foo\nbar\r\nbaz";
        auto actual = lines(s);

        expect(actual.size() == 3);
        expect(actual.at(0) == "foo");
        expect(actual.at(1) == "bar");
        expect(actual.at(2) == "baz");
    };
};
