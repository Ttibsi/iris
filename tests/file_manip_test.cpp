#include <string>
#include <vector>

#include "file_manip.h"
#include "gtest/gtest.h"

TEST(fileManip, openingFile) {
    std::vector<std::string> expected = {
        "foo, bar", "\tbaz with a tab", "  two trailing spaces",
        "",         "empty line above",
    };

    EXPECT_EQ(open_file("fixture/example_file.txt"), expected);
}

TEST(fileManip, readonly) {
    EXPECT_EQ(is_readonly("fixture/example_file.txt"), false);
}

TEST(fileManip, filenameOnly) {
    EXPECT_EQ(filename_only("fixture/example_file.txt"), "example_file.txt");
}

TEST(fileManip, fileType) {
    EXPECT_EQ(get_file_type("fixture/example_file.txt"), ".txt");
}
