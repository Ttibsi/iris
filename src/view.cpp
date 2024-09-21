#include "view.h"

#include <format>

#include <rawterm/color.h>
#include <rawterm/text.h>

#include <stdexcept>
#include "constants.h"
#include "controller.h"
#include "logger.h"

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
        line_number_offset = std::to_string(m->line_count).size() + 1;
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
    int line_count = viewable_models.at(active_model - 1)->vertical_file_offset + 1;

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

    // Find starting point in gapvector
    unsigned int gv_counter =
        viewable_models.at(active_model - 1)
            ->buf.find_ith_char('\n', viewable_models.at(active_model - 1)->vertical_file_offset);

    if (gv_counter != 0) {
        gv_counter++;
    }

    while (remaining_rows) {
        if (gv_counter == viewable_models.at(active_model - 1)->buf.size() - 1) {
            break;
        }
        char c = viewable_models.at(active_model - 1)->buf.at(gv_counter);
        screen += c;

        if (c == '\n') {
            remaining_rows--;

            line_count++;
            if (LINE_NUMBERS) {
                screen += rawterm::set_foreground(
                    std::format("{:>{}}\u2502", line_count, line_number_offset), COLOR_UI_BG);
            }
        }

        gv_counter++;
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
    rawterm::Pos cur_pos = cur;

    cur.move({cur.vertical, 1});
    int current_line = get_active_model()->current_line;

    if (LINE_NUMBERS) {
        std::cout << rawterm::set_foreground(
            std::format("{:>{}}\u2502", current_line, line_number_offset), COLOR_UI_BG);
    }

    std::cout << get_active_model()->buf.line(get_active_model()->get_abs_pos());
    cur.move({cur_pos.vertical, cur_pos.horizontal + 1});
    get_active_model()->current_char_in_line++;

    // throw std::logic_error("Not Implemented");
}

void View::set_status(const std::string& msg) {
    cur.move({1, view_size.horizontal});
    rawterm::clear_line();
    std::cout << rawterm::set_foreground(msg, COLOR_NOTIFY);
}

void View::cursor_left() {
    int left_most_pos = (LINE_NUMBERS ? line_number_offset + 2 : 0);
    if (cur.horizontal > left_most_pos) {
        cur.move_left();
        viewable_models.at(active_model - 1)->current_char_in_line--;
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
        viewable_models.at(active_model - 1)->vertical_file_offset--;
        render_screen();
    } else {
        // move cursor
        cur.move_up();
    }

    viewable_models.at(active_model - 1)->current_line--;
    draw_status_bar();
}

void View::cursor_down() {
    // Check if we're at the bottom of the file
    if (viewable_models.at(active_model - 1)->current_line ==
        viewable_models.at(active_model - 1)->line_count) {
        return;
    }

    int vertical_offset = (open_files.size() > 1 ? 3 : 2);
    if (cur.vertical + 1 > view_size.vertical - vertical_offset) {
        // scroll view
        viewable_models.at(active_model - 1)->vertical_file_offset++;
        render_screen();
    } else {
        // move cursor
        cur.move_down();
    }

    viewable_models.at(active_model - 1)->current_line++;
    draw_status_bar();
}

void View::cursor_right() {
    if (cur.horizontal >= view_size.horizontal) {
        return;
    }

    char next_char = viewable_models.at(active_model - 1)->get_next_char();
    if (next_char != '\r') {
        cur.move_right();
        viewable_models.at(active_model - 1)->current_char_in_line++;
        draw_status_bar();
    }
}
