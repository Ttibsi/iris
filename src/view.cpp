#include <algorithm>
#include <format>
#include <iterator>
#include <print>
#include <string>

#include "constants.h"
#include "controller.h"
#include "view.h"

#include <rawterm/color.h>
#include <rawterm/core.h>
#include <rawterm/text.h>

View::View(Controller *controller, const rawterm::Pos dims)
    : ctrlr_ptr(controller), view_size(dims), cur(rawterm::Cursor()) {
    models.reserve(8);
}

void View::add_model(Model *m) {
    models.push_back(m);
    active_model = models.size();

    // Set visual offset
    if (LINE_NUMBERS) {
        int line_count = std::count(m->buf.begin(), m->buf.end(), '\n');
        line_number_offset = std::to_string(line_count).size() + 1;
    }
}

Model *View::get_active_model() const {
    return models.at(active_model);
}

void View::draw_screen() {
    rawterm::Pos starting_cur_pos = cur;

    // Draw to screen
    rawterm::clear_screen();
    cur.move({ 1, 1 });
    std::print("{}", render_screen());
    draw_status_bar();
    std::print("\n"); // Notification bar

    // Cursor positioning
    cur.move(starting_cur_pos);
    if (LINE_NUMBERS && !(rawterm::detail::is_debug())) {
        while (cur.horizontal < line_number_offset + 2) {
            cur.move_right();
        }
    }
}

// TODO: Rework
[[nodiscard]] const std::string View::render_screen() const {
    int remaining_rows = view_size.vertical - 2;
    std::string screen = "";
    int line_count = get_active_model()->vertical_scroll_offset + 1;
    const int max_hor_line_len = view_size.horizontal - line_number_offset - 4;

    // Draw tab bar
    if (models.size() > 1) {
        screen += render_tab_bar();
        remaining_rows--;
    }

    // Initial line number, even in an empty buffer
    if (LINE_NUMBERS) {
        screen += rawterm::set_foreground(
            std::format("{:>{}}\u2502", line_count, line_number_offset),
            COLOR_UI_BG);
    }

    if (get_active_model()->buf.size() > 0) {
        // Find starting point in buffer
        unsigned int buf_playhead = get_active_model()->buf.find(
            '\n', get_active_model()->vertical_scroll_offset);

        if (buf_playhead != 0) {
            buf_playhead++;
        }

        // To truncate lines
        int horizontal_counter = 0;

        while (remaining_rows) {
            if (buf_playhead == get_active_model()->buf.size()) {
                screen += "\r\n";
                break;
            }

            char c = get_active_model()->buf.at(buf_playhead);
            // Don't draw tab chars
            if (c == '\t') {
                screen += std::string(TAB_SIZE, ' ');
                horizontal_counter += TAB_SIZE;
            } else {
                screen += c;
                horizontal_counter++;
            }

            // Truncate line
            if (horizontal_counter >= max_hor_line_len) {
                screen += "\u00BB\r\n";
                TwinArray<char> *buf_ptr = &get_active_model()->buf;

                // Skip characters being cut off
                buf_playhead +=
                    std::distance(buf_ptr->begin() + buf_playhead,
                                  std::find(buf_ptr->begin() + buf_playhead,
                                            buf_ptr->end(), '\n'));

                remaining_rows--;
                line_count++;
                horizontal_counter = 0;

                if (LINE_NUMBERS) {
                    screen += rawterm::set_foreground(
                        std::format("{:>{}}\u2502", line_count,
                                    line_number_offset),
                        COLOR_UI_BG);
                }

                if (buf_playhead < get_active_model()->buf.size()) {
                    buf_playhead++; // Skip the newline
                }
                continue;
            }
            if (c == '\n') {
                remaining_rows--;
                line_count++;
                horizontal_counter = 0;

                if (LINE_NUMBERS) {
                    screen += rawterm::set_foreground(
                        std::format("{:>{}}\u2502", line_count,
                                    line_number_offset),
                        COLOR_UI_BG);
                }
            }

            buf_playhead++;
        }
    } else {
        // Display an empty buffer
        screen += "\r\n";
    }

    // Space at the end of the screen
    while (remaining_rows) {
        remaining_rows--;
        screen += "~\r\n";
    }

    return screen;
}

const std::string View::render_tab_bar() const {
    std::string ret = "| ";

    for (unsigned int i = 0; i < models.size(); ++i) {
        if (i == static_cast<unsigned int>(active_model - 1)) {
            ret += rawterm::inverse(models.at(i)->filename);
        } else {
            ret += models.at(i)->filename;
        }

        ret += " | ";
    }

    ret += "\n";
    return ret;
}

