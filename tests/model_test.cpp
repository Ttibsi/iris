#include "model.h"

#include "ut/ut.hpp"

boost::ut::suite<"Model"> model_suite = [] {
    using namespace boost::ut;

    "Constructor"_test = [] {
        auto m = Model();

        expect(m.filename == "");
        expect(m.buf == TwinArray<char>());
    };

    "Constructor with params"_test = [] {
        auto expected_buf = TwinArray<char>("This is a test");
        auto m = Model(expected_buf, "");

        expect(m.filename == "");
        expect(m.buf.to_str() == "This is a test");
    };
};
