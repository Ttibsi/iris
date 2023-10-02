#include "text_manip.h"
#include "gtest/gtest.h"

TEST(textManip, shellExec) { EXPECT_EQ(shell_exec("echo 'hi'"), "hi"); }
