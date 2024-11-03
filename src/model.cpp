#include "model.h"

#include <algorithm>

#include "file_io.h"

Model::Model() : buf(Gapvector()), line_count(0) {};

// TODO: readonly and modified
Model::Model(Gapvector<> file_chars, const std::string& filename)
    : buf(file_chars), file_name(filename), line_count(std::count(buf.begin(), buf.end(), '\n')) {};

// 0-based position of the current character, used for insertion/deletion
[[nodiscard]] int Model::get_abs_pos() const {
    int char_pos = buf.find_ith_char('\n', current_line - 1);
    char_pos += current_char_in_line;
    if (current_line == 1) {
        char_pos--;
    }
    return char_pos;
}

[[nodiscard]] char Model::get_current_char() const {
    return buf.at(get_abs_pos());
}

[[nodiscard]] char Model::get_next_char() const {
    return buf.at(get_abs_pos() + 1);
}

[[nodiscard]] const std::string Model::get_current_line() const {
    // The -1 is because we want the abs pos of the last char, not
    // the cursor
    return buf.line(std::max(get_abs_pos() - 1, 0));
}

void Model::insert_char(char c) {
    buf.insert(buf.begin() + get_abs_pos(), c);
}

// TODO: Display "saved x bytes"
int Model::save_file() const {
    return write_to_file(file_name, buf);
}
