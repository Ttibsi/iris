#include "text_io.h"

#include "ut/ut.hpp"

boost::ut::suite<"File IO"> file_io_suite = [] {
    using namespace boost::ut;

    "open_file"_test = [] {
        lines_t expected = {
            "This is some text\r\n", "\there is a newline and a tab\r\n", "and another newline"};

        opt_lines_t actual = open_file("tests/fixture/test_file_1.txt");

        expect(actual.has_value() == true);
        expect(actual.value() == expected);
        expect(actual.value().at(1) == expected.at(1));
    };

    skip / "lines"_test = [] {};
};
