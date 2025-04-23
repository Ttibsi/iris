#include "view.h"

#include <algorithm>
#include <cassert>
#include <cctype>
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
        line_number_offset = static_cast<int>(std::to_string(m->buf.size()).size() + 1);
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
        const uint_t viewable_hor_len = static_cast<unsigned int>(
            view_size.horizontal - (LINE_NUMBERS ? line_number_offset + 1 : 0));

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

    for (std::size_t i = 0; i < view_models.size(); ++i) {
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
        std::print(
            "{}", render_line(get_active_model()->current_line + static_cast<uint_t>(redraw_prev)));
    }

    cur.move({cur_pos.vertical, 1});
    rawterm::clear_line();
    std::print("{}", render_line(get_active_model()->current_line));
    cur.move(cur_pos);
}

[[nodiscard]] const std::string View::render_line(const uint_t idx) const {
    std::string line = "";

    const uint_t viewable_hor_len = static_cast<unsigned int>(
        view_size.horizontal - (LINE_NUMBERS ? line_number_offset + 1 : 0));

    std::string_view curr_line = get_active_model()->buf.at(idx);

    if (LINE_NUMBERS) {
        const rawterm::Color color =
            (idx == get_active_model()->current_line) ? COLOR_DARK_YELLOW : COLOR_UI_BG;
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
    } else if (get_active_model()->undo_stack.size()) {
        left += " | [X]";
    }

    // TODO: file language after highlighting engine
    const std::string right = "| " + std::to_string(get_active_model()->current_line + 1) + ":" +
                              std::to_string(get_active_model()->current_char + 1) + " ";

    // TODO: handle overflows

    std::string viewable_filename = filename;
    const std::size_t two_thirds = static_cast<uint_t>((view_size.horizontal * 2) / 3);
    if (filename.size() > two_thirds) {
        viewable_filename = "..." + filename.substr(two_thirds + 3);
    }

    const std::size_t filename_start = static_cast<std::size_t>(std::floor(
        (view_size.horizontal / 2) - (static_cast<int>(viewable_filename.size()) / 2) -
        static_cast<int>(left.size())));

    const std::size_t filename_end = static_cast<std::size_t>(std::floor(
        (view_size.horizontal / 2) - (static_cast<int>(viewable_filename.size()) / 2) -
        static_cast<int>(right.size())));

    const std::string ret =
        left + std::string(filename_start, ' ') + viewable_filename +
        std::string(filename_end - (viewable_filename.size() % 2 ? 1 : 0), ' ') + right;

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

// TODO: investigate moving string in instead of copy
void View::display_message(std::string msg, std::optional<rawterm::Color> color) {
    rawterm::Pos prev_pos = cur;
    cur.move({view_size.vertical, 1});
    rawterm::clear_line();

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
        get_active_model()->current_char = static_cast<uint_t>(tmp - (line_number_offset + 2));
        return tmp;
    }

    const int line_pos = static_cast<int>(get_active_model()->current_line) + offset;
    std::string_view line_moving_to =
        get_active_model()->buf.at(static_cast<std::size_t>(line_pos));

    // if (static_cast<int>(line_moving_to.size()) < (cur.horizontal - line_number_offset)) {
    if (line_moving_to.size() < get_active_model()->current_char) {
        prev_cur_hor_pos = cur.horizontal;
        get_active_model()->current_char = static_cast<uint_t>(line_moving_to.size());
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

[[maybe_unused]] bool View::cursor_up(unsigned int count) {
    // If no line above, do nothing
    if (!(get_active_model()->current_line)) {
        return false;
    }

    std::optional<int> horizontal_clamp = clamp_horizontal_movement(-1);
    get_active_model()->current_line -= count;

    bool redraw_sentinal = false;

    for (unsigned int i = 0; i < count; i++) {
        if (get_active_model()->view_offset > get_active_model()->current_line) {
            // Scroll view
            get_active_model()->view_offset -= 1;
            redraw_sentinal = true;
        } else if (!(horizontal_clamp.has_value())) {
            // Move cursor
            cur.move_up();
        } else {
            cur.move(
                {static_cast<int>(cur.vertical - 1),
                 std::max(horizontal_clamp.value(), line_number_offset + 2)});
        }
    }

    return redraw_sentinal;
}

[[maybe_unused]] bool View::cursor_down(unsigned int count) {
    // If we're on the last line, do nothing
    if (get_active_model()->current_line >= get_active_model()->buf.size() - 1) {
        return false;
    }

    std::optional<int> horizontal_clamp = clamp_horizontal_movement(1);
    get_active_model()->current_line += count;

    bool redraw_sentinal = false;
    const uint_t text_view_height =
        get_active_model()->view_offset + (static_cast<uint_t>(view_size.vertical) - 2);

    for (unsigned int i = 0; i < count; i++) {
        if (get_active_model()->current_line >= text_view_height) {
            // scroll
            get_active_model()->view_offset += 1;
            redraw_sentinal = true;
        } else if (!(horizontal_clamp.has_value())) {
            // Move cursor
            cur.move_down();
        } else {
            cur.move(
                {cur.vertical + 1, std::max(horizontal_clamp.value(), line_number_offset + 2)});
        }
    }

    return redraw_sentinal;
}

void View::cursor_right() {
    // TODO: Handle line longer than view
    if (cur.horizontal == view_size.horizontal) {
        return;
    }

    // Only scroll if we're still in the line
    int line_size =
        static_cast<int>(get_active_model()->buf.at(get_active_model()->current_line).size() + 1u);

    if (LINE_NUMBERS) {
        line_size += line_number_offset + 1;
    }

    if (cur.horizontal < line_size) {
        get_active_model()->current_char++;
        cur.move_right();
    }
}

void View::cursor_end_of_line() {
    std::size_t line_len = get_active_model()->buf.at(get_active_model()->current_line).size();
    std::size_t curr_pos = get_active_model()->current_char;

    for (std::size_t i = curr_pos; i < line_len; i++) {
        cursor_right();
    }
}

void View::cursor_start_of_line() {
    const std::string& cur_line = get_active_model()->buf.at(get_active_model()->current_line);

    if (!(cur_line.empty())) {
        auto it = std::find_if(
            cur_line.begin(), cur_line.end(), [](char c) { return !(std::isspace(c)); });

        if (it != cur_line.end()) {
            while (get_active_model()->current_char > std::distance(cur_line.begin(), it)) {
                cursor_left();
            }
        }
    }
}

void View::center_current_line() {
    set_current_line(get_active_model()->current_line + 1);
}

void View::set_current_line(const unsigned int lineno) {
    if (lineno > get_active_model()->buf.size()) {
        return;
    }

    prev_cur_hor_pos = -1;
    get_active_model()->current_char = 0;
    get_active_model()->current_line = lineno - 1;

    uint_t half_view = static_cast<uint_t>(std::floor(view_size.vertical / 2));
    if (lineno < half_view) {
        get_active_model()->view_offset = 0;
        cur.move({static_cast<int>(lineno), line_number_offset + 2});
    } else {
        get_active_model()->view_offset = lineno - half_view - 1;
        cur.move({static_cast<int>(half_view + 1), line_number_offset + 2});
    }
}
