#include "constants.h"
#include "text_manip.h"

#include "gtest/gtest.h"
#include <filesystem>

TEST(textManipSuite, filterWhitespace) {
    std::vector<std::string> fixture = {
        "This is a normal string", "\tThis string starts with a tab",
        "    This string starts with 4 spaces"
    };

    std::vector<std::string> expected = {
        "This is a normal string",
        std::string(TABSTOP, ' ') + "This string starts with a tab",
        "    This string starts with 4 spaces"
    };

    auto out = filter_whitespace(fixture);
    EXPECT_EQ(out, expected);
}

TEST(textManipSuite, shellExec) {
    Response r = { "hi", "", 0 };
    Response out = shell_exec("echo 'hi'", true);
    EXPECT_EQ(out.stdout, r.stdout);
    EXPECT_EQ(out.stderr, r.stderr);
    EXPECT_EQ(out.retcode, r.retcode);

    out = shell_exec("echo 'hi'", false);
    r = { "", "", -1 };
    EXPECT_EQ(out.stdout, r.stdout);
    EXPECT_EQ(out.stderr, r.stderr);
    EXPECT_EQ(out.retcode, r.retcode);

#ifdef __APPLE__
    out = shell_exec("mv", true);
    r.stderr = "usage: mv [-f | -i | -n] [-hv] source target"
               "       mv [-f | -i | -n] [-v] source ... directory";
    r.retcode = 16384;

    EXPECT_EQ(out.stdout, r.stdout);
    EXPECT_EQ(out.stderr, r.stderr);
    EXPECT_EQ(out.retcode, r.retcode);
#else
    out = shell_exec("mv", true);
    r.stderr = "mv: missing file operandTry 'mv "
               "--help' for more information.";
    r.retcode = 256;

    EXPECT_EQ(out.stdout, r.stdout);
    EXPECT_EQ(out.stderr, r.stderr);
    EXPECT_EQ(out.retcode, r.retcode);
#endif
}

TEST(textManipSuite, countChar) {
    EXPECT_EQ(count_char("Hello world", 'l'), 3);
    EXPECT_EQ(count_char("\ttab test", '\t'), 1);
}

TEST(textManipSuite, lineSize) {
    EXPECT_EQ(line_size("foo"), 3);
    EXPECT_EQ(line_size("\tfoo"), 7);
    EXPECT_EQ(line_size("\t\tfoo"), 11);
}

TEST(textManipSuite, findNextWord) {
    std::string s = "This is some text";
    EXPECT_EQ(find_next_whitespace(s, 6), 7) << "char: '" << s[6] << "'";
    EXPECT_EQ(find_next_whitespace(s, 14), 17) << "char: '" << s[14] << "'";
}

TEST(textManipSuite, findPrevWord) {
    std::string s = "This is some text";
    EXPECT_EQ(find_prev_whitespace(s, 6), 4) << "char: '" << s[6] << "'";
    EXPECT_EQ(find_prev_whitespace(s, 14), 12) << "char: '" << s[14] << "'";
}

TEST(textManipSuite, isNumeric) {
    EXPECT_EQ(is_numeric("123"), true);
    EXPECT_EQ(is_numeric("foo"), false);
}

TEST(textManipSuite, findInText) {
    std::vector<std::string> inp = { "foo", "bar", "baz",
                                     "foo with some text" };
    rawterm::Pos expected = { 3, 4 };

    auto out = find_in_text(inp, "with");
    EXPECT_EQ(out.has_value(), true);
    EXPECT_EQ(out.value().vertical, expected.vertical);
    EXPECT_EQ(out.value().horizontal, expected.horizontal);
}
