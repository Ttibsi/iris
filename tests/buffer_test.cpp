#include <string>
#include <vector>

#include "buffer.h"
#include "constants.h"
#include "cursor.h"
#include "test_utils.h"
#include "gtest/gtest.h"

TEST(bufferClass, constructorEmpty) {
    Buffer b = setup("");
    std::vector<std::string> lines{ "" };

    EXPECT_EQ(b.file, "NO FILE");
    EXPECT_EQ(b.lines, lines);
    EXPECT_EQ(b.readonly, false);
    EXPECT_EQ(b.modified, false);
    EXPECT_EQ(b.current_line, 0);
}

TEST(bufferClass, constructorWithFile) {
    Buffer b = setup("fixture/example_file.txt");

    std::vector<std::string> expected = {
        "foo, bar", "\tbaz with a tab", "  two trailing spaces",
        "",         "empty line above",
    };

    EXPECT_EQ(b.file, "fixture/example_file.txt");
    EXPECT_EQ(b.lines, expected);
    EXPECT_EQ(b.readonly, false);
    EXPECT_EQ(b.modified, false);
    EXPECT_EQ(b.current_line, 0);
}

TEST(bufferClass, renderStatusBar) {
    Buffer b = setup("fixture/example_file.txt");
    Cursor c;

    std::string bar = b.render_status_bar(79, &c);
    EXPECT_EQ(bar.size(), 88); // This won't be the same due to ascii codes

    std::vector<std::string> split;
    std::string tok;
    std::size_t i = 0;

    for (; i < bar.size(); i++) {
        if (bar[i] == '|') {
            split.push_back(tok);
            tok.clear();
        } else {
            tok += bar[i];
        }
    }
    split.push_back(tok);

    // We aren't checking for the mode at the start because the editor
    // that creates it is out of scope by now
    EXPECT_EQ(split[1], " example_file.txt ");
    EXPECT_EQ(split[2], " [ ] ");
    EXPECT_EQ(split[5], " .txt ");

    if (CURSOR_STATUS) {
        EXPECT_EQ(split[6], " Cursor: (1:1) ");

        EXPECT_EQ(split[7], " 1/5 \x1B[27m");
    } else {
        EXPECT_EQ(split[6], " 1/5 \x1B[27m");
    }
}
