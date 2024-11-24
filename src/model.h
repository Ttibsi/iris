#ifndef MODEL_H
#define MODEL_H

#include <string>

#include "gapbuffer.h"

struct Model {
    Gapbuffer buf;
    std::string file_name = "";
    unsigned int current_line = 1;
    int current_char_in_line = 1;
    bool readonly = false;
    bool modified = false;
    int vertical_file_offset = 0;

    Model();
    Model(Gapbuffer, const std::string&);
    [[nodiscard]] int get_abs_pos() const;
    [[nodiscard]] char get_current_char() const;
    [[nodiscard]] char get_next_char() const;
    [[nodiscard]] const std::string get_current_line() const;
    void line_up();
    void line_down();
    void insert_char(char);
    int save_file() const;
};

#endif  // MODEL_H
