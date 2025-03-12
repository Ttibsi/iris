#include "model.h"

#include "controller.h"
#include "text_io.h"

Model::Model(const std::size_t view_height, std::string_view file_name)
    : buf({""}), filename(file_name) {
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

        const std::size_t prev_line_len = buf.at(current_line - 1).size();
        buf.at(current_line - 1) += buf.at(current_line);
        buf.erase(buf.begin() + current_line);

        current_line--;
        current_char = static_cast<unsigned int>(prev_line_len);

        return Redraw::Screen;
    } else {
        current_char--;
        buf.at(current_line).erase(current_char, 1);

        return Redraw::Line;
    }
}

[[nodiscard]] std::size_t Model::newline() {
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

// Word (noun) - a sequence of characters that match regex A-Za-z
[[nodiscard]] std::optional<int> Model::next_word_pos() {
    std::string_view line_frag = std::string_view(buf.at(current_line)).substr(current_char);
    uint incrementer = 0;

    // go to end of current "word"
    while (is_letter(line_frag.at(incrementer))) {
        incrementer++;

        // At end of line, don't move
        if (incrementer == line_frag.size() - 1) {
            return {};
        }
    }

    // go to start of next word
    while (!(is_letter(line_frag.at(incrementer)))) {
        incrementer++;

        // At end of line, don't move
        if (incrementer == line_frag.size() - 1) {
            return incrementer;
        }
    }

    return incrementer;
}
