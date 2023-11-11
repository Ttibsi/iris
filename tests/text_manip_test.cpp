#include "constants.h"
#include "text_manip.h"

#include "gtest/gtest.h"

TEST(textManipSuite, filterWhitespace) {
    std::vector<std::string> fixture = {
        "This is a normal string", "\tThis string starts with a tab",
        "    This string starts with 4 spaces"
    };

    std::vector<std::string> expected = {
        "This is a normal string\r\n",
        std::string(TABSTOP, ' ') + "This string starts with a tab\r\n",
        "    This string starts with 4 spaces\r\n"
    };

    auto out = filter_whitespace(fixture);
    EXPECT_EQ(out, expected);
}

TEST(textManipSuite, shellExec) { EXPECT_EQ(shell_exec("echo 'hi'"), "hi"); }

TEST(textManipSuite, countChar) {
    EXPECT_EQ(count_char("Hello world", 'l'), 3);
    EXPECT_EQ(count_char("\ttab test", '\t'), 1);
}

TEST(textManipSuite, line_size) {
    EXPECT_EQ(line_size("foo"), 3);
    EXPECT_EQ(line_size("\tfoo"), 8);
    EXPECT_EQ(line_size("\t\tfoo"), 13);
}
