#include "file_io.h"
#include "twin_array.h"

#include "ut/ut.hpp"

boost::ut::suite<"File IO"> file_io_suite = [] {
    using namespace boost::ut;

    "open_file"_test = [] {
        std::string contents = "This is some text\r\n"
                               "\there is a newline and a tab\r\n"
                               "and another newline";

        auto buf = TwinArray<char>(contents);
        for (unsigned int i = 0; i <= buf.size(); i++) {
            buf.move_left();
        }
        auto actual = open_file("tests/fixture/test_file_1.txt");

        expect(actual.has_value() == true);
        expect(actual.value() == buf);
        expect(actual.value().curr_line_index() == 1);
        expect(actual.value().curr_char_index() == 1)
            << actual.value().curr_char_index();
        expect(actual.value().to_str() == contents);
    };
};
