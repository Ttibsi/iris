#include "model.h"

#include <catch2/catch_test_macros.hpp>

#include <vector>

TEST_CASE("Constructor", "[MODEL]") {
    auto m = Model();
    REQUIRE(m.line_count == 0);
    REQUIRE(m.buf.size() == 0);
}

TEST_CASE("Constructor_with_values", "[MODEL]") {
    std::vector<char> expected = {
        'T',  'h', 'i', 's', ' ', 'i', 's', ' ', 's', 'o', 'm', 'e', ' ', 't',  'e', 'x', 't',
        '\n', ' ', ' ', ' ', ' ', 'h', 'e', 'r', 'e', ' ', 'i', 's', ' ', 'a',  ' ', 'n', 'e',
        'w',  'l', 'i', 'n', 'e', ' ', 'a', 'n', 'd', ' ', 't', 'a', 'b', '\n', 'a', 'n', 'd',
        ' ',  'a', 'n', 'o', 't', 'h', 'e', 'r', ' ', 'n', 'e', 'w', 'l', 'i',  'n', 'e', '\n'};

    auto m = Model(expected, "test_file.txt");

    REQUIRE(m.line_count == 3);
    REQUIRE(m.buf.size() == expected.size());
    REQUIRE(m.file_name == "test_file.txt");
}

TEST_CASE("get_abs_pos", "[MODEL]") {
    std::vector<char> expected = {
        'T',  'h', 'i', 's', ' ', 'i', 's', ' ', 's', 'o', 'm', 'e', ' ', 't',  'e', 'x', 't',
        '\n', ' ', ' ', ' ', ' ', 'h', 'e', 'r', 'e', ' ', 'i', 's', ' ', 'a',  ' ', 'n', 'e',
        'w',  'l', 'i', 'n', 'e', ' ', 'a', 'n', 'd', ' ', 't', 'a', 'b', '\n', 'a', 'n', 'd',
        ' ',  'a', 'n', 'o', 't', 'h', 'e', 'r', ' ', 'n', 'e', 'w', 'l', 'i',  'n', 'e', '\n'};

    auto m = Model(expected, "test_file.txt");

    REQUIRE(m.get_abs_pos() == 0);
    m.current_line++;
    REQUIRE(m.get_abs_pos() == 18);
    m.current_char_in_line = 5;
    REQUIRE(m.get_abs_pos() == 22);
}

TEST_CASE("get_current_char", "[MODEL]") {
    std::vector<char> expected = {
        'T',  'h', 'i', 's', ' ', 'i', 's', ' ', 's', 'o', 'm', 'e', ' ', 't',  'e', 'x', 't',
        '\n', ' ', ' ', ' ', ' ', 'h', 'e', 'r', 'e', ' ', 'i', 's', ' ', 'a',  ' ', 'n', 'e',
        'w',  'l', 'i', 'n', 'e', ' ', 'a', 'n', 'd', ' ', 't', 'a', 'b', '\n', 'a', 'n', 'd',
        ' ',  'a', 'n', 'o', 't', 'h', 'e', 'r', ' ', 'n', 'e', 'w', 'l', 'i',  'n', 'e', '\n'};

    auto m = Model(expected, "test_file.txt");

    REQUIRE(m.get_current_char() == 'T');
    m.current_line++;
    REQUIRE(m.get_current_char() == ' ');
    m.current_char_in_line = 5;
    REQUIRE(m.get_current_char() == 'h');
}

TEST_CASE("get_next_char", "[MODEL]") {
    std::vector<char> expected = {
        'T',  'h', 'i', 's', ' ', 'i', 's', ' ', 's', 'o', 'm', 'e', ' ', 't',  'e', 'x', 't',
        '\n', ' ', ' ', ' ', ' ', 'h', 'e', 'r', 'e', ' ', 'i', 's', ' ', 'a',  ' ', 'n', 'e',
        'w',  'l', 'i', 'n', 'e', ' ', 'a', 'n', 'd', ' ', 't', 'a', 'b', '\n', 'a', 'n', 'd',
        ' ',  'a', 'n', 'o', 't', 'h', 'e', 'r', ' ', 'n', 'e', 'w', 'l', 'i',  'n', 'e', '\n'};

    auto m = Model(expected, "test_file.txt");

    REQUIRE(m.get_next_char() == 'h');
    m.current_line++;
    REQUIRE(m.get_next_char() == ' ');
    m.current_char_in_line = 5;
    REQUIRE(m.get_next_char() == 'e');
}

TEST_CASE("insert_char", "[MODEL]") {
    SKIP("Untested");
}

TEST_CASE("save_file", "[MODEL]") {
    SKIP("Untested");
}
