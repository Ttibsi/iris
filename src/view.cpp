#include "view.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <format>
#include <functional>
#include <print>
#include <ranges>
#include <string>
#include <thread>

#include <rawterm/color.h>
#include <rawterm/core.h>
#include <rawterm/text.h>

#include "constants.h"
#include "controller.h"
#include "text_io.h"

View::View(Controller* controller, const rawterm::Pos dims)
    : ctrlr_ptr(controller), view_size(dims), cur(rawterm::Cursor()) {
    view_models.reserve(8);
}

void View::add_model(Model* m) {
    view_models.push_back(m);
    active_model = view_models.size() - 1;
    set_lineno_offset(m);

    // Set visual offset
}

Model* View::get_active_model() const {
    return view_models.at(active_model);
}

void View::draw_screen() {
    std::jthread jth(std::bind_front(&View::get_git_branch, this));
    rawterm::Pos starting_cur_pos = cur;

    // Draw to screen
    rawterm::clear_screen();
    cur.move({1, 1});
    std::print("{}", render_screen());

    // Wait for the thread to finish
    jth.join();
    draw_status_bar();
    std::print("\n");  // Notification bar

    // Cursor positioning
    cur.move(starting_cur_pos);
    if (LINE_NUMBERS && !(rawterm::detail::is_debug())) {
        while (cur.horizontal < line_number_offset + 2) {
            cur.move_right();
        }
    }

    if (visible_tab_bar() && cur.vertical == 1) {
        cur.move_down();
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
    const int empty_space = view_size.vertical - 2 - visible_tab_bar();
    while (end < empty_space) {
        screen += "~\r\n";
        end++;
    }

    return screen;
}

// TODO: shorten file names and truncate
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

    ret += "\r\n";
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
    const std::size_t thirds = std::size_t(view_size.horizontal / 3);
    std::string filename = view_models.at(active_model)->filename;

    // left = mode | (git branch) | status
    // center = file name
    // right = language | open buffer count | cursor position

    // LHS
    std::string left = " " + ctrlr_ptr->get_mode();

    if (get_active_model()->readonly) {
        left += " | [RO]";
    } else if (get_active_model()->unsaved) {
        left += " | [X]";
    }

    if (!(git_branch.empty()) && left.size() + git_branch.size() < thirds - 2) {
        left += std::string(" | ") + git_branch;
    }

    left += " |";

    // RHS
    // TODO: file language after highlighting engine
    std::string right = "";
    if (ctrlr_ptr->models.size() > 1) {
        right += "| [" + std::to_string(ctrlr_ptr->models.size()) + "] ";
    }

    const std::string cursor_pos = "| " + std::to_string(get_active_model()->current_line + 1) +
                                   ":" + std::to_string(get_active_model()->current_char + 1) + " ";

    right += cursor_pos;

    // Center
    std::string visible_filename = filename;
    if (filename.size() >= thirds) {
        const std::size_t chars_to_remove = filename.size() - thirds + 3;  // +3 for "..."
        if (chars_to_remove <= 3) {
            // If we'd only remove 3 or fewer chars, keep the full filename instead
            visible_filename = filename;
        } else {
            visible_filename = "..." + filename.substr(chars_to_remove, filename.size());
        }
    }

    const std::size_t available_center_space = view_size.horizontal - left.size() - right.size();
    const std::size_t lhs_padding = std::clamp(
        (available_center_space - visible_filename.size()) / 2, 0ul, available_center_space);
    const int rhs_padding = int32_t(
        std::size_t(view_size.horizontal) -
        (right.size() + left.size() + lhs_padding + visible_filename.size()));

    std::string ret = left + std::string(std::size_t(lhs_padding), ' ');
    ret += visible_filename + std::string(std::size_t(rhs_padding), ' ') + right;

    assert(ret.size() == static_cast<std::size_t>(view_size.horizontal));
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

// NOTE: This has been notably complex to implement -- see commit ad20b16
// for where I previously removed functionality to restore cursor position
// after being clamped (moving the cursor right). I was experiencing many
// crashes that were down to the offset being wrong after editing text (I think)
[[nodiscard]] std::size_t View::clamp_horizontal_movement(const int offset) {
    const int line_pos = static_cast<int>(get_active_model()->current_line) + offset;
    if (line_pos < 0 || line_pos > int32_t(get_active_model()->buf.size())) {
        return 0;
    }

    std::string_view line_moving_to =
        get_active_model()->buf.at(static_cast<std::size_t>(line_pos));

    if (line_moving_to.size() < get_active_model()->current_char) {
        return get_active_model()->current_char - line_moving_to.size();
    }

    return 0;
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

    std::size_t horizontal_clamp = clamp_horizontal_movement(int32_t(-count));
    get_active_model()->current_line -= count;

    bool redraw_sentinal = false;

    for (unsigned int i = 0; i < count; i++) {
        if (get_active_model()->view_offset > get_active_model()->current_line) {
            // Scroll view
            get_active_model()->view_offset -= 1;
            redraw_sentinal = true;
        } else {
            // Move cursor
            cur.move_up();
        }
    }

    while (horizontal_clamp) {
        cursor_left();
        horizontal_clamp--;
    }

    return redraw_sentinal;
}

[[maybe_unused]] bool View::cursor_down(unsigned int count) {
    // If we're on the last line, do nothing
    if (get_active_model()->current_line >= get_active_model()->buf.size() - 1) {
        return false;
    }

    std::size_t horizontal_clamp = clamp_horizontal_movement(int32_t(count));
    get_active_model()->current_line += count;

    bool redraw_sentinal = false;
    const uint_t text_view_height =
        get_active_model()->view_offset + (static_cast<uint_t>(view_size.vertical) - 2);

    for (unsigned int i = 0; i < count; i++) {
        if (get_active_model()->current_line >= text_view_height) {
            // scroll
            get_active_model()->view_offset += 1;
            redraw_sentinal = true;
        } else {
            // Move cursor
            cur.move_down();
        }
    }

    while (horizontal_clamp) {
        cursor_left();
        horizontal_clamp--;
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

    get_active_model()->current_char = 0;
    get_active_model()->current_line = lineno - 1;

    uint_t half_view = static_cast<uint_t>(std::floor(view_size.vertical / 2));
    if (lineno <= half_view) {
        get_active_model()->view_offset = 0;
        cur.move({static_cast<int>(lineno + visible_tab_bar()), line_number_offset + 2});
    } else {
        get_active_model()->view_offset = lineno - half_view - 1;
        cur.move({static_cast<int>(half_view + 1 + visible_tab_bar()), line_number_offset + 2});
    }

    // if (view_models.size() > 1) {
    //     cur.move_down();
    // }
}

void View::get_git_branch() {
    auto resp = shell_exec("git rev-parse --abbrev-ref HEAD");
    if (resp.has_value()) {
        git_branch = resp.value().out;
    }
}

void View::tab_new() {
    ctrlr_ptr->models.emplace_back(view_size.vertical, "NO NAME");
    add_model(&ctrlr_ptr->models.at(ctrlr_ptr->models.size() - 1));
    cur.move(2, line_number_offset + 2);
}

void View::tab_next() {
    if (active_model == view_models.size() - 1) {
        active_model = 0;
    } else {
        active_model++;
    }

    set_lineno_offset(get_active_model());
    change_model_cursor();
}

void View::tab_prev() {
    if (active_model == 0) {
        active_model = view_models.size() - 1;
    } else {
        active_model--;
    }

    set_lineno_offset(get_active_model());
    change_model_cursor();
}

[[nodiscard]] uint_t View::visible_tab_bar() const {
    return view_models.size() > 1 ? 1 : 0;
}

[[maybe_unused]] int View::set_lineno_offset(Model* m) {
    if (LINE_NUMBERS) {
        line_number_offset = static_cast<int>(std::to_string(m->buf.size()).size() + 1);
        return line_number_offset;
    }

    return 0;
}

void View::change_model_cursor() {
    uint_t vertical =
        get_active_model()->current_line - get_active_model()->view_offset + visible_tab_bar() + 1;
    uint_t horizontal = get_active_model()->current_char + uint_t(line_number_offset) + 2;

    cur.move(int(vertical), int(horizontal));
}
