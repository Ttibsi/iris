#include "constants.h"
#include "text_manip.h"

#include "gtest/gtest.h"

TEST(textManipSuite, filterForSensibleWhitespace) {
    std::vector<std::string> fixture = {
        "This is a normal string", "\tThis string starts with a tab",
        "    This string starts with 4 spaces"
    };

    std::vector<std::string> expected = {
        "This is a normal string\r\n",
        std::string(TABSTOP, ' ') + "This string starts with a tab\r\n",
        "    This string starts with 4 spaces\r\n"
    };

    filter_for_sensible_whitespace(fixture);
    EXPECT_EQ(fixture, expected);
}

TEST(textManipSuite, shellExec) { EXPECT_EQ(shell_exec("echo 'hi'"), "hi"); }

TEST(textManipSuite, tabCount) {
    EXPECT_EQ(tab_count("foo\r"), 0);
    EXPECT_EQ(tab_count("\tfoo\r"), 1);
    EXPECT_EQ(tab_count("\t\tfoo\r"), 2);
}
