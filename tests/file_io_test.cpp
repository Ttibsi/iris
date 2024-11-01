#include "file_io.h"

#include <filesystem>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include "gapvector.h"

TEST_CASE("open_file", "[FILESYSTEM]") {
    Gapvector<> gv = Gapvector<>(
        "This is some text\r\n"
        "    here is a newline and tab\r\n"
        "and another newline");

    auto actual = open_file("tests/fixture/test_file_1.txt");

    REQUIRE(actual.has_value() == true);
    REQUIRE(actual.value() == gv);
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
        r.stderr = "mv: missing file operand\r\nTry 'mv --help' for more information.";
        r.retcode = 256;

        REQUIRE(out.value().stdout == r.stdout);
        REQUIRE(out.value().stderr == r.stderr);
        REQUIRE(out.value().retcode == r.retcode);
    }
}

TEST_CASE("write_to_file", "[FILESYSTEM]") {
    Gapvector<> gv = Gapvector<>("Hello world");
    size_t bytes = write_to_file("save_test_file.txt", gv);
    REQUIRE(bytes == gv.size());

    auto contents = open_file("save_test_file.txt");
    REQUIRE(contents.has_value());
    REQUIRE(contents.value() == gv);

    std::filesystem::remove("save_test_file.txt");
}
