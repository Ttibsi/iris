#include "model.h"

#include <filesystem>
#include <optional>

#include <catch2/catch_test_macros.hpp>

#include "file_io.h"
#include "gapvector.h"

TEST_CASE("Constructor", "[MODEL]") {
    auto m = Model();
    REQUIRE(m.line_count == 0);
    REQUIRE(m.buf.size() == 0);
}

TEST_CASE("Constructor_with_values", "[MODEL]") {
    std::string expected =
        "This is some text\n"
        "    here is a newline and tab\n"
        "and another newline\n";
    auto m = Model(Gapvector<>(expected), "test_file.txt");

    REQUIRE(m.line_count == 3);
    REQUIRE(m.buf.size() == expected.size());
    REQUIRE(m.file_name == "test_file.txt");
}

TEST_CASE("get_abs_pos", "[MODEL]") {
    std::string expected =
        "This is some text\r\n"
        "    here is a newline and tab\r\n"
        "and another newline";
    auto m = Model(Gapvector<>(expected), "test_file.txt");

    REQUIRE(m.get_abs_pos() == 0);
    m.current_line++;
    REQUIRE(m.get_abs_pos() == 19);
    m.current_char_in_line = 5;
    REQUIRE(m.get_abs_pos() == 23);
}

TEST_CASE("get_current_char", "[MODEL]") {
    std::string expected =
        "This is some text\n"
        "    here is a newline and tab\n"
        "and another newline\n";
    auto m = Model(Gapvector<>(expected), "test_file.txt");

    REQUIRE(m.get_current_char() == 'T');
    m.current_line++;
    REQUIRE(m.get_current_char() == ' ');
    m.current_char_in_line = 5;
    REQUIRE(m.get_current_char() == 'h');
}

TEST_CASE("get_next_char", "[MODEL]") {
    std::string expected =
        "This is some text\n"
        "    here is a newline and tab\n"
        "and another newline\n";
    auto m = Model(Gapvector<>(expected), "test_file.txt");

    REQUIRE(m.get_next_char() == 'h');
    m.current_line++;
    REQUIRE(m.get_next_char() == ' ');
    m.current_char_in_line = 5;
    REQUIRE(m.get_next_char() == 'e');
}

TEST_CASE("get_current_line", "[MODEL]") {
    std::string expected = "#include <iostream>\r\n\r\nint main";
    auto m = Model(Gapvector<>(expected), "test_file.txt");

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
    std::string expected =
        "This is some text\n"
        "    here is a newline and tab\n"
        "and another newline\n";
    auto m = Model(Gapvector<>(expected), "test_file.txt");

    REQUIRE(m.get_current_char() == 'T');
    m.insert_char('a');
    REQUIRE(m.get_current_char() == 'a');
}

TEST_CASE("save_file", "[MODEL]") {
    Gapvector<> expected = Gapvector("Hello world");
    auto m = Model(expected, "save_test_file.txt");

    m.save_file();
    std::optional<Gapvector<>> contents = open_file("save_test_file.txt");
    REQUIRE(contents.has_value());
    REQUIRE(contents.value() == expected);

    std::filesystem::remove("save_test_file.txt");
}
