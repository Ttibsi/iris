#include "view.h"

#include <algorithm>
#include <format>
#include <iterator>
#include <print>
#include <ranges>
#include <string>

#include <rawterm/color.h>
#include <rawterm/core.h>
#include <rawterm/text.h>

#include "constants.h"
#include "controller.h"

View::View(Controller* controller, const rawterm::Pos dims)
    : ctrlr_ptr(controller), view_size(dims), cur(rawterm::Cursor()) {
    view_models.reserve(8);
}

void View::add_model(Model* m) {
    view_models.push_back(m);
    active_model = view_models.size() - 1;

    // Set visual offset
    if (LINE_NUMBERS) {
        line_number_offset = std::to_string(m->buf.size()).size() + 1;
    }
}

Model* View::get_active_model() const {
    return view_models.at(active_model);
}

void View::draw_screen() {
    rawterm::Pos starting_cur_pos = cur;

    // Draw to screen
    rawterm::clear_screen();
    cur.move({1, 1});
    std::print("{}", render_screen());
    draw_status_bar();
    std::print("\n");  // Notification bar

    // Cursor positioning
    cur.move(starting_cur_pos);
    if (LINE_NUMBERS && !(rawterm::detail::is_debug())) {
        while (cur.horizontal < line_number_offset + 2) {
            cur.move_right();
        }
    }
}

[[nodiscard]] const std::string View::render_screen() const {
    std::string screen;

    // Draw tab bar
    if (view_models.size() > 1) {
        screen += render_tab_bar();
    }

    // Get displayable subrange
    int end = std::min(view_size.vertical - 2, static_cast<int>(get_active_model()->buf.size()));
    auto viewable_range = get_active_model()->buf |
                          std::views::drop(get_active_model()->view_offset) | std::views::take(end);

    for (const auto [idx, line] : std::views::enumerate(viewable_range)) {
        if (LINE_NUMBERS) {
            screen += rawterm::set_foreground(
                std::format(
                    "{:>{}}\u2502", get_active_model()->view_offset + idx + 1, line_number_offset),
                COLOR_UI_BG);
        }

        // Truncate
        const unsigned int viewable_hor_len =
            view_size.horizontal - (LINE_NUMBERS ? line_number_offset + 1 : 0);

        if (line.size() > viewable_hor_len) {
            screen += line.substr(0, viewable_hor_len - 1);
            screen += "\u00BB\r\n";
        } else {
            screen += line + "\r\n";
        };
    }

    // Set 1 line number if no text in the buffer
    if (LINE_NUMBERS && !(get_active_model()->buf.size())) {
        screen += rawterm::set_foreground(
                      std::format("{:>{}}\u2502", 1, line_number_offset), COLOR_UI_BG) +
                  "\n";
        end++;
    }

    // Any empty lines populate with tildes
    while (end < view_size.vertical - 2) {
        screen += "~\r\n";
        end++;
    }

    return screen;
}

const std::string View::render_tab_bar() const {
    std::string ret = "| ";

    for (int i = 0; i < static_cast<int>(view_models.size()); ++i) {
        if (i == active_model) {
            ret += rawterm::inverse(view_models.at(i)->filename);
        } else {
            ret += view_models.at(i)->filename;
        }

        ret += " | ";
    }

    ret += "\n";
    return ret;
}

void View::draw_line() {
    rawterm::clear_line();
    const rawterm::Pos cur_pos = cur;
    cur.move({cur.vertical, 1});
    std::print("{}", render_line());
    cur.move(cur_pos);
}

[[nodiscard]] const std::string View::render_line() const {
    std::string line = "";

    const unsigned int viewable_hor_len =
        view_size.horizontal - (LINE_NUMBERS ? line_number_offset + 1 : 0);
    std::string_view curr_line = get_active_model()->buf.at(get_active_model()->current_line);

    if (LINE_NUMBERS) {
        line += rawterm::set_foreground(
            std::format("{:>{}}\u2502", get_active_model()->current_line + 1, line_number_offset),
            COLOR_UI_BG);
    }

    // Truncate
    if (curr_line.size() > viewable_hor_len) {
        line += curr_line.substr(0, viewable_hor_len - 1);
        line += "\u00BB";
    } else {
        line += curr_line;
    }

    return line;
}

// TODO: Break this up so it's easier to update specific elements in statusbar
void View::draw_status_bar() {
    rawterm::Pos starting_cur_pos = cur;
    cur.move({view_size.vertical - 1, 1});
    rawterm::clear_line();
    std::print("{}", render_status_bar());
    cur.move(starting_cur_pos);
}

// TODO: The center text isn't aligned right - it ends at the center point
const std::string View::render_status_bar() const {
    std::string filename = view_models.at(active_model)->filename;

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
    std::string right = "| " + std::to_string(get_active_model()->current_line + 1) + ":" +
                        std::to_string(get_active_model()->current_char + 1) + " ";

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

void View::cursor_left() {
    if (get_active_model()->current_char) {
        get_active_model()->current_char--;
        cur.move_left();
    }
}

[[maybe_unused]] bool View::cursor_up() {
    // If no line above, do nothing
    if (!(get_active_model()->current_line)) {
        return false;
    }

    // TODO: Clamp to length of line

    get_active_model()->current_line--;

    bool redraw_sentinal = false;
    if (get_active_model()->view_offset > get_active_model()->current_line) {
        // Scroll view
        get_active_model()->view_offset--;
        redraw_sentinal = true;
    } else {
        // Move cursor
        cur.move_up();
    }

    return redraw_sentinal;
}

[[maybe_unused]] bool View::cursor_down() {
    // If we're on the last line, do nothing
    if (get_active_model()->current_line == get_active_model()->buf.size() - 1) {
        return false;
    }

    // TODO: Clamp to length of line

    get_active_model()->current_line++;

    bool redraw_sentinal = false;
    const unsigned int text_view_height =
        get_active_model()->view_offset + (view_size.vertical - 2);

    if (get_active_model()->current_line > text_view_height) {
        // scroll
        get_active_model()->view_offset++;
        redraw_sentinal = true;
    } else {
        // Move cursor
        cur.move_down();
    }

    return redraw_sentinal;
}

void View::cursor_right() {
    // TODO: Handle line longer than view
    if (cur.horizontal == view_size.horizontal) {
        return;
    }

    // Only scroll if we're still in the line
    int line_size = get_active_model()->buf.at(get_active_model()->current_line).size();
    if (LINE_NUMBERS) {
        line_size += line_number_offset + 1;
    }

    if (cur.horizontal < line_size) {
        get_active_model()->current_char++;
        cur.move_right();
    }
}
