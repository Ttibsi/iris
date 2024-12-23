#include "ut/ut.hpp"
#include "view.h"

boost::ut::suite<"View"> view_suite = [] {
    using namespace boost::ut;

    skip / "Constructor"_test = [] {};
    skip / "add_model"_test = [] {};
    skip / "get_active_model"_test = [] {};
    skip / "render_screen"_test = [] {};
    skip / "generate_tab_bar"_test = [] {};
    skip / "draw_tab_bar"_test = [] {};
    skip / "render_status_bar"_test = [] {};
};
