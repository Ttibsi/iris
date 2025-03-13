#include "controller.h"

#include <format>
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
        models.emplace_back(term_size.vertical - 2, "");
    } else {
        auto logger = spdlog::get("basic_logger");
        if (logger != nullptr) {
            logger->info("Creating view from file: " + file_name);
        }

        opt_lines_t file_chars = open_file(file_name);

        if (file_chars.has_value()) {
            models.emplace_back(file_chars.value(), file_name);
        } else {
            models.emplace_back(term_size.vertical - 2, (file_name.empty() ? "" : file_name));
        }
    }

    view.add_model(&models.at(models.size() - 1));
}

void Controller::start_action_engine() {
    bool break_loop = false;
    bool redraw_all = true;
    auto signals_thread = rawterm::enable_signals();

    while (!(break_loop)) {
        if (redraw_all) {
            view.draw_screen();
            redraw_all = false;
        } else if (quit_flag) {
            break;
        }

        auto k = rawterm::process_keypress();

        // Handle signals
        rawterm::signal_handler(rawterm::Signal::SIG_CONT, [this]() { view.draw_screen(); });
        rawterm::signal_handler(rawterm::Signal::SIG_WINCH, [this]() {
            term_size = rawterm::get_term_size();
            view.view_size = rawterm::get_term_size();
            view.draw_screen();
        });

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
                } else {
                    view.draw_line(Draw_Line_dir::Prev);
                }

            } else if (k.value() == rawterm::Key('A', rawterm::Mod::Arrow)) {
                auto draw =
                    parse_action<void, bool>(&view, Action<void> {ActionType::MoveCursorUp});
                if (draw.value()) {
                    redraw_all = true;
                } else {
                    view.draw_line(Draw_Line_dir::Next);
                }

            } else if (k.value() == rawterm::Key('C', rawterm::Mod::Arrow)) {
                parse_action<void, None>(&view, Action<void> {ActionType::MoveCursorRight});

            } else if (k.value() == rawterm::Key(' ', rawterm::Mod::Backspace)) {
                auto draw = parse_action<void, Redraw>(&view, Action<void> {ActionType::Backspace});
                switch (draw.value()) {
                    case Redraw::Screen:
                        redraw_all = true;
                        break;
                    case Redraw::Line:
                        view.draw_line(Draw_Line_dir::None);
                        break;
                    case Redraw::None:
                        break;
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
                view.draw_line(Draw_Line_dir::None);
            }

        } else if (mode == Mode::Read) {
            // Move right, then enter insert mode
            if (k.value() == rawterm::Key('a')) {
                parse_action<void, None>(&view, Action<void> {ActionType::MoveCursorRight});
                parse_action<Mode, None>(&view, Action<Mode> {ActionType::ChangeMode, Mode::Write});

                // Move cur to end of line and enter insert mode
            } else if (k.value() == rawterm::Key('A', rawterm::Mod::Shift)) {
                parse_action<void, None>(&view, Action<void> {ActionType::EndOfLine});
                parse_action<Mode, None>(&view, Action<Mode> {ActionType::ChangeMode, Mode::Write});

                // Jump to prev "word"
            } else if (k.value() == rawterm::Key('b')) {
                parse_action<void, None>(&view, Action<void> {ActionType::JumpPrevWord});

            } else if (k.value() == rawterm::Key('h')) {
                parse_action<void, None>(&view, Action<void> {ActionType::MoveCursorLeft});
            } else if (k.value() == rawterm::Key('i')) {
                parse_action<Mode, None>(&view, Action<Mode> {ActionType::ChangeMode, Mode::Write});
            } else if (k.value() == rawterm::Key('j')) {
                auto redraw =
                    parse_action<void, bool>(&view, Action<void> {ActionType::MoveCursorDown});
                if (redraw.value()) {
                    redraw_all = true;
                } else {
                    view.draw_line(Draw_Line_dir::Prev);
                }
            } else if (k.value() == rawterm::Key('k')) {
                auto redraw =
                    parse_action<void, bool>(&view, Action<void> {ActionType::MoveCursorUp});
                if (redraw.value()) {
                    redraw_all = true;
                } else {
                    view.draw_line(Draw_Line_dir::Next);
                }
            } else if (k.value() == rawterm::Key('l')) {
                parse_action<void, None>(&view, Action<void> {ActionType::MoveCursorRight});

                // add new line and go to insert mode (below)
            } else if (k.value() == rawterm::Key('o')) {
                parse_action<void, None>(&view, Action<void> {ActionType::EndOfLine});
                parse_action<void, None>(&view, Action<void> {ActionType::Newline});
                redraw_all = true;
                parse_action<Mode, None>(&view, Action<Mode> {ActionType::ChangeMode, Mode::Write});

                // add new line and go to insert mode (above)
            } else if (k.value() == rawterm::Key('O', rawterm::Mod::Shift)) {
                parse_action<void, None>(&view, Action<void> {ActionType::StartOfLine});
                parse_action<void, None>(&view, Action<void> {ActionType::Newline});
                std::ignore =
                    parse_action<void, bool>(&view, Action<void> {ActionType::MoveCursorUp});
                redraw_all = true;
                parse_action<Mode, None>(&view, Action<Mode> {ActionType::ChangeMode, Mode::Write});

                // Jump to next "word"
            } else if (k.value() == rawterm::Key('w')) {
                parse_action<void, None>(&view, Action<void> {ActionType::JumpNextWord});

                // Delete char under key
            } else if (k.value() == rawterm::Key('x')) {
                auto draw =
                    parse_action<void, Redraw>(&view, Action<void> {ActionType::DelCurrentChar});
                if (draw == Redraw::Line) {
                    view.draw_line(Draw_Line_dir::None);
                }

                // Trigger command mode
            } else if (k.value() == rawterm::Key(';')) {
                parse_action<Mode, None>(
                    &view, Action<Mode> {ActionType::ChangeMode, Mode::Command});
                view.draw_status_bar();
                enter_command_mode();
                parse_action<Mode, None>(&view, Action<Mode> {ActionType::ChangeMode, Mode::Read});

                // Move to beginning/end of line
            } else if (k.value() == rawterm::Key('_')) {
                parse_action<void, None>(&view, Action<void> {ActionType::StartOfLine});
            } else if (k.value() == rawterm::Key('$')) {
                parse_action<void, None>(&view, Action<void> {ActionType::EndOfLine});

                // Move cursor by paragraph
            } else if (k.value() == rawterm::Key('[')) {
                parse_action<void, None>(&view, Action<void> {ActionType::JumpPrevPara});
            } else if (k.value() == rawterm::Key(']')) {
                parse_action<void, None>(&view, Action<void> {ActionType::JumpNextPara});
            }
        }

        // After every input, refresh the status bar
        view.draw_status_bar();
    }

    signals_thread.join();
}

