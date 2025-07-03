#include "controller.h"

#include <algorithm>
#include <format>
#include <ranges>

#include <rawterm/core.h>
#include <rawterm/cursor.h>
#include <rawterm/text.h>

#include "action.h"
#include "constants.h"
#include "spdlog/spdlog.h"
#include "view.h"

Controller::Controller() : term_size(rawterm::get_term_size()), view(View(this, term_size)) {
    models.reserve(8);
    meta_buffers.reserve(8);
}

void Controller::set_mode(Mode m) {
    if (view.view_models.size() && view.get_active_model()->readonly && m == Mode::Write) {
        return;
    }

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

void Controller::create_view(const std::string& file_name, unsigned long lineno) {
    if (file_name.empty()) {
        models.emplace_back(term_size.vertical - 2, "NO NAME");
        view.add_model(&models.at(models.size() - 1));
    } else {
        auto logger = spdlog::get("basic_logger");
        if (logger != nullptr) {
            logger->info("Creating view from file: " + file_name);
        }

        opt_lines_t file_chars = open_file(file_name);

        if (file_chars.has_value()) {
            models.emplace_back(file_chars.value(), file_name);
            view.add_model(&models.at(models.size() - 1));

            if (lineno) {
                lineno = std::min(lineno, models.at(models.size() - 1).buf.size());
                view.cursor_down(uint32_t(std::min(lineno - 1, file_chars.value().size())));
                view.center_current_line();
            }
        } else {
            models.emplace_back(term_size.vertical - 2, (file_name.empty() ? "" : file_name));
            view.add_model(&models.at(models.size() - 1));
        }
    }
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

            } else if (k.value() == rawterm::Key('i', rawterm::Mod::Tab)) {
                for (int i = 0; i < TAB_SIZE; i++) {
                    parse_action<char, None>(&view, Action<char> {ActionType::InsertChar, ' '});
                }
                view.draw_line(Draw_Line_dir::None);
            } else {
                parse_action<char, None>(
                    &view, Action<char> {ActionType::InsertChar, k.value().code});
                view.draw_line(Draw_Line_dir::None);
            }

        } else if (mode == Mode::Read) {
            // Move right, then enter insert mode
            if (k.value() == rawterm::Key('a')) {
                if (is_readonly_model()) {
                    continue;
                }

                parse_action<void, None>(&view, Action<void> {ActionType::MoveCursorRight});
                parse_action<Mode, None>(&view, Action<Mode> {ActionType::ChangeMode, Mode::Write});

                // Move cur to end of line and enter insert mode
            } else if (k.value() == rawterm::Key('A', rawterm::Mod::Shift)) {
                if (is_readonly_model()) {
                    continue;
                }

                parse_action<void, None>(&view, Action<void> {ActionType::EndOfLine});
                parse_action<Mode, None>(&view, Action<Mode> {ActionType::ChangeMode, Mode::Write});

                // Jump to prev "word"
            } else if (k.value() == rawterm::Key('b')) {
                parse_action<void, None>(&view, Action<void> {ActionType::JumpPrevWord});

                // find forward
            } else if (k.value() == rawterm::Key('f')) {
                auto k2 = rawterm::wait_for_input();
                if (k2.isCharInput() || (k2.code == ' ' && k2.getMod() == rawterm::Mod::Space)) {
                    parse_action<char, None>(&view, Action<char> {ActionType::FindNext, k2.code});
                }

                // find backward
            } else if (k.value() == rawterm::Key('F', rawterm::Mod::Shift)) {
                auto k2 = rawterm::wait_for_input();
                if (k2.isCharInput() || (k2.code == ' ' && k2.getMod() == rawterm::Mod::Space)) {
                    parse_action<char, None>(&view, Action<char> {ActionType::FindPrev, k2.code});
                }

                // Move to top of file
            } else if (k.value() == rawterm::Key('g')) {
                std::size_t count = view.get_active_model()->current_line;
                for (std::size_t i = 0; i < count; i++) {
                    std::ignore =
                        parse_action<void, bool>(&view, Action<void> {ActionType::MoveCursorUp});
                }
                redraw_all = true;

                // Move to bottom of file
            } else if (k.value() == rawterm::Key('G', rawterm::Mod::Shift)) {
                std::size_t count =
                    view.get_active_model()->buf.size() - view.get_active_model()->current_line;
                for (std::size_t i = 0; i < count; i++) {
                    std::ignore =
                        parse_action<void, bool>(&view, Action<void> {ActionType::MoveCursorDown});
                }
                redraw_all = true;

            } else if (k.value() == rawterm::Key('h')) {
                parse_action<void, None>(&view, Action<void> {ActionType::MoveCursorLeft});
            } else if (k.value() == rawterm::Key('i')) {
                if (is_readonly_model()) {
                    continue;
                }

                parse_action<Mode, None>(&view, Action<Mode> {ActionType::ChangeMode, Mode::Write});
            } else if (k.value() == rawterm::Key('j')) {
                auto redraw =
                    parse_action<void, bool>(&view, Action<void> {ActionType::MoveCursorDown});
                if (redraw.value()) {
                    redraw_all = true;
                } else {
                    view.draw_line(Draw_Line_dir::Prev);
                }

            } else if (k.value() == rawterm::Key('J', rawterm::Mod::Shift)) {
                if (is_readonly_model()) {
                    continue;
                }

                parse_action<void, None>(&view, Action<void> {ActionType::MoveLineDown});
                redraw_all = true;

            } else if (k.value() == rawterm::Key('k')) {
                auto redraw =
                    parse_action<void, bool>(&view, Action<void> {ActionType::MoveCursorUp});
                if (redraw.value()) {
                    redraw_all = true;
                } else {
                    view.draw_line(Draw_Line_dir::Next);
                }

            } else if (k.value() == rawterm::Key('K', rawterm::Mod::Shift)) {
                if (is_readonly_model()) {
                    continue;
                }

                parse_action<void, None>(&view, Action<void> {ActionType::MoveLineUp});
                redraw_all = true;

            } else if (k.value() == rawterm::Key('l')) {
                parse_action<void, None>(&view, Action<void> {ActionType::MoveCursorRight});

                // add new line and go to insert mode (below)
            } else if (k.value() == rawterm::Key('o')) {
                if (is_readonly_model()) {
                    continue;
                }

                parse_action<void, None>(&view, Action<void> {ActionType::EndOfLine});
                parse_action<void, None>(&view, Action<void> {ActionType::Newline});
                redraw_all = true;
                parse_action<Mode, None>(&view, Action<Mode> {ActionType::ChangeMode, Mode::Write});

                // add new line and go to insert mode (above)
            } else if (k.value() == rawterm::Key('O', rawterm::Mod::Shift)) {
                if (is_readonly_model()) {
                    continue;
                }

                uint32_t horizontal_cursor_pos = view.get_active_model()->current_char;
                while (horizontal_cursor_pos) {
                    parse_action<void, None>(&view, Action<void> {ActionType::MoveCursorLeft});
                    horizontal_cursor_pos--;
                }

                parse_action<void, None>(&view, Action<void> {ActionType::Newline});
                std::ignore =
                    parse_action<void, bool>(&view, Action<void> {ActionType::MoveCursorUp});
                redraw_all = true;
                parse_action<Mode, None>(&view, Action<Mode> {ActionType::ChangeMode, Mode::Write});

                // Replace current char
            } else if (k.value() == rawterm::Key('r')) {
                if (is_readonly_model()) {
                    continue;
                }

                auto k2 = rawterm::wait_for_input();
                if (k2.isCharInput()) {
                    parse_action<char, None>(
                        &view, Action<char> {ActionType::ReplaceChar, k2.code});
                }
                view.draw_line(Draw_Line_dir::None);

                // redo
            } else if (k.value() == rawterm::Key('R', rawterm::Mod::Shift)) {
                if (is_readonly_model()) {
                    continue;
                }

                auto ret = parse_action<void, bool>(&view, Action<void> {ActionType::TriggerRedo});
                if (ret.has_value()) {
                    redraw_all = ret.value();
                }

                // tabs
            } else if (k.value() == rawterm::Key('t')) {
                auto k2 = rawterm::wait_for_input();

                if (k2 == rawterm::Key('t')) {
                    parse_action<void, None>(&view, Action<void> {ActionType::TabNew});
                } else if (k2 == rawterm::Key('n')) {
                    parse_action<void, None>(&view, Action<void> {ActionType::TabNext});
                } else if (k2 == rawterm::Key('p')) {
                    parse_action<void, None>(&view, Action<void> {ActionType::TabPrev});
                }

                redraw_all = true;

                // Undo
            } else if (k.value() == rawterm::Key('u')) {
                if (is_readonly_model()) {
                    continue;
                }

                auto ret = parse_action<void, bool>(&view, Action<void> {ActionType::TriggerUndo});
                if (ret.has_value()) {
                    redraw_all = ret.value();
                }

                // Jump to next "word"
            } else if (k.value() == rawterm::Key('w')) {
                parse_action<void, None>(&view, Action<void> {ActionType::JumpNextWord});

                // Delete char under key
            } else if (k.value() == rawterm::Key('x')) {
                if (is_readonly_model()) {
                    continue;
                }

                auto draw =
                    parse_action<void, Redraw>(&view, Action<void> {ActionType::DelCurrentChar});
                if (draw == Redraw::Line) {
                    view.draw_line(Draw_Line_dir::None);
                }

                // Center current line on view
            } else if (k.value() == rawterm::Key('z')) {
                parse_action<void, None>(&view, Action<void> {ActionType::CenterCurrentLine});
                redraw_all = true;

                // Trigger command mode
            } else if (k.value() == rawterm::Key(';')) {
                parse_action<Mode, None>(
                    &view, Action<Mode> {ActionType::ChangeMode, Mode::Command});
                view.draw_status_bar();
                redraw_all = enter_command_mode();
                if (quit_flag) {
                    break;
                }
                parse_action<Mode, None>(&view, Action<Mode> {ActionType::ChangeMode, Mode::Read});

                // Move to beginning/end of line
            } else if (k.value() == rawterm::Key('_')) {
                parse_action<void, None>(&view, Action<void> {ActionType::StartOfLine});
            } else if (k.value() == rawterm::Key('$')) {
                parse_action<void, None>(&view, Action<void> {ActionType::EndOfLine});

                // Move cursor by paragraph
            } else if (k.value() == rawterm::Key('[')) {
                parse_action<void, None>(&view, Action<void> {ActionType::JumpPrevPara});
                redraw_all = true;
            } else if (k.value() == rawterm::Key(']')) {
                parse_action<void, None>(&view, Action<void> {ActionType::JumpNextPara});
                redraw_all = true;

                // toggle case of char
            } else if (k.value() == rawterm::Key('~')) {
                if (is_readonly_model()) {
                    continue;
                }

                parse_action<void, None>(&view, Action<void> {ActionType::ToggleCase});
                view.draw_line(Draw_Line_dir::None);
            }
        }

        // After every input, refresh the status bar and tab bar
        // If we redraw on the next loop, we'll trigger these anyway
        if (!redraw_all) {
            view.draw_status_bar();
            view.draw_tab_bar();
        }
    }

    signals_thread.join();
}

