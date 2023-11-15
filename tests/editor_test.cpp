#include <utility>

#include <rawterm/rawterm.h>

#include "editor.h"
#include "gtest/gtest.h"

TEST(editorClass, constructor) {
    Editor e("");

    EXPECT_EQ(e.active_buffer, 0);
    EXPECT_EQ(e.mode, Mode::Read);
}

TEST(editorClass, setMode) {
    Editor e("");
    e.set_mode(Mode::Write);
    EXPECT_EQ(e.mode, Mode::Write);

    e.set_mode(Mode::Read);
    EXPECT_EQ(e.mode, Mode::Read);
}

TEST(editorClass, getModeStr) {
    Editor e("");
    EXPECT_EQ(e.get_mode(), "READ");
    e.set_mode(Mode::Write);
    EXPECT_EQ(e.get_mode(), "WRITE");
}
