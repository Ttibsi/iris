#include "view.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <format>
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
            rawterm::Color c = COLOR_UI_BG;
            if (get_active_model()->view_offset + idx + 1 == get_active_model()->current_line + 1) {
                c = COLOR_DARK_YELLOW;
            }

            screen += rawterm::set_foreground(
                std::format(
                    "{:>{}}\u2502", get_active_model()->view_offset + idx + 1, line_number_offset),
                c);
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

void View::draw_line(const Draw_Line_dir::values redraw_prev) {
    const rawterm::Pos cur_pos = cur;

    if (redraw_prev) {
        cur.move({cur.vertical + redraw_prev, 1});
        rawterm::clear_line();
        std::print("{}", render_line(get_active_model()->current_line + redraw_prev));
    }

    cur.move({cur_pos.vertical, 1});
    rawterm::clear_line();
    std::print("{}", render_line(get_active_model()->current_line));
    cur.move(cur_pos);
}

[[nodiscard]] const std::string View::render_line(const unsigned int idx) const {
    std::string line = "";

    const unsigned int viewable_hor_len =
        view_size.horizontal - (LINE_NUMBERS ? line_number_offset + 1 : 0);
    std::string_view curr_line = get_active_model()->buf.at(idx);

    if (LINE_NUMBERS) {
        rawterm::Color color = COLOR_UI_BG;
        if (idx == get_active_model()->current_line) {
            color = COLOR_DARK_YELLOW;
        }

        line += rawterm::set_foreground(
            std::format("{:>{}}\u2502", idx + 1, line_number_offset), color);
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
    const std::string right = "| " + std::to_string(get_active_model()->current_line + 1) + ":" +
                              std::to_string(get_active_model()->current_char + 1) + " ";

    // TODO: handle overflows
    const float filename_start =
        std::floor((view_size.horizontal / 2) - (filename.size() / 2) - left.size());
    const float filename_end =
        std::floor((view_size.horizontal / 2) - (filename.size() / 2) - right.size());

    const std::string ret = left + std::string(filename_start, ' ') + filename +
                            std::string(filename_end - (filename.size() % 2 ? 1 : 0), ' ') + right;

    // assert(ret.size() == static_cast<std::size_t>(view_size.horizontal));
    return rawterm::set_background(ret, COLOR_UI_BG);
}

[[maybe_unused]] const rawterm::Pos View::draw_command_bar() {
    rawterm::Pos prev_pos = cur;
    cur.move({view_size.vertical, 1});
    rawterm::clear_line();
    std::print("{}", command_text);
    cur.move({view_size.vertical, static_cast<int>(command_text.size() + 1)});

    return prev_pos;
}

void View::display_message(std::string& msg, std::optional<rawterm::Color> color) {
    rawterm::Pos prev_pos = cur;
    cur.move({view_size.vertical, 1});

    if (color.has_value()) {
        msg = rawterm::set_foreground(msg, color.value());
    }

    std::print("{}", msg);
    cur.move(prev_pos);
}

[[nodiscard]] std::optional<int> View::clamp_horizontal_movement(const int offset) {
    // Make sure this only uses adjacent lines
    if (!(offset == 1 || offset == -1)) {
        return std::nullopt;
    }

    if (prev_cur_hor_pos > -1) {
        const int tmp = prev_cur_hor_pos;
        prev_cur_hor_pos = -1;
        get_active_model()->current_char = tmp - (line_number_offset + 2);
        return tmp;
    }

    std::string_view line_moving_to =
        get_active_model()->buf.at(get_active_model()->current_line + offset);

    // if (static_cast<int>(line_moving_to.size()) < (cur.horizontal - line_number_offset)) {
    if (line_moving_to.size() < get_active_model()->current_char) {
        prev_cur_hor_pos = cur.horizontal;
        get_active_model()->current_char = line_moving_to.size();
        return line_moving_to.size();
    }

    return std::nullopt;
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

    std::optional<int> horizontal_clamp = clamp_horizontal_movement(-1);
    get_active_model()->current_line--;

    bool redraw_sentinal = false;
    if (get_active_model()->view_offset > get_active_model()->current_line) {
        // Scroll view
        get_active_model()->view_offset--;
        redraw_sentinal = true;
    } else if (!(horizontal_clamp.has_value())) {
        // Move cursor
        cur.move_up();
    } else {
        cur.move({cur.vertical - 1, std::max(horizontal_clamp.value(), line_number_offset + 2)});
    }

    return redraw_sentinal;
}

[[maybe_unused]] bool View::cursor_down() {
    // If we're on the last line, do nothing
    if (get_active_model()->current_line >= get_active_model()->buf.size() - 1) {
        return false;
    }

    std::optional<int> horizontal_clamp = clamp_horizontal_movement(1);
    get_active_model()->current_line++;

    bool redraw_sentinal = false;
    const unsigned int text_view_height =
        get_active_model()->view_offset + (view_size.vertical - 2);

    if (get_active_model()->current_line >= text_view_height) {
        // scroll
        get_active_model()->view_offset++;
        redraw_sentinal = true;
    } else if (!(horizontal_clamp.has_value())) {
        // Move cursor
        cur.move_down();
    } else {
        cur.move({cur.vertical + 1, std::max(horizontal_clamp.value(), line_number_offset + 2)});
    }

    return redraw_sentinal;
}

void View::cursor_right() {
    // TODO: Handle line longer than view
    if (cur.horizontal == view_size.horizontal) {
        return;
    }

    // Only scroll if we're still in the line
    int line_size = get_active_model()->buf.at(get_active_model()->current_line).size() + 1;
    if (LINE_NUMBERS) {
        line_size += line_number_offset + 1;
    }

    if (cur.horizontal < line_size) {
        get_active_model()->current_char++;
        cur.move_right();
    }
}
