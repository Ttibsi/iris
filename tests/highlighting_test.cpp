#include <string>

#include "constants.h"
#include "highlighting/highlighter.h"
#include "highlighting/languages.h"
#include "highlighting/theme_parsing.h"
#include "gtest/gtest.h"

TEST(highlightSuite, parseColour) {
    EXPECT_EQ(parse_colour("#ABCDEF"), "171;205;239m");
}

TEST(highlightSuite, highlightLine) {
    std::string inp = "foo: int = 4";
    std::string out =
        "foo: \x1b[38;2;196;160;0mint\x1B[0m = \x1b[38;2;255;34;34m4\x1B[0m";
    highlight_line(Language::PYTHON, inp);
    EXPECT_EQ(inp, out);

    inp = "add_subdirectory(foo)";
    out = "\x1b[38;2;102;102;255madd_subdirectory\x1B[0m(foo)";
    highlight_line(Language::CMAKE, inp);
    EXPECT_EQ(inp, out);
}

TEST(highlightSuite, langString) {
    EXPECT_EQ(lang_string(Language::PYTHON), "PYTHON");
    EXPECT_EQ(lang_string(Language::UNKNOWN), "UNKNOWN");
}

TEST(highlightSuite, getTheme) {
    if (THEME == "default") {
        auto theme = get_theme();

        EXPECT_EQ(theme.size(), 8);
    }
}
