#include "buffer.h"
#include "test_utils.h"
#include "viewport.h"
#include "gtest/gtest.h"
#include <utility>

TEST_F(BufferConstructedTestSuite, constructor) {
    Viewport v(&b, { 6, 9 });

    EXPECT_EQ(v.view_size.vertical, 6);
    EXPECT_EQ(v.view_size.horizontal, 9);
    EXPECT_EQ(v.cursor.row, 1);
    EXPECT_EQ(v.cursor.col, 1);
}
