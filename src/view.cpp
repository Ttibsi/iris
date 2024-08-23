#include "view.h"

#include <filesystem>
#include <stdexcept>

#include <rawterm/color.h>
#include <rawterm/text.h>

#include "constants.h"
#include "controller.h"

View::View(const Controller* controller, const rawterm::Pos dims)
    : ctrlr_ptr(controller), view_size(dims), cur(rawterm::Cursor()) {
    open_files.reserve(8);
    viewable_models.reserve(8);
}

void View::add_model(Model* m) {
    viewable_models.push_back(m);
    open_files.push_back({m->file_name, rawterm::Pos(1, 1)});
    active_model = viewable_models.size();
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
    std::string screen = "";

    rawterm::clear_screen();
    cur.move({1, 1});

    // Draw tab bar
    if (open_files.size() > 1) {
        screen += generate_tab_bar();
        remaining_rows--;
    }

    int gv_counter = 0;
    // Render gapvector into a single string
    while (remaining_rows) {
        char c = viewable_models.at(active_model - 1)->buf.at(gv_counter);

        if (c == '\n') {
            screen += "\r";
            remaining_rows--;
        }

        screen += c;
        gv_counter++;
    }

    std::cout << screen;
    std::cout << render_status_bar();
    std::cout << "\n";  // Notification bar

    // Place cursor in right place
    if (open_files.size() > 1) {
        cur.move(open_files.at(active_model - 1).pos + rawterm::Pos(0, 1));
    } else {
        cur.move(open_files.at(active_model - 1).pos);
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

const std::string View::render_status_bar() const {
    std::string filename = open_files.at(active_model - 1).filename;

    // left = mode | git branch | status (read_only/modified)
    // center = file name
    // right = language | cursor position
    std::string left = " " + ctrlr_ptr->get_mode();
    if (!(ctrlr_ptr->git_branch.empty())) {
        left += " | " + ctrlr_ptr->git_branch;
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
    auto temp_file = std::filesystem::path(filename);

    std::string ret =
        left + std::string(divide - static_cast<int>(filename.size() / 2), ' ') +
        ((filename.size() > static_cast<unsigned int>(view_size.horizontal / 3))
             ? temp_file.filename().string()
             : filename) +
        std::string(
            divide + static_cast<int>(filename.size() / 2) + !(floorf(divide) == divide), ' ') +
        right;

    return rawterm::set_background(ret, COLOR_UI_BG);
}

void View::render_line() {
    throw std::logic_error("Not Implemented");
}

void View::set_status(const std::string& msg) {
    cur.move({1, view_size.horizontal});
    rawterm::clear_line();
    std::cout << rawterm::set_foreground(msg, COLOR_NOTIFY);
}
