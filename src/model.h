#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>

#include "gapvector.h"

struct Model {
    Gapvector<> buf;
    std::string file_name = "";
    int current_line = 1;
    int current_char_in_line = 1;
    bool readonly = false;
    bool modified = false;

    Model();
    Model(const std::vector<char>&, const std::string&);
};

#endif  // MODEL_H
