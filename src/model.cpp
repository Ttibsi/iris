#include "model.h"

#include <algorithm>
#include <format>
#include <functional>
#include <ranges>
#include <regex>

#include "action.h"
#include "constants.h"
#include "controller.h"
#include "text_io.h"

Model::Model(const std::size_t view_height, std::string_view file_name)
    : buf({""}), filename(file_name) {
    buf.reserve(view_height);
    set_read_only(file_name);
}

// NOTE: Intentional copy of file_chars
Model::Model(std::vector<std::string> file_chars, std::string_view file_name)
    : buf(file_chars), filename(file_name) {
    set_read_only(file_name);
}

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

        unsaved = true;
        return Redraw::Screen;
    } else {
        current_char--;
        buf.at(current_line).erase(current_char, 1);

        unsaved = true;
        return Redraw::Line;
    }
}

[[nodiscard]] std::size_t Model::newline() {
    std::string first = buf.at(current_line).substr(0, current_char);
    std::string second = buf.at(current_line).substr(current_char);

    // clean up whitespace
    std::size_t start = second.find_first_not_of(WHITESPACE);
    if (start == std::string::npos) {
        second.clear();
    } else if (start == 1) {
        // if we have a single whitespace char at the start, trim that off
        // NOTE: We don't want to strip all whitespace as it messes with indentation
        second.erase(0, 1);
    }

    buf.at(current_line) = first;
    current_line++;

    if (!second.size()) {
        second.push_back(' ');
    }

    // Add intentional indentation
    const int preceeding_ws = first_non_whitespace(first);
    if (preceeding_ws % TAB_SIZE == 0 && preceeding_ws > 0) {
        second = std::format("{}{}", std::string(std::size_t(preceeding_ws), ' '), second);
        current_char = TAB_SIZE;
    } else {
        current_char = 0;
    }

    buf.insert(buf.begin() + current_line, second);
    unsaved = true;

    if (preceeding_ws % TAB_SIZE == 0 && preceeding_ws > 0) {
        return TAB_SIZE < first.size() ? first.size() - TAB_SIZE : 0;
    } else {
        return first.size();
    }
}

void Model::insert(const char c) {
    buf.at(current_line).insert(current_char, 1, c);
    current_char++;
    unsaved = true;
}

[[nodiscard]] bool Model::lineno_in_scope(const int idx) const {
    return (idx < static_cast<int>(buf.size()) && idx >= 0);
}

// Word (noun) - a sequence of characters that match regex A-Za-z
[[nodiscard]] std::optional<int> Model::next_word_pos() {
    if (current_char == buf.at(current_line).size() - 1) {
        return {};
    } else if (buf.at(current_line).empty()) {
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
    } else if (buf.at(current_line).empty()) {
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
    auto pos = std::find_if(buf.begin() + current_line + 1, buf.end(), [](const std::string& s) {
        return s.empty() ||
               std::all_of(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); });
    });

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

    auto rev_pos = std::find_if(
        buf.rbegin() + static_cast<long>(buf.size() - current_line + 1), buf.rend(),
        [](const std::string& s) {
            return s.empty() ||
                   std::all_of(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); });
        });
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
    unsaved = true;
}

void Model::toggle_case() {
    char c = buf.at(current_line).at(current_char);

    if (c >= 'A' && c <= 'Z') {
        buf.at(current_line).at(current_char) = c + 32;
    } else if (c >= 'a' && c <= 'z') {
        buf.at(current_line).at(current_char) = c - 32;
    }

    unsaved = true;
}

[[nodiscard]] std::optional<rawterm::Pos> Model::find_next(const char c) {
    unsigned int cur_line = current_line;
    int cur_char = int32_t(current_char);

    for (; cur_line < buf.size(); cur_line++) {
        auto iter = std::find(buf.at(cur_line).begin() + cur_char + 1, buf.at(cur_line).end(), c);

        if (iter != buf.at(cur_line).end()) {
            cur_char = int32_t(std::distance(buf.at(cur_line).begin(), iter));

            // line is a relative value, char is an absolute value
            return rawterm::Pos(
                {static_cast<int>(cur_line - current_line), static_cast<int>(cur_char)});
        }
    }

    return {};
}

