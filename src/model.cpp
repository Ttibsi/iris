#include "model.h"

#include <algorithm>

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

    // clean up whitespace
    std::size_t start = second.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) {
        second.clear();
    } else if (start == 1) {
        // if we have a single whitespace char at the start, trim that off
        // NOTE: We don't want to strip all whitespace as it messes with indentation
        second.erase(0, 1);
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
    if (current_char == buf.at(current_line).size() - 1) {
        return {};
    }

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
        if (incrementer >= line_frag.size() - 1) {
            return incrementer;
        }
    }

    return incrementer;
}

[[nodiscard]] std::optional<int> Model::prev_word_pos() {
    if (!(current_char)) {
        return {};
    }

    std::string_view line_frag = std::string_view(buf.at(current_line)).substr(0, current_char);
    uint incrementer = 1;

    while (is_letter(line_frag.at(current_char - incrementer))) {
        incrementer++;

        // At end of line, don't move
        if (incrementer == line_frag.size()) {
            return incrementer;
        }
    }

    while (!(is_letter(line_frag.at(current_char - incrementer)))) {
        incrementer++;

        // At end of line, don't move
        if (incrementer == line_frag.size()) {
            return incrementer;
        }
    }

    return incrementer;
}

[[nodiscard]] std::optional<unsigned int> Model::next_para_pos() {
    if (current_line == buf.size() - 1) {
        return {};
    }
    auto pos = std::find(buf.begin() + current_line + 1, buf.end(), "");
    unsigned int distance =
        static_cast<unsigned int>(std::distance(buf.begin() + current_line, pos));
    if (current_line + distance >= buf.size()) {
        return buf.size() - current_line - 1;
    }
    return distance;
}

[[nodiscard]] std::optional<unsigned int> Model::prev_para_pos() {
    if (current_line == 0) {
        return {};
    }
    auto rev_pos =
        std::find(buf.rbegin() + static_cast<long>(buf.size() - current_line + 1), buf.rend(), "");
    rev_pos++;
    auto pos = rev_pos.base();
    unsigned int distance =
        static_cast<unsigned int>(std::distance(pos, buf.begin() + current_line));
    if (distance > current_line) {
        return current_line;
    }
    return distance;
}

void Model::replace_char(const char c) {
    if (buf.at(current_line).empty()) {
        buf.at(current_line).push_back(c);
        return;
    }

    buf.at(current_line).at(current_char) = c;
}
