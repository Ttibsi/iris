#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>

#include "gapvector.h"

struct Model {
    Gapvector<> buf;
    std::string file_name = "";
    unsigned int line_count;
    unsigned int current_line = 1;
    unsigned int current_char_in_line = 1;
    bool readonly = false;
    bool modified = false;
    unsigned int vertical_file_offset = 0;

    Model();
    Model(const std::vector<char>&, const std::string&);
    [[nodiscard]] int get_abs_pos() const;
    [[nodiscard]] char get_prev_char() const;
    [[nodiscard]] char get_current_char() const;
    [[nodiscard]] char get_next_char() const;
    void insert_char(char);
    int save_file() const;
};

#endif  // MODEL_H
