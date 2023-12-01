#include "constants.h"
#include "text_manip.h"

#include "gtest/gtest.h"

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
    EXPECT_EQ(shell_exec("echo 'hi'", true), "hi");
    EXPECT_EQ(shell_exec("echo 'hi'", false), "");

#ifdef __APPLE__
    EXPECT_EQ(shell_exec("mv", true),
              "usage: mv [-f | -i | -n] [-hv] source target"
              "       mv [-f | -i | -n] [-v] source ... directory");
#else
    EXPECT_EQ(shell_exec("mv", true), "mv: missing file operandTry 'mv "
                                      "--help' for more information.");

#endif
}

TEST(textManipSuite, countChar) {
    EXPECT_EQ(count_char("Hello world", 'l'), 3);
    EXPECT_EQ(count_char("\ttab test", '\t'), 1);
}

TEST(textManipSuite, line_size) {
    EXPECT_EQ(line_size("foo"), 3);
    EXPECT_EQ(line_size("\tfoo"), 8);
    EXPECT_EQ(line_size("\t\tfoo"), 13);
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