// TODO: Command history
// TODO: Autocomplete with Tab
bool Controller::enter_command_mode() {
    std::optional<rawterm::Pos> prev_cursor_pos;
    bool ret = false;

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
            ret = parse_command();
            break;
        } else if (in == rawterm::Key(' ', rawterm::Mod::Backspace)) {
            view.command_text.pop_back();
        } else {
            view.command_text.push_back(in.code);
        }
    }

    view.command_text = ";";
    if (!ret) {
        view.cur = prev_cursor_pos.value();
    }

    return ret;
}

bool Controller::parse_command() {
    std::string cmd = std::move(view.command_text);
    auto logger = spdlog::get("basic_logger");
    if (logger != nullptr) {
        logger->info("Iris CMD called: " + cmd);
    }

    // Empty command
    if (cmd.size() < 2) {
        return false;
    }

    if (std::isdigit(cmd.at(1))) {
        const unsigned int offset = uint32_t(std::stoi(cmd.substr(1, cmd.size())));
        view.set_current_line(offset);
        return true;

        // edit a new file
    } else if (cmd.substr(0, 2) == ";e" && cmd.size() > 3 && cmd.at(2) == ' ') {
        add_model(cmd.substr(3, cmd.size()));
        return true;

    } else if (cmd.substr(0, 2) == ";b" && cmd.size() > 2) {
        std::size_t bufnr = 0;

        try {
            bufnr = std::stoul(cmd.substr(2, cmd.size() - 1));
        } catch (const std::invalid_argument& e) {
            std::string msg = "Unknown bufnr provided";
            view.display_message(msg, rawterm::Colors::red);
            return false;
        }

        return view.set_buffer(bufnr, models.size());

    } else if (cmd == ";wq") {
        // This just does the same as ;w and ;q
        std::ignore = write_to_file(view.get_active_model());
        return quit_app(false);

    } else if (cmd == ";wa") {
        WriteAllData write_data = write_all();
        if (write_data.valid) {
            std::string msg = std::format("Saved {} files", write_data.files);
            view.display_message(msg, rawterm::Colors::green);
        } else {
            view.display_message("Error with saving files", rawterm::Colors::red);
        }

    } else if (cmd == ";w") {
        WriteData file_write = write_to_file(view.get_active_model());
        if (file_write.valid) {
            std::string msg =
                std::format("Saved {} bytes ({} lines)", file_write.bytes, file_write.lines);
            view.display_message(msg, rawterm::Colors::green);
        }

    } else if (cmd == ";qa") {
        auto ret = quit_all();
        switch (ret) {
            using enum QuitAll;
            case Close:
                quit_flag = true;
                return false;
            case Redraw:
                view.change_model_cursor();
                return true;
            default:
                return false;
        };

    } else if (cmd == ";q") {
        return quit_app(false);

    } else if (cmd == ";q!") {
        return quit_app(true);

        // ping cmd used for testing
    } else if (cmd == ";ping") {
        view.display_message(std::string("pong"), rawterm::Colors::white);

    } else if (cmd == ";ls") {
        view.display_message(std::string("Did you mean: `lb`"), rawterm::Colors::red);
    } else if (cmd == ";lb") {
        display_all_buffers();

    } else {
        std::string msg = "Unknown command";
        view.display_message(msg, rawterm::Colors::red);
    }

    return false;
}

