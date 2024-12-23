#include "controller.h"
#include "ut/ut.hpp"

boost::ut::suite<"Controller"> controller_suite = [] {
    using namespace boost::ut;

    skip / "Constructor"_test = [] {};
    skip / "set_mode"_test = [] {};
    skip / "get_mode"_test = [] {};
    skip / "create_view"_test = [] {};
    skip / "start_action_engine"_test = [] {};
};
