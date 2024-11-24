#include "controller.h"

#include <optional>

#include "action.h"
#include "constants.h"
#include "file_io.h"
#include "gapbuffer.h"
#include "logger.h"

Controller::Controller() : term_size(rawterm::get_term_size()), view(View(this, term_size)) {
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

void Controller::create_view(const std::string& file_name) {
    if (file_name.empty()) {
        models.emplace_back();
    } else {
        log("Creating view from file: " + file_name);
        std::optional<Gapbuffer> file_chars = open_file(file_name);

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

        if (mode == Mode::Write) {
            if (k.value() == rawterm::Key(' ', rawterm::Mod::Escape)) {
                parse_action<Mode, None>(&view, Action<Mode> {ActionType::ChangeMode, Mode::Read});
            } else if (k.value() == rawterm::Key('D', rawterm::Mod::Arrow)) {
                parse_action<void, None>(&view, Action<void> {ActionType::MoveCursorLeft});
            } else if (k.value() == rawterm::Key('B', rawterm::Mod::Arrow)) {
                parse_action<void, None>(&view, Action<void> {ActionType::MoveCursorDown});
            } else if (k.value() == rawterm::Key('A', rawterm::Mod::Arrow)) {
                parse_action<void, None>(&view, Action<void> {ActionType::MoveCursorUp});
            } else if (k.value() == rawterm::Key('C', rawterm::Mod::Arrow)) {
                parse_action<void, None>(&view, Action<void> {ActionType::MoveCursorRight});
            } else if (k.value() == rawterm::Key(' ', rawterm::Mod::Backspace)) {
                parse_action<void, None>(&view, Action<void> {ActionType::Backspace});
            } else if (k.value() == rawterm::Key('m', rawterm::Mod::Enter)) {
                parse_action<void, None>(&view, Action<void> {ActionType::Newline});
            } else if (k.value() == rawterm::Key('i', rawterm::Mod::Tab)) {
                for (int i = 0; i < TAB_SIZE; i++) {
                    parse_action<char, None>(&view, Action<char> {ActionType::InsertChar, ' '});
                }
                view.render_line();
                view.draw_status_bar();
            } else {
                parse_action<char, None>(
                    &view, Action<char> {ActionType::InsertChar, k.value().code});
                view.render_line();
                view.draw_status_bar();
                continue;
            }
        }

        // TODO: snap cursor to the right when you go up/down
        if (k.value() == rawterm::Key('h')) {
            parse_action<void, None>(&view, Action<void> {ActionType::MoveCursorLeft});
        } else if (k.value() == rawterm::Key('i')) {
            parse_action<Mode, None>(&view, Action<Mode> {ActionType::ChangeMode, Mode::Write});
        } else if (k.value() == rawterm::Key('j')) {
            parse_action<void, None>(&view, Action<void> {ActionType::MoveCursorDown});
        } else if (k.value() == rawterm::Key('k')) {
            parse_action<void, None>(&view, Action<void> {ActionType::MoveCursorUp});
        } else if (k.value() == rawterm::Key('l')) {
            parse_action<void, None>(&view, Action<void> {ActionType::MoveCursorRight});
        } else if (k.value() == rawterm::Key('q')) {
            break_loop = true;
        } else if (k.value() == rawterm::Key('s')) {
            parse_action<void, None>(&view, Action<void> {ActionType::SaveFile});
        }
    }
}
