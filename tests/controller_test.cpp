#include "controller.h"

#include "ut/ut.hpp"

boost::ut::suite<"Controller"> controller_suite = [] {
    using namespace boost::ut;

    "Constructor"_test = [] {
        Controller c;
        expect(c.models.capacity() == 8);
    };

    "set_mode"_test = [] {
        Controller c;

        c.set_mode(Mode::Write);
        expect(c.mode == Mode::Write);

        c.set_mode(Mode::Read);
        expect(c.mode == Mode::Read);
    };

    "get_mode"_test = [] {
        Controller c;

        expect(c.get_mode() == "READ");
        c.set_mode(Mode::Write);
        expect(c.get_mode() == "WRITE");
    };

    "create_view"_test = [] {
        should("View with file") = [=] {
            Controller c;
            c.create_view("tests/fixture/test_file_1.txt");

            expect(c.models.size() == 1);
            expect(*c.models.at(0).buf.at(0).begin() == 'T');
        };

        should("Empty view") = [=] {
            Controller c;
            c.create_view("");

            expect(c.models.size() == 1);
            expect(c.models.at(0).buf.size() == 0);
        };
    };

    skip / "start_action_engine"_test = [] {};
};