void View::draw_line() {
    rawterm::clear_line();
    const rawterm::Pos cur_pos = cur;
    cur.move({ cur.vertical, 1 });
    std::print("{}", render_line());
    cur.move(cur_pos);
}

// TODO: Rework
[[nodiscard]] const std::string View::render_line() const {
    const unsigned int horizontal_draw_space =
        view_size.horizontal - line_number_offset - 2;
    std::string line = "";

    if (LINE_NUMBERS) {
        line = rawterm::set_foreground(
            std::format("{:>{}}\u2502",
                        get_active_model()->buf.curr_line_index() + 1,
                        line_number_offset),
            COLOR_UI_BG);
    }

    std::string line_chars = get_active_model()->buf.get_current_line();

    // Truncate
    if (line_chars.size() > horizontal_draw_space) {
        line += line_chars.substr(0, horizontal_draw_space);
        line += "\u00BB";
    } else {
        line += line_chars;
    }

    return line;
}

// TODO: Break this up so it's easier to update specific elements in statusbar
void View::draw_status_bar() {
    rawterm::Pos starting_cur_pos = cur;
    cur.move({ view_size.vertical - 1, 1 });
    rawterm::clear_line();
    std::print("{}", render_status_bar());
    cur.move(starting_cur_pos);
}

// TODO: The center text isn't aligned right - it ends at the center point
const std::string View::render_status_bar() const {
    std::string filename = models.at(active_model - 1)->filename;

    // left = mode | git branch | status (read_only/modified)
    // center = file name
    // right = language | cursor position
    std::string left = " " + ctrlr_ptr->get_mode();
    if (!(ctrlr_ptr->git_branch.empty())) {
        left += std::string(" | ") + ctrlr_ptr->git_branch;
    }

    if (get_active_model()->readonly) {
        left += " | [RO]";
    } else if (get_active_model()->modified) {
        left += " | [X]";
    }

    // TODO: file language after highlighting engine
    std::string right =
        "| " + std::to_string(get_active_model()->current_line + 1) +
        ":" + std::to_string(get_active_model()->current_char + 1) +
        " ";

    // TODO: handle overflows
    float divide =
        static_cast<float>(view_size.horizontal -
                           (left.size() + filename.size() + right.size())) /
        2.0;

    std::string ret =
        left +
        std::string(divide - static_cast<int>(filename.size() / 2), ' ') +
        filename +
        std::string(divide + static_cast<int>(filename.size() / 2) +
                        !(floorf(divide) == divide),
                    ' ') +
        right;

    return rawterm::set_background(ret, COLOR_UI_BG);
}

void View::cursor_left() {
    if (get_active_model()->current_char) {
        get_active_model()->current_char--; 
        curr.move_left;
    }
}

// TODO: Rework
[[maybe_unused]] bool View::cursor_up() {
    // Check if we're at the top of the file
    if (get_active_model()->buf.curr_line_index() == 1) {
        return false;
    }

    bool redraw_sentinal = false;
    const int vertical_offset = (models.size() > 1 ? 1 : 0);
    if (cur.vertical - 1 == vertical_offset) {
        // scroll view
        get_active_model()->vertical_scroll_offset--;
        redraw_sentinal = true;
    } else {
        // move cursor
        cur.move_up();
    }

    get_active_model()->line_up();
    return redraw_sentinal;
}

// TODO: Rework
[[maybe_unused]] bool View::cursor_down() {
    // Check if we're at the bottom of the file
    if (get_active_model()->eof() && get_active_model()->buf.peek() != '\n') {
        return false;
    }

    bool redraw_sentinal = false;
    const int vertical_offset = (models.size() > 1 ? 3 : 2);
    if (cur.vertical + 1 > view_size.vertical - vertical_offset) {
        // scroll view
        get_active_model()->vertical_scroll_offset++;
        redraw_sentinal = true;
    } else {
        // move cursor
        cur.move_down();
    }

    get_active_model()->line_down();
    return redraw_sentinal;
}

void View::cursor_right() {
    // TODO: Handle line longer than view

    // Only scroll if we're still in the line
    int line_size = get_active_model()->buf.at(get_active_model()->current_line).size();
    if (line_size <= cur.vertical + line_number_offset) {
        get_active_model()->current_line++;
        cur.move_right();
    } 
}
