#include "cursor.h"
#include "gtest/gtest.h"

TEST(cursorClass, constructor) {
    Cursor c;

    EXPECT_EQ(c.row, 0);
    EXPECT_EQ(c.col, 0);
}

TEST(cursorClass, setPosAbs) {
    Cursor c;
    c.set_pos_abs(6, 9);

    EXPECT_EQ(c.row, 6);
    EXPECT_EQ(c.col, 9);
}

TEST(cursorClass, setPosRel) {
    Cursor c;

    c.set_pos_rel(3, 2);
    EXPECT_EQ(c.row, 3);
    EXPECT_EQ(c.col, 2);

    c.set_pos_rel(-1, 0);
    EXPECT_EQ(c.row, 2);
    EXPECT_EQ(c.col, 2);
}
