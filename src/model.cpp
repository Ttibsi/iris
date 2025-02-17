#include "model.h"

#include "controller.h"

Model::Model(const int view_height, std::string_view file_name) : buf({""}), filename(file_name) {
    buf.reserve(view_height);
}

// TODO: readonly and modified
// NOTE: Intentional copy of file_chars
Model::Model(std::vector<std::string> file_chars, std::string_view filename)
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
    std::string first = buf.at(current_line).substr(0, current_char);
    std::string second = buf.at(current_line).substr(current_char);

    // trip preceeding whitespace from second line
    std::size_t start = second.find_first_not_of(" \t\n\r\f\v");
    if (start != std::string::npos) {
        second.erase(0, start);
    } else {
        second.clear();
    }

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

[[nodiscard]] bool Model::lineno_in_scope(const int idx) const {
    return (idx < static_cast<int>(buf.size()) && idx >= 0);
}
