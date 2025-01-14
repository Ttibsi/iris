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

    skip / "backspace"_test = [] {};
    skip / "newline"_test = [] {};
    skip / "insert"_test = [] {};
};
