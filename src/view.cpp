#include <algorithm>
#include <format>
#include <iterator>
#include <print>

#include "constants.h"
#include "controller.h"
#include "view.h"

#include <rawterm/color.h>
#include <rawterm/core.h>
#include <rawterm/text.h>

View::View(Controller *controller, const rawterm::Pos dims)
    : ctrlr_ptr(controller), view_size(dims), cur(rawterm::Cursor()) {
    viewable_models.reserve(8);
}

void View::add_model(Model *m) {
    viewable_models.push_back(m);
    active_model = viewable_models.size();

    // Set visual offset
    if (LINE_NUMBERS) {
        int line_count = std::count(m->buf.begin(), m->buf.end(), '\n');
        line_number_offset = std::to_string(line_count).size() + 1;
    }
}

Model *View::get_active_model() const {
    return viewable_models.at(active_model - 1);
}

void View::render_screen() {
    int remaining_rows = view_size.vertical - 2;
    rawterm::Pos starting_cur_pos = cur;
    std::string screen = "";
    int line_count = get_active_model()->vertical_scroll_offset + 1;
    const int max_hor_line_len = view_size.horizontal - line_number_offset - 3;

    // Draw tab bar
    if (viewable_models.size() > 1) {
        screen += generate_tab_bar();
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
            } else {
                screen += c;
            }

            // Truncate line
            horizontal_counter++;
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

    // Draw to screen
    rawterm::clear_screen();
    cur.move({ 1, 1 });
    std::print("{}", screen);
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

const std::string View::generate_tab_bar() const {
    std::string ret = "| ";

    for (unsigned int i = 0; i < viewable_models.size(); ++i) {
        if (i == static_cast<unsigned int>(active_model - 1)) {
            ret += rawterm::inverse(viewable_models.at(i)->filename);
        } else {
            ret += viewable_models.at(i)->filename;
        }

        ret += " | ";
    }

    ret += "\n";
    return ret;
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
    std::string filename = viewable_models.at(active_model - 1)->filename;

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
        "| " + std::to_string(get_active_model()->buf.curr_line_index()) + ":" +
        std::to_string(get_active_model()->buf.curr_char_index() + 1) + " ";

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
