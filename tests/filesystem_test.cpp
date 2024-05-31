#include "filesystem.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("open_file", "[FILESYSTEM]") {
    std::vector<char> expected = {
        'T',  'h', 'i', 's', ' ', 'i', 's', ' ', 's', 'o', 'm', 'e', ' ', 't',  'e', 'x', 't',
        '\n', ' ', ' ', ' ', ' ', 'h', 'e', 'r', 'e', ' ', 'i', 's', ' ', 'a',  ' ', 'n', 'e',
        'w',  'l', 'i', 'n', 'e', ' ', 'a', 'n', 'd', ' ', 't', 'a', 'b', '\n', 'a', 'n', 'd',
        ' ',  'a', 'n', 'o', 't', 'h', 'e', 'r', ' ', 'n', 'e', 'w', 'l', 'i',  'n', 'e', '\n'};

    auto actual = open_file("tests/fixture/test_file_1.txt");

    REQUIRE(actual.has_value() == true);
    REQUIRE(actual.value() == expected);
}
