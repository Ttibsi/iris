#include "file_io.h"
#include "ut/ut.hpp"

boost::ut::suite<"File IO"> file_io_suite = [] {
    using namespace boost::ut;

    skip / "open_file"_test = [] {};
};