[[nodiscard]] std::optional<rawterm::Pos> Model::find_prev(const char c) {
    unsigned int cur_line = current_line;
    int cur_char = int32_t(current_char);

    for (; cur_line >= 0 && cur_line < buf.size(); cur_line--) {
        if (!(cur_line == current_line)) {
            cur_char = int32_t(buf.at(cur_line).size() - 1);
        }

        auto iter = std::find(
            buf.at(cur_line).rbegin() + int32_t(buf.at(cur_line).size()) - cur_char,
            buf.at(cur_line).rend(), c);

        if (iter != buf.at(cur_line).rend()) {
            cur_char = int32_t(std::distance(buf.at(cur_line).begin(), iter.base() - 1));

            // line is a relative value, char is an absolute value
            return rawterm::Pos(
                {static_cast<int>(current_line - cur_line), static_cast<int>(cur_char)});
        }
    }

    return {};
}

[[nodiscard]] bool Model::undo(const int height) {
    if (!undo_stack.size()) {
        return false;
    }

    Change cur_change = undo_stack.at(undo_stack.size() - 1);
    undo_stack.pop_back();
    redo_stack.push(cur_change);

    rawterm::Pos cur_pos = {int32_t(current_line), int32_t(current_char)};
    current_line = cur_change.line_pos;
    current_char = cur_change.char_pos;

    // NOTE: Currently we are not moving the cursor alongside an undo update
    // as we're restoring the model position after the undo. While these are
    // separate values, keeping them in sync is currently easier this way.
    // I want to see how using it feels and if I need to update the cursor or
    // not during usage
    switch (cur_change.action) {
        case ActionType::Backspace:
            current_char--;
            [[fallthrough]];
        case ActionType::DelCurrentChar: {
            insert(cur_change.payload.value());
        } break;

        case ActionType::Newline: {
            current_char = 0;
            current_line++;
            std::ignore = backspace();
        } break;

        case ActionType::ToggleCase: {
            toggle_case();
        } break;

        case ActionType::InsertChar: {
            std::ignore = backspace();
        } break;

        case ActionType::ReplaceChar: {
            const char cur_char = cur_change.payload.value();
            redo_stack.top().payload = get_current_char();
            replace_char(cur_char);
        } break;

        case ActionType::DelCurrentLine: {
            if (cur_change.text.has_value()) {
                buf.insert(buf.begin() + cur_change.line_pos, cur_change.text.value());
            }
        } break;

        case ActionType::DelCurrentWord: {
            if (cur_change.text.has_value()) {
                buf.at(current_line).insert(current_char, cur_change.text.value());
            }
        } break;

        default:
            break;
    };

    if (!undo_stack.size()) {
        unsaved = false;
    }

    current_line = uint32_t(cur_pos.vertical);
    current_char = uint32_t(cur_pos.horizontal);

    if (view_offset <= cur_change.line_pos) {
        return true;
    }
    if (cur_change.line_pos <= view_offset + uint32_t(height)) {
        return true;
    }

    return false;
}

[[nodiscard]] char Model::get_current_char() const {
    return buf.at(current_line).at(current_char);
}

