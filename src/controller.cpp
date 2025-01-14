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
            if (k.value() == rawterm::Key(' ', rawterm::Mod::Escape)) {
                parse_action<Mode, None>(&view, Action<Mode> {ActionType::ChangeMode, Mode::Read});

            } else if (k.value() == rawterm::Key('D', rawterm::Mod::Arrow)) {
                parse_action<void, None>(&view, Action<void> {ActionType::MoveCursorLeft});

            } else if (k.value() == rawterm::Key('B', rawterm::Mod::Arrow)) {
                auto redraw =
                    parse_action<void, bool>(&view, Action<void> {ActionType::MoveCursorDown});
                if (redraw.value()) {
                    redraw_all = true;
                }

            } else if (k.value() == rawterm::Key('A', rawterm::Mod::Arrow)) {
                auto draw = parse_action<void, bool>(&view, Action<void> {ActionType::MoveCursorUp});
                if (draw.value()) {
                    redraw_all = true;
                }

            } else if (k.value() == rawterm::Key('C', rawterm::Mod::Arrow)) {
                parse_action<void, None>(&view, Action<void> {ActionType::MoveCursorRight});

            } else if (k.value() == rawterm::Key(' ', rawterm::Mod::Backspace)) {
                auto draw = parse_action<void, Redraw>(&view, Action<void> {ActionType::Backspace});
                switch (draw.value()) {
                    case Redraw::Screen: redraw_all = true;
                    case Redraw::Line: view.draw_line();
                    case Redraw:: None: break;
                }

            } else if (k.value() == rawterm::Key('m', rawterm::Mod::Enter)) {
                parse_action<void, None>(&view, Action<void> {ActionType::Newline});
                redraw_all = true;

                // } else if (k.value() == rawterm::Key('i', rawterm::Mod::Tab))
                // {
                //     for (int i = 0; i < TAB_SIZE; i++) {
                //         parse_action<char, None>(&view, Action<char>
                //         {ActionType::InsertChar, ' '});
                //     }
                //     view.render_line();
                //     view.draw_status_bar();
            } else {
                parse_action<char, None>(
                    &view, Action<char> {ActionType::InsertChar, k.value().code});
                view.draw_line();
            }

        } else if (mode == Mode::Read) {
            if (k.value() == rawterm::Key('h')) {
                parse_action<void, None>(&view, Action<void> {ActionType::MoveCursorLeft});
            } else if (k.value() == rawterm::Key('i')) {
                parse_action<Mode, None>(&view, Action<Mode> {ActionType::ChangeMode, Mode::Write});
            } else if (k.value() == rawterm::Key('j')) {
                auto redraw =
                    parse_action<void, bool>(&view, Action<void> {ActionType::MoveCursorDown});
                if (redraw.value()) {
                    redraw_all = true;
                }
            } else if (k.value() == rawterm::Key('k')) {
                auto redraw =
                    parse_action<void, bool>(&view, Action<void> {ActionType::MoveCursorUp});
                if (redraw.value()) {
                    redraw_all = true;
                }
            } else if (k.value() == rawterm::Key('l')) {
                parse_action<void, None>(&view, Action<void> {ActionType::MoveCursorRight});
            } else if (k.value() == rawterm::Key('q')) {
                break_loop = true;
            }
        }

        // After every input, refresh the status bar
        view.draw_status_bar();
    }
}
