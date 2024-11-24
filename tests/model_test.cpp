#include "model.h"

#include <filesystem>
#include <optional>

#include <catch2/catch_test_macros.hpp>

#include "file_io.h"
#include "gapbuffer.h"

TEST_CASE("Constructor", "[MODEL]") {
    auto m = Model();
    REQUIRE(m.buf.line_count() == 0);
    REQUIRE(m.buf.size() == 0);
}

TEST_CASE("Constructor_with_values", "[MODEL]") {
    std::string expected =
        "This is some text\n"
        "    here is a newline and tab\n"
        "and another newline\n";
    auto m = Model(Gapbuffer(expected), "test_file.txt");

    REQUIRE(m.buf.line_count() == 3);
    REQUIRE(m.buf.size() == expected.size());
    REQUIRE(m.file_name == "test_file.txt");
}

TEST_CASE("get_abs_pos", "[MODEL]") {
    std::string expected =
        "This is some text\r\n"
        "    here is a newline and tab\r\n"
        "and another newline";
    auto m = Model(Gapbuffer(expected), "test_file.txt");

    REQUIRE(m.get_abs_pos() == 0);
    m.current_line++;
    REQUIRE(m.get_abs_pos() == 19);
    m.current_char_in_line = 5;
    REQUIRE(m.get_abs_pos() == 23);
}

TEST_CASE("get_current_char", "[MODEL]") {
    std::string expected =
        "This is some text\r\n"
        "    here is a newline and tab\r\n"
        "and another newline";
    auto m = Model(Gapbuffer(expected), "test_file.txt");

    for (unsigned int i = 0; i <= m.buf.size(); i++) {
        m.buf.retreat();
    }

    REQUIRE(m.get_current_char() == 'T');
    REQUIRE(m.buf.pos() == 0);

    m.buf.advance();
    REQUIRE(m.buf.pos() == 1);
    REQUIRE(m.get_current_char() == 'h');
    m.line_down();
    REQUIRE(m.get_current_char() == ' ');
}

TEST_CASE("get_next_char", "[MODEL]") {
    std::string expected =
        "This is some text\n"
        "    here is a newline and tab\n"
        "and another newline\n";
    auto m = Model(Gapbuffer(expected), "test_file.txt");

    for (unsigned int i = 0; i <= m.buf.size(); i++) {
        m.buf.retreat();
    }

    REQUIRE(m.get_next_char() == 'h');
    m.buf.advance();
    REQUIRE(m.get_next_char() == 'i');
    m.line_down();
    REQUIRE(m.get_next_char() == ' ');
}

TEST_CASE("get_current_line", "[MODEL]") {
    std::string expected = "#include <iostream>\r\n\r\nint main";
    auto m = Model(Gapbuffer(expected), "test_file.txt");

    for (unsigned int i = 0; i <= m.buf.size(); i++) {
        m.buf.retreat();
    }

    SECTION("At the start of buffer") {
        REQUIRE(m.current_line == 1);
        REQUIRE(m.current_char_in_line == 1);
        REQUIRE(m.get_current_line() == "#include <iostream>\r\n");
    }

    SECTION("At the end of the buffer") {
        m.current_line++;
        m.current_line++;
        m.current_char_in_line = 8;
        for (unsigned int i = 0; i <= m.buf.size() - 1; i++) {
            m.buf.advance();
        }

        REQUIRE(m.current_line == 3);
        REQUIRE(m.current_char_in_line == 8);
        REQUIRE(m.get_current_line() == "int main");
    }
}

TEST_CASE("insert_char", "[MODEL]") {
    std::string expected =
        "This is some text\n"
        "    here is a newline and tab\n"
        "and another newline\n";
    auto m = Model(Gapbuffer(expected), "test_file.txt");
    for (unsigned int i = 0; i <= m.buf.size(); i++) {
        m.buf.retreat();
    }

    REQUIRE(m.buf.at(0) == 'T');
    REQUIRE(m.get_current_char() == 'T');

    m.insert_char('a');
    REQUIRE(m.get_current_char() == 'T');
    REQUIRE(m.buf.at(0) == 'a');
}

TEST_CASE("save_file", "[MODEL]") {
    Gapbuffer expected = Gapbuffer("Hello world");
    auto m = Model(expected, "save_test_file.txt");

    m.save_file();
    std::optional<Gapbuffer> contents = open_file("save_test_file.txt");
    REQUIRE(contents.has_value());
    REQUIRE(contents.value() == expected);

    std::filesystem::remove("save_test_file.txt");
}
