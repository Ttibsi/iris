#include "controller.h"
#include "file_io.h"
#include "twin_array.h"
#include "view.h"

#include <rawterm/core.h>
#include <rawterm/cursor.h>

#include "spdlog/spdlog.h"

Controller::Controller()
    : term_size(rawterm::get_term_size()), view(View(this, term_size)) {
    models.reserve(8);
}

void Controller::set_mode(Mode m) {
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
    mode = m;
}

const std::string Controller::get_mode() const {
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

void Controller::create_view(const std::string &file_name) {
    if (file_name.empty()) {
        models.emplace_back();
    } else {
        spdlog::get("basic_logger")
            ->info("Creating view from file: " + file_name);
        std::optional<TwinArray<char>> file_chars = open_file(file_name);

        if (file_chars.has_value()) {
            models.emplace_back(file_chars.value(), file_name);
        } else {
            models.emplace_back();
        }
    }

    view.add_model(&models.at(models.size() - 1));
}

void Controller::start_action_engine() {
    bool break_loop = false;
    bool redraw_all = true;

    while (!(break_loop)) {
        if (redraw_all) {
            view.render_screen();
            redraw_all = false;
        }

        auto k = rawterm::process_keypress();
        if (!(k.has_value())) {
            continue;
        }

        if (k.value() == rawterm::Key('q')) {
            break_loop = true;
        }
    }
}
