#include "model.h"

#include <algorithm>

#include "file_io.h"

Model::Model() : buf(Gapbuffer()) {};

// TODO: readonly and modified
Model::Model(Gapbuffer file_chars, const std::string& filename)
    : buf(file_chars), file_name(filename) {};

// 0-based position of the current character, used for insertion/deletion
[[nodiscard]] int Model::get_abs_pos() const {
    int char_pos = buf.find('\n', current_line - 1);
    char_pos += current_char_in_line;
    if (current_line == 1) {
        char_pos--;
    }
    return char_pos;
}

[[nodiscard]] char Model::get_current_char() const {
    return buf.at(buf.pos());
}

[[nodiscard]] char Model::get_next_char() const {
    return buf.at(buf.pos() + 1);
}

[[nodiscard]] const std::string Model::get_current_line() const {
    return buf.line(buf.pos());
}

void Model::insert_char(char c) {
    buf.push_back(c);
    current_char_in_line++;
}

void Model::line_up() {
    for (unsigned int i = 0; i < buf.line(buf.pos()).size(); i++) {
        buf.retreat();
    }
}

void Model::line_down() {
    for (unsigned int i = 0; i < buf.line(buf.pos()).size(); i++) {
        buf.advance();
    }
}

// TODO: Display "saved x bytes"
int Model::save_file() const {
    return write_to_file(file_name, buf);
}
