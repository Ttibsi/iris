#include "view.h"

#include <algorithm>
#include <format>
#include <iterator>
#include <stdexcept>

#include <rawterm/color.h>
#include <rawterm/text.h>

#include "constants.h"
#include "controller.h"
#include "gapbuffer.h"

View::View(Controller* controller, const rawterm::Pos dims)
    : ctrlr_ptr(controller), view_size(dims), cur(rawterm::Cursor()) {
    open_files.reserve(8);
    viewable_models.reserve(8);
}

Model* View::get_active_model() {
    return viewable_models.at(active_model - 1);
}

void View::add_model(Model* m) {
    viewable_models.push_back(m);
    open_files.push_back({m->file_name, rawterm::Pos(1, 1)});
    active_model = viewable_models.size();

    if (LINE_NUMBERS) {
        line_number_offset = std::to_string(m->buf.line_count()).size() + 1;
    }
}

void View::close_model() {
    throw std::logic_error("Not Implemented");
}

void View::change_view_forward() {
    throw std::logic_error("Not Implemented");
}

void View::change_view_backward() {
    throw std::logic_error("Not Implemented");
}

void View::render_screen() {
    int remaining_rows = view_size.vertical - 2;
    rawterm::Pos starting_cur_pos = cur;
    std::string screen = "";
    int line_count = get_active_model()->vertical_file_offset + 1;
    const int max_line_width = view_size.horizontal - line_number_offset - 4;

    rawterm::clear_screen();
    cur.move({1, 1});

    // Draw tab bar
    if (open_files.size() > 1) {
        screen += generate_tab_bar();
        remaining_rows--;
    }

    // Even though it appears this might not be needed, we still need this to
    // prevent the first line being cut off
    if (LINE_NUMBERS) {
        screen += rawterm::set_foreground(
            std::format("{:>{}}\u2502", line_count, line_number_offset), COLOR_UI_BG);
    }

    if (get_active_model()->buf.size() > 0) {
        // Find starting point in gapvector
        int buf_playhead =
            get_active_model()->buf.find('\n', get_active_model()->vertical_file_offset);

        if (buf_playhead != 0) {
            buf_playhead++;
        }

        // To truncate lines
        int horizontal_counter = 0;

        while (remaining_rows) {
            if (buf_playhead == static_cast<int>(get_active_model()->buf.size())) {
                screen += "\r\n";
                break;
            }

            char c = get_active_model()->buf.at(buf_playhead);
            screen += c;

            // Truncate line
            horizontal_counter++;
            if (horizontal_counter >= max_line_width) {
                screen += "\u00BB\r\n";
                Gapbuffer* buf_ptr = &get_active_model()->buf;

                // Skip characters being cut off
                buf_playhead += std::distance(
                    buf_ptr->begin() + buf_playhead,
                    std::find(buf_ptr->begin() + buf_playhead, buf_ptr->end(), '\n'));

                remaining_rows--;
                line_count++;
                horizontal_counter = 0;

                if (LINE_NUMBERS) {
                    screen += rawterm::set_foreground(
                        std::format("{:>{}}\u2502", line_count, line_number_offset), COLOR_UI_BG);
                }

                if (buf_playhead < static_cast<int>(get_active_model()->buf.size())) {
                    buf_playhead++;  // Skip the newline
                }
                continue;
            }

            if (c == '\n') {
                remaining_rows--;
                line_count++;
                horizontal_counter = 0;

                if (LINE_NUMBERS) {
                    screen += rawterm::set_foreground(
                        std::format("{:>{}}\u2502", line_count, line_number_offset), COLOR_UI_BG);
                }
            }

            buf_playhead++;
        }
    } else {
        // Display an empty buffer
        screen += "\r\n";
    }

    while (remaining_rows) {
        remaining_rows--;
        screen += "~\r\n";
    }

    std::cout << screen;
    draw_status_bar();
    std::cout << "\n";  // Notification bar

    cur.move(starting_cur_pos);
    if (LINE_NUMBERS && !(rawterm::detail::is_debug())) {
        while (cur.horizontal < line_number_offset + 2) {
            cur.move_right();
        }
    }
}

const std::string View::generate_tab_bar() const {
    std::string ret = "| ";

    for (unsigned int i = 0; i < open_files.size(); ++i) {
        if (i == static_cast<unsigned int>(active_model - 1)) {
            ret += rawterm::inverse(open_files[i].filename);
        } else {
            ret += open_files[i].filename;
        }

        ret += " | ";
    }

    ret += "\n";
    return ret;
}

