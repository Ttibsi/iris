#include "model.h"

#include "controller.h"

Model::Model(const int view_height) : buf({""}), filename("") {
    buf.reserve(view_height);
}

// TODO: readonly and modified
// NOTE: Intentional copy of file_chars
Model::Model(std::vector<std::string> file_chars, const std::string& filename)
    : buf(file_chars), filename(filename) {}

[[nodiscard]] Redraw Model::backspace() {
    if (current_char == 0) {
        // Concat two lines

        // At top of buffer
        if (current_line == 0) {
            return Redraw::None;
        }

        const int prev_line_len = buf.at(current_line - 1).size();
        buf.at(current_line - 1) += buf.at(current_line);
        buf.erase(buf.begin() + current_line);

        current_line--;
        current_char = prev_line_len;

        return Redraw::Screen;
    } else {
        current_char--;
        buf.at(current_line).erase(current_char, 1);

        return Redraw::Line;
    }
}

[[nodiscard]] int Model::newline() {
    // TODO: Trim whitespace
    std::string first = buf.at(current_line).substr(0, current_char);
    std::string second = buf.at(current_line).substr(current_char);

    buf.at(current_line) = first;
    current_line++;
    buf.insert(buf.begin() + current_line, second);

    current_char = 0;

    return first.size();
}

void Model::insert(const char c) {
    buf.at(current_line).insert(current_char, 1, c);
    current_char++;
}