[[nodiscard]] bool Model::redo(const int height) {
    if (!redo_stack.size()) {
        return false;
    }

    const Change cur_change = redo_stack.top();
    redo_stack.pop();
    undo_stack.push_back(cur_change);
    if (undo_stack.size()) {
        unsaved = true;
    }

    rawterm::Pos cur_pos = {int32_t(current_line), int32_t(current_char)};
    current_line = cur_change.line_pos;
    current_char = cur_change.char_pos;

    switch (cur_change.action) {
        case ActionType::Backspace: {
            std::ignore = backspace();
        } break;

        case ActionType::DelCurrentChar: {
            current_char++;
            std::ignore = backspace();
        } break;

        case ActionType::Newline: {
            std::ignore = newline();
        } break;

        case ActionType::ToggleCase: {
            toggle_case();
        } break;

        case ActionType::InsertChar: {
            current_char--;
            insert(cur_change.payload.value());
        } break;

        case ActionType::ReplaceChar: {
            replace_char(cur_change.payload.value());
        } break;

        case ActionType::DelCurrentLine: {
            delete_current_line();
        } break;

        case ActionType::DelCurrentWord: {
            delete_current_word({cur_change.text.value(), current_char, current_line});
        } break;

        default:
            break;
    };

    current_line = uint32_t(cur_pos.vertical);
    current_char = uint32_t(cur_pos.horizontal);
    if ((view_offset <= cur_change.line_pos) ||
        (cur_change.line_pos <= view_offset + uint32_t(height))) {
        return true;
    }

    return false;
}

void Model::move_line_down() {
    std::iter_swap(buf.begin() + current_line, buf.begin() + current_line + 1);
}

void Model::move_line_up() {
    std::iter_swap(buf.begin() + current_line, buf.begin() + current_line - 1);
}

void Model::set_read_only(std::string_view file) {
    if (file == "" || !(file_exists(file))) {
        return;
    }

    readonly = (access(file.data(), W_OK) == -1);
}

void Model::delete_current_line() {
    buf.erase(buf.begin() + current_line);
    current_line = (current_line > 0) ? current_line - 1 : 0;
    unsaved = true;
    if (buf.at(current_line).size() < current_char) {
        current_char = uint_t(buf.at(current_line).size());
    }
}

[[nodiscard]] const WordPos Model::current_word() const {
    WordPos ret = {"", 0, 0};
    const std::string* cur_line = &buf.at(current_line);
    uint_t start = current_char;

    while (start && is_letter(cur_line->at(start))) {
        start--;
    }

    // if we aren't at 0, we're on a whitespace, so we want to increment once
    if (start) {
        start++;
    }

    uint_t len = uint_t(
        std::distance(
            cur_line->begin() + start,
            std::find_if(cur_line->begin() + start, cur_line->end(), std::not_fn(is_letter))));

    ret.start_pos = start;
    ret.text = cur_line->substr(start, len);
    ret.lineno = current_line;
    return ret;
}

void Model::delete_current_word(const WordPos pos) {
    buf.at(pos.lineno).erase(pos.start_pos, pos.text.size());
}

[[nodiscard]] std::vector<std::string> Model::search_text(const std::string& input) const {
    std::vector<std::string> ret = {};
    ret.reserve(7);

    auto re = std::regex(input);
    for (const auto&& [idx, line] : std::views::enumerate(buf)) {
        if (std::regex_search(line, re)) {
            // TODO: Truncate line?
            ret.push_back(std::format("|{}| {}", idx + 1, line));
        }

        if (ret.size() == 7) {
            break;
        }
    }

    return ret;
}

/// Input: `to|from|flags`
/// Flag options: m (multiline)
void Model::search_and_replace(const std::string& input) {
    // TODO: c (confirm) (todo once we have highlighting?)
    std::vector<std::string> parts = split_by(input, '|');
    if (parts.size() < 2 || parts.size() > 3) {
        return;
    }

    auto logger = spdlog::get("basic_logger");
    if (logger != nullptr) {
        logger->info("Find: '" + parts.at(0) + "' Replace: '" + parts.at(1) + "'");
    }
    auto find = std::regex(parts.at(0));

    if (parts.size() == 3 && parts.at(2).find('m') <= parts.at(2).size()) {
        for (auto& line : buf) {
            line = std::regex_replace(line, find, parts.at(1));
        }
    } else {
        buf.at(current_line) = std::regex_replace(buf.at(current_line), find, parts.at(1));
    }
}
