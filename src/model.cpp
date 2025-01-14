#include "model.h"

Model::Model(const int view_height) : buf({""}), filename("") {
    buf.reserve(view_height);
}

// TODO: readonly and modified
// NOTE: Intentional copy of file_chars
Model::Model(std::vector<std::string> file_chars, const std::string& filename)
    : buf(file_chars), filename(filename) {}

void Model::backspace() {
    // Model *active = v->get_active_model();
    // const int curr_char = active->buf.curr_char_index() + 1;
    // const int curr_line = active->buf.curr_line_index();
    //
    // if (curr_char == 1 && curr_line == 1) {
    //     return {};
    // }
    //
    // if (curr_char == v->line_number_offset && curr_line > 1) {
    //     // At the start of the line, move cursor up
    //     int line_counter = 0;
    //     int prev_line_len = 0;
    //     for (char &c : active->buf) {
    //         if (c == '\n') {
    //             line_counter++;
    //         }
    //         if (line_counter == curr_line - 1) {
    //             prev_line_len++;
    //         }
    //         if (line_counter >= curr_line) {
    //             break;
    //         }
    //     }
    //
    //     active->buf.move_left();
    //     active->buf.pop();
    //     active->buf.pop();
    //
    //     // We have to use the cursor method here because we don't want any
    //     // other side affects from v->cursor_up()
    //     v->cur.move_up();
    //     for (int i = 0; i <= prev_line_len; i++) {
    //         v->cur.move_right();
    //     }
    //
    // } else {
    //     // Move cursor backwards
    //     active->buf.pop();
    //     v->draw_line();
    //     v->cursor_left();
    // }
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