// TODO: Command history
// TODO: Autocomplete with Tab
void Controller::enter_command_mode() {
    std::optional<rawterm::Pos> prev_cursor_pos;

    while (true) {
        auto previous = view.draw_command_bar();
        if (!(prev_cursor_pos.has_value())) {
            prev_cursor_pos = previous;
        }

        auto in = rawterm::wait_for_input();

        if (in == rawterm::Key(' ', rawterm::Mod::Escape)) {
            rawterm::clear_line();
            break;
        } else if (in == rawterm::Key('m', rawterm::Mod::Enter)) {
            parse_command();
            break;
        } else if (in == rawterm::Key(' ', rawterm::Mod::Backspace)) {
            view.command_text.pop_back();
        } else if (in.isCharInput()) {
            view.command_text.push_back(in.code);
        }
    }

    view.command_text = ";";
    view.cur = prev_cursor_pos.value();
}

void Controller::parse_command() {
    std::string cmd = std::move(view.command_text);

    // Empty command
    if (cmd.size() < 2) {
        return;
    }

    if (cmd == ";wq") {
        // This just does the same as ;w and ;q
        std::ignore = write_to_file(*view.get_active_model());
        quit_flag = true;

    } else if (cmd == ";w") {
        WriteData file_write = write_to_file(*view.get_active_model());
        if (file_write.valid) {
            std::string msg =
                std::format("Saved {} bytes ({} lines)", file_write.bytes, file_write.lines);
            view.display_message(msg, rawterm::Colors::green);
        }

    } else if (cmd == ";q") {
        // TODO: Check if file is modified
        quit_flag = true;

        // ping cmd used for testing
    } else if (cmd == ";ping") {
        view.display_message(std::string("pong"), rawterm::Colors::white);

    } else {
        std::string msg = "Unknown command";
        view.display_message(msg, rawterm::Colors::red);
    }
}