[[nodiscard]] bool Controller::is_readonly_model() {
    return view.get_active_model()->readonly;
}

[[nodiscard]] bool Controller::quit_app(bool skip_check) {
    if (view.visible_tab_bar()) {
        if (check_for_saved_file(skip_check)) {
            view.view_models.erase(view.view_models.begin() + std::ptrdiff_t(view.active_model));

            if (view.active_model > 0) {
                view.active_model--;
            } else {
                view.active_model = view.view_models.size() - 1;
            }

            view.change_model_cursor();

            return true;
        }
    } else {
        // Only one tab open
        quit_flag = check_for_saved_file(skip_check);
    }

    return false;
}

[[nodiscard]] bool Controller::check_for_saved_file(bool skip) {
    if (!skip && view.get_active_model()->unsaved) {
        view.display_message("Unsaved changes. Use `;q!` to discard", rawterm::Colors::red);
        return false;
    }

    return true;
}

void Controller::add_model(const std::string& filename) {
    opt_lines_t contents = open_file(filename);
    if (contents.has_value()) {
        models.emplace_back(contents.value(), filename);
    } else {
        models.emplace_back(term_size.vertical - 2, filename);
    }

    view.cur.move(
        int(1 + view.visible_tab_bar()), 1 + view.set_lineno_offset(&models.at(models.size() - 1)));
    view.view_models.at(view.active_model) = &models.at(models.size() - 1);
}

