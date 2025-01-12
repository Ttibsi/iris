#include "controller.h"

#include <string>

#include <rawterm/core.h>
#include <rawterm/cursor.h>

#include "action.h"
#include "spdlog/spdlog.h"
#include "text_io.h"
#include "view.h"

Controller::Controller() : term_size(rawterm::get_term_size()), view(View(this, term_size)) {
    models.reserve(8);
}

void Controller::set_mode(Mode m) {
    mode = m;
    switch (m) {
        case Mode::Read:
            rawterm::Cursor::cursor_block();
            break;
        case Mode::Write:
            rawterm::Cursor::cursor_pipe();
            break;
        case Mode::Command:
            rawterm::Cursor::cursor_block();
            break;
    }
}

[[nodiscard]] const std::string Controller::get_mode() const {
    switch (mode) {
        case Mode::Read:
            return "READ";
        case Mode::Write:
            return "WRITE";
        case Mode::Command:
            return "COMMAND";
        default:
            return "";
    }
}

void Controller::create_view(const std::string& file_name) {
    if (file_name.empty()) {
        models.emplace_back(term_size.vertical - 2);
    } else {
        auto logger = spdlog::get("basic_logger");
        if (logger != nullptr) {
            logger->info("Creating view from file: " + file_name);
        }

        opt_lines_t file_chars = open_file(file_name);

        if (file_chars.has_value()) {
            models.emplace_back(file_chars.value(), file_name);
        } else {
            models.emplace_back(term_size.vertical - 2);
        }
    }

    view.add_model(&models.at(models.size() - 1));
}

void Controller::start_action_engine() {
    bool break_loop = false;
    bool redraw_all = true;

    while (!(break_loop)) {
        if (redraw_all) {
            view.draw_screen();
            redraw_all = false;
        }

        auto k = rawterm::process_keypress();
        if (!(k.has_value())) {
            continue;
        }

        if (mode == Mode::Write) {
        } else if (mode == Mode::Read) {
        }

        // After every input, refresh the status bar
        view.draw_status_bar();
    }
}
