#include "buffer.h"
#include "test_utils.h"
#include "viewport.h"
#include "gtest/gtest.h"
#include <utility>

TEST(viewportClass, constructor) {
    Buffer b = setup("fixture/example_file.txt");
    Viewport v(&b, std::make_pair(6, 9));

    EXPECT_EQ(v.view_size.first, 6);
    EXPECT_EQ(v.view_size.second, 9);
    EXPECT_EQ(v.cursor.row, 0);
    EXPECT_EQ(v.cursor.col, 0);
}
