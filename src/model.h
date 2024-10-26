#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>

#include "gapvector.h"

struct Model {
    Gapvector<> buf;
    std::string file_name = "";
    int line_count;
    int current_line = 1;
    int current_char_in_line = 1;
    bool readonly = false;
    bool modified = false;
    int vertical_file_offset = 0;

    Model();
    Model(const std::vector<char>&, const std::string&);
    [[nodiscard]] int get_abs_pos() const;
    [[nodiscard]] char get_current_char() const;
    [[nodiscard]] char get_next_char() const;
    [[nodiscard]] const std::string get_current_line() const;
    void insert_char(char);
    int save_file() const;
};

#endif  // MODEL_H
