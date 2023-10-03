#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include "editor.h"

inline Buffer setup(std::string f) {
    Editor e(f);
    return e.buffers[0];
}

#endif // TEST_UTILS_H
