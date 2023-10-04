#include <string>
#include <vector>

#include "file_manip.h"
#include "gtest/gtest.h"

TEST(fileManipSuite, openingFile) {
    std::vector<std::string> expected = {
        "foo, bar", "\tbaz with a tab", "  two trailing spaces",
        "",         "empty line above",
    };

    EXPECT_EQ(open_file("fixture/example_file.txt"), expected);
}

TEST(fileManipSuite, readonly) {
    EXPECT_EQ(is_readonly("fixture/example_file.txt"), false);
}

TEST(fileManipSuite, filenameOnly) {
    EXPECT_EQ(filename_only("fixture/example_file.txt"), "example_file.txt");
}

TEST(fileManipSuite, fileType) {
    EXPECT_EQ(get_file_type("fixture/example_file.txt"), ".txt");
}
