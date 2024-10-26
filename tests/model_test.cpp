#include "model.h"

#include <filesystem>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "file_io.h"

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
        'T',  'h', 'i', 's', ' ', 'i', 's', ' ', 's', 'o', 'm', 'e', ' ',  't', 'e', 'x', 't', '\r',
        '\n', ' ', ' ', ' ', ' ', 'h', 'e', 'r', 'e', ' ', 'i', 's', ' ',  'a', ' ', 'n', 'e', 'w',
        'l',  'i', 'n', 'e', ' ', 'a', 'n', 'd', ' ', 't', 'a', 'b', '\n', 'a', 'n', 'd', ' ', 'a',
        'n',  'o', 't', 'h', 'e', 'r', ' ', 'n', 'e', 'w', 'l', 'i', 'n',  'e', '\n'};

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

TEST_CASE("get_current_line", "[MODEL]") {
    std::vector<char> expected = {
        '#', 'i', 'n', 'c',  'l',  'u',  'd',  'e', ' ', '<', 'i', 'o', 's', 't', 'r', 'e',
        'a', 'm', '>', '\r', '\n', '\r', '\n', 'i', 'n', 't', ' ', 'm', 'a', 'i', 'n',
    };

    auto m = Model(expected, "test_file.txt");

    SECTION("At the start of buffer") {
        REQUIRE(m.current_line == 1);
        REQUIRE(m.current_char_in_line == 1);
        REQUIRE(m.get_current_line() == "#include <iostream>");
    }

    SECTION("At the end of the buffer") {
        m.current_line++;
        m.current_line++;
        m.current_char_in_line = 8;
        REQUIRE(m.current_line == 3);
        REQUIRE(m.current_char_in_line == 8);
        REQUIRE(m.get_current_line() == "int main");
    }
}

TEST_CASE("insert_char", "[MODEL]") {
    std::vector<char> expected = {
        'T',  'h', 'i', 's', ' ', 'i', 's', ' ', 's', 'o', 'm', 'e', ' ', 't',  'e', 'x', 't',
        '\n', ' ', ' ', ' ', ' ', 'h', 'e', 'r', 'e', ' ', 'i', 's', ' ', 'a',  ' ', 'n', 'e',
        'w',  'l', 'i', 'n', 'e', ' ', 'a', 'n', 'd', ' ', 't', 'a', 'b', '\n', 'a', 'n', 'd',
        ' ',  'a', 'n', 'o', 't', 'h', 'e', 'r', ' ', 'n', 'e', 'w', 'l', 'i',  'n', 'e', '\n'};

    auto m = Model(expected, "test_file.txt");
    REQUIRE(m.get_current_char() == 'T');
    m.insert_char('a');
    REQUIRE(m.get_current_char() == 'a');
}

TEST_CASE("save_file", "[MODEL]") {
    std::vector<char> expected = {
        'H', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd',
    };

    auto m = Model(expected, "save_test_file.txt");

    m.save_file();
    auto contents = open_file("save_test_file.txt");
    REQUIRE(contents.has_value());
    REQUIRE(contents.value() == expected);

    std::filesystem::remove("save_test_file.txt");
}