void View::draw_status_bar() {
    rawterm::Pos starting_cur_pos = cur;
    cur.move({view_size.vertical - 1, 1});
    rawterm::clear_line();
    std::cout << render_status_bar();
    cur.move(starting_cur_pos);
}

// TODO: The center text isn't aligned right - it ends at the center point
const std::string View::render_status_bar() const {
    std::string filename = open_files.at(active_model - 1).filename;

    // left = mode | git branch | status (read_only/modified)
    // center = file name
    // right = language | cursor position
    std::string left = " " + ctrlr_ptr->get_mode();
    if (!(ctrlr_ptr->git_branch.empty())) {
        left += std::string(" | ") + ctrlr_ptr->git_branch;
    }

    if (viewable_models.at(active_model - 1)->readonly) {
        left += " | [RO]";
    } else if (viewable_models.at(active_model - 1)->modified) {
        left += " | [X]";
    }

    // TODO: file language after highlighting engine
    std::string right =
        "| " + std::to_string(viewable_models.at(active_model - 1)->current_line) + ":" +
        std::to_string(viewable_models.at(active_model - 1)->current_char_in_line) + " ";

    // TODO: handle overflows
    float divide =
        static_cast<float>(view_size.horizontal - (left.size() + filename.size() + right.size())) /
        2.0;

    std::string ret =
        left + std::string(divide - static_cast<int>(filename.size() / 2), ' ') + filename +
        std::string(
            divide + static_cast<int>(filename.size() / 2) + !(floorf(divide) == divide), ' ') +
        right;

    return rawterm::set_background(ret, COLOR_UI_BG);
}

void View::render_line() {
    rawterm::clear_line();
    const rawterm::Pos cur_pos = cur;
    const unsigned int horizontal_draw_space = view_size.horizontal - line_number_offset - 2;

    cur.move({cur.vertical, 1});

    std::string line = "";
    if (LINE_NUMBERS) {
        line = rawterm::set_foreground(
            std::format("{:>{}}\u2502", get_active_model()->current_line, line_number_offset),
            COLOR_UI_BG);
    }

    try {
        std::string line_chars = get_active_model()->get_current_line();
        line += line_chars.substr(0, horizontal_draw_space);
    } catch (const std::runtime_error&) {
    }

    // Truncate
    if (line.size() > horizontal_draw_space) {
        line += "\u00BB";
    }

    std::cout << line;
    cur.move(cur_pos);
}

void View::set_status(const std::string& msg) {
    cur.move({1, view_size.horizontal});
    rawterm::clear_line();
    std::cout << rawterm::set_foreground(msg, COLOR_NOTIFY);
}

void View::cursor_left() {
    const int left_most_pos = (LINE_NUMBERS ? line_number_offset + 2 : 0);
    if (cur.horizontal > left_most_pos) {
        cur.move_left();
        get_active_model()->current_char_in_line--;
        get_active_model()->buf.retreat();
        draw_status_bar();
    }
}

void View::cursor_up() {
    // Check if we're at the bottom of the file
    if (viewable_models.at(active_model - 1)->current_line == 1) {
        return;
    }

    int vertical_offset = (open_files.size() > 1 ? 1 : 0);
    if (cur.vertical - 1 == vertical_offset) {
        // scroll view
        get_active_model()->vertical_file_offset--;
        render_screen();
    } else {
        // move cursor
        cur.move_up();
    }

    get_active_model()->line_up();
    get_active_model()->current_line--;
    draw_status_bar();
}

void View::cursor_down() {
    // Check if we're at the bottom of the file
    if (get_active_model()->current_line == get_active_model()->buf.line_count()) {
        return;
    }

    int vertical_offset = (open_files.size() > 1 ? 3 : 2);
    if (cur.vertical + 1 > view_size.vertical - vertical_offset) {
        // scroll view
        get_active_model()->vertical_file_offset++;
        render_screen();
    } else {
        // move cursor
        cur.move_down();
    }

    get_active_model()->line_down();
    get_active_model()->current_line++;
    draw_status_bar();
}

void View::cursor_right() {
    if (cur.horizontal >= view_size.horizontal) {
        return;
    }

    auto trigger = [this]() {
        cur.move_right();
        get_active_model()->buf.advance();
        get_active_model()->current_char_in_line++;
        draw_status_bar();
    };

    char next_char = '\0';
    try {
        next_char = get_active_model()->get_next_char();
    } catch (const std::out_of_range& e) {
        next_char = '\0';
    }

    switch (ctrlr_ptr->mode) {
        case Mode::Read:
            if (next_char != '\r') {
                trigger();
            }
            break;
        case Mode::Write:
            if (next_char != '\n') {
                trigger();
            }
            break;
        case Mode::Command:
            // TODO: command mode
            break;
    }
}
