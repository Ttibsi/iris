#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include "editor.h"
#include "buffer.h"
#include "gtest/gtest.h"

inline Buffer create_Buf(std::string file) {
    Editor e(file);
    Buffer b = e.buffers[0];
    return b;
}

class BufferConstructedTestSuite : public ::testing::Test {
  protected:
    Buffer b;

    BufferConstructedTestSuite();
};

inline BufferConstructedTestSuite::BufferConstructedTestSuite() : b(create_Buf("fixture/example_file.txt")) {}


#endif // TEST_UTILS_H
