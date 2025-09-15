#include "text_io.h"

#include <catch2/catch_test_macros.hpp>

#include "model.h"

TEST_CASE("open_file", "[textio]") {
    SECTION("Normal file") {
        lines_t expected = {
            "This is some text", "    here is a newline and a tab", "and another newline"};

        opt_lines_t actual = open_file("tests/fixture/test_file_1.txt");

        REQUIRE(actual.has_value() == true);
        REQUIRE(actual.value() == expected);
        REQUIRE(actual.value().at(1) == expected.at(1));
    }

    SECTION("One line, no newlines") {
        opt_lines_t actual = open_file("tests/fixture/no_newline_file.txt");
        REQUIRE(actual.has_value() == true);
        REQUIRE(actual.value().size() == 1);
        REQUIRE(actual.value().at(0) == "hello");
    }
}

TEST_CASE("get_file_size", "[textio]") {
    REQUIRE(get_file_size("tests/fixture/test_file_1.txt") == 70);
    REQUIRE(get_file_size("tests/fixture/lorem_ipsum.txt") == 5676);
}

TEST_CASE("write_to_file", "[textio]") {
    lines_t expected_buf = {"foo", "bar", "baz"};
    auto m = Model(expected_buf, "tests/fixture/temp_file.txt");
    const WriteData data = write_to_file(&m);
    REQUIRE(data.valid == true);
    REQUIRE(data.bytes == 12);
    REQUIRE(data.lines == 3);
    REQUIRE(m.unsaved == false);
}

TEST_CASE("lines", "[textio]") {
    std::string s = "foo\nbar\r\nbaz";
    auto actual = lines(s);

    REQUIRE(actual.size() == 3);
    REQUIRE(actual.at(0) == "foo");
    REQUIRE(actual.at(1) == "bar");
    REQUIRE(actual.at(2) == "baz");
}

TEST_CASE("is_letter", "[textio]") {
    REQUIRE(is_letter('a'));
    REQUIRE(is_letter('L'));
    REQUIRE_FALSE(is_letter('_'));
    REQUIRE_FALSE(is_letter(':'));
}

TEST_CASE("check_filename", "[textio]") {
    std::string err_text = "Iris currently does not support tab-delineated files";

    REQUIRE(check_filename("Makefile") == err_text);
    REQUIRE(check_filename("makefile") == err_text);
    REQUIRE(check_filename("t.go") == err_text);
    REQUIRE(check_filename("t.py") == "");
}

TEST_CASE("file_exists", "[textio]") {
    REQUIRE(file_exists("tests/text_io_test.cpp"));
    REQUIRE(!(file_exists("tests/something_else.cpp")));
}

TEST_CASE("shell_exec", "[textio]") {
    SECTION("Successful command (stdout)") {
        REQUIRE(shell_exec("echo \"hello\"").has_value());
        REQUIRE(shell_exec("echo \"hello\"").value().out == "hello");
        REQUIRE(shell_exec("echo \"hello\"").value().err == "");
    }

    SECTION("Failing command (stderr)") {
        std::string err_msg = "cat: does_not_exist: No such file or directory\n";
        REQUIRE(shell_exec("cat does_not_exist").has_value());
        REQUIRE(shell_exec("cat does_not_exist").value().out == "");
        REQUIRE(shell_exec("cat does_not_exist").value().err == err_msg);
    }
}

TEST_CASE("split_by", "[textio]") {
    std::string s = "This is some text|divided by a pipe";
    auto ret = split_by(s, '|');
    REQUIRE(ret.size() == 2);
    REQUIRE(ret.at(0) == "This is some text");
    REQUIRE(ret.at(1) == "divided by a pipe");
}

TEST_CASE("first_non_whitespace", "[textio]") {
    REQUIRE(first_non_whitespace("    test") == 4);
    REQUIRE(first_non_whitespace("test") == 0);
}
