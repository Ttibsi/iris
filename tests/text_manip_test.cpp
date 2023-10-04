#include "text_manip.h"
#include "gtest/gtest.h"

TEST(textManipSuite, shellExec) { EXPECT_EQ(shell_exec("echo 'hi'"), "hi"); }
