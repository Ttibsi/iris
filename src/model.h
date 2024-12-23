#ifndef MODEL_H
#define MODEL_H

#include <string>

#include "twin_array.h"

struct Model {
    TwinArray<char> buf;
    std::string filename;
    bool readonly = false;
    bool modified = false;
    int vertical_scroll_offset = 0;

    Model();
    Model(TwinArray<char>, const std::string &);
};

#endif // MODEL_H
