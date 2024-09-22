#include "file_io.h"

#include <filesystem>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("open_file", "[FILESYSTEM]") {
    std::vector<char> expected = {'T',  'h',  'i', 's', ' ', 'i',  's',  ' ', 's', 'o',  'm', 'e',
                                  ' ',  't',  'e', 'x', 't', '\r', '\n', ' ', ' ', ' ',  ' ', 'h',
                                  'e',  'r',  'e', ' ', 'i', 's',  ' ',  'a', ' ', 'n',  'e', 'w',
                                  'l',  'i',  'n', 'e', ' ', 'a',  'n',  'd', ' ', 't',  'a', 'b',
                                  '\r', '\n', 'a', 'n', 'd', ' ',  'a',  'n', 'o', 't',  'h', 'e',
                                  'r',  ' ',  'n', 'e', 'w', 'l',  'i',  'n', 'e', '\r', '\n'};

    auto actual = open_file("tests/fixture/test_file_1.txt");

    REQUIRE(actual.has_value() == true);
    REQUIRE(actual.value() == expected);
}

TEST_CASE("shell_exec", "[FILESYSTEM]") {
    SECTION("Standard accept") {
        Response expected = {"hi", "", 0};
        auto out = shell_exec("echo -n 'hi'", true);

        REQUIRE(out.has_value());
        REQUIRE(out.value().stdout == expected.stdout);
        REQUIRE(out.value().stderr == expected.stderr);
        REQUIRE(out.value().retcode == expected.retcode);
    }

    SECTION("No Response needed") {
        auto out = shell_exec("echo 'hi'", false);
        REQUIRE_FALSE(out.has_value());
    }

    SECTION("Executed command failed") {
        auto out = shell_exec("mv", true);
        Response r;
        r.stderr = "mv: missing file operand\r\nTry 'mv --help' for more information.\r\n";
        r.retcode = 256;

        REQUIRE(out.value().stdout == r.stdout);
        REQUIRE(out.value().stderr == r.stderr);
        REQUIRE(out.value().retcode == r.retcode);
    }
}

TEST_CASE("write_to_file", "[FILESYSTEM]") {
    std::vector<char> expected = {
        'H', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd',
    };

    size_t bytes = write_to_file("save_test_file.txt", Gapvector(expected.begin(), expected.end()));
    REQUIRE(bytes == expected.size());

    auto contents = open_file("save_test_file.txt");
    REQUIRE(contents.has_value());
    REQUIRE(contents.value() == expected);

    std::filesystem::remove("save_test_file.txt");
}
