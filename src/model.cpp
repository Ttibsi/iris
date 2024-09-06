#include "model.h"

#include <algorithm>

Model::Model() : buf(Gapvector()), line_count(0) {};

// TODO: readonly and modified
Model::Model(const std::vector<char>& file_chars, const std::string& filename)
    : buf(Gapvector(file_chars.begin(), file_chars.end())),
      file_name(filename),
      line_count(std::count(buf.begin(), buf.end(), '\n')) {};

[[nodiscard]] int Model::get_abs_pos() const {
    int char_pos = buf.find_ith_char('\n', current_line - 1) + current_char_in_line;
    if (current_line == 1) {
        char_pos--;
    }

    return char_pos;
}

[[nodiscard]] char Model::get_current_char() const {
    return buf.at(get_abs_pos());
}

[[nodiscard]] char Model::get_next_char() const {
    int char_pos = buf.find_ith_char('\n', current_line - 1) + current_char_in_line;
    // if (current_line == 1) { char_pos++; }
    return buf.at(char_pos);
}

void Model::insert_char(char c) {
    buf.insert(buf.begin() + get_abs_pos(), c);
}