[[nodiscard]] WriteAllData Controller::write_all() {
    WriteAllData write_all_data = {};

    for (auto&& m : models) {
        if (m.filename == "NO NAME") {
            continue;
        }

        if (write_to_file(&m).valid) {
            write_all_data.files++;
        } else {
            write_all_data.valid = false;
            return write_all_data;
        }
    }

    write_all_data.valid = true;
    return write_all_data;
}

[[nodiscard]] QuitAll Controller::quit_all() {
    // remove every model that's saved
    models.erase(
        std::remove_if(
            models.begin(), models.end(),
            [](const Model& m) { return !m.unsaved || m.filename == "NO NAME"; }),
        models.end());

    if (!models.size()) {
        view.view_models.clear();
        return QuitAll::Close;
    }

    // Clear the pointers
    view.view_models.clear();
    for (auto& model : models) {
        view.view_models.push_back(&model);
    }

    // If there's anything left, display to the user
    if (view.view_models.size()) {
        view.active_model = 0;
        return QuitAll::Redraw;
    }

    return QuitAll::Close;
}

// TODO: Break this into a render function too (return list of string)
void Controller::display_all_buffers() {
    meta_buffers.emplace_back(term_size.vertical - 2, "");
    Model* list = &meta_buffers.at(meta_buffers.size() - 1);

    list->type = ModelType::META;
    list->filename = "[BUFFERS]";
    list->readonly = true;
    list->buf.reserve(8);

    std::size_t max_name_len =
        std::max_element(models.begin(), models.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.filename.size() < rhs.filename.size();
        })->filename.size();

    std::string title = "\u2551 id \u2502  filename " + std::string(max_name_len - 8, ' ');
    title += " \u2502  pos  \u2551";
    std::size_t title_len = title.size() - 9;

    // header border
    std::string top_border = "\u2554";
    for (std::size_t i = 0; i < title_len - 1; i++) {
        top_border += "\u2550";
    }
    top_border += "\u2557";
    list->buf.at(0) = top_border;

    // header
    list->buf.push_back(title);
    list->buf.push_back("\u2551");
    for (std::size_t i = 0; i < title_len - 1; i++) {
        list->buf.at(2) += "\u2500";
    }
    list->buf.at(2) += "\u2551";

    for (const auto&& [idx, m] : std::views::enumerate(models)) {
        std::string line = "\u2551  " + std::to_string(idx);
        line += " \u2502  ";
        line += rawterm::bold(m.filename);
        if (m.unsaved) {
            line += rawterm::bold("*");
            if (m.filename.size() < max_name_len) {
                line += std::string(max_name_len - m.filename.size() - 1, ' ');
            }
        } else {
            line += std::string(max_name_len - m.filename.size() + 1, ' ');
        }

        line += " \u2502  ";
        line += std::to_string(m.current_line + 1);
        line.push_back(':');
        line += std::to_string(m.current_char + 1);

        std::size_t diff = title.size() - rawterm::raw_size(line) - std::string("\u2551").size();
        line += std::string(diff, ' ');
        line += "\u2551";

        list->buf.push_back(line);
    }

    // bottom border
    std::string btm_border = "\u255A";
    for (std::size_t i = 0; i < title_len - 1; i++) {
        btm_border += "\u2550";
    }
    btm_border += "\u255D";
    list->buf.push_back(btm_border);

    view.view_models.push_back(list);
    view.active_model++;
    view.draw_screen();
}
