#include <algorithm>
#include <string>
#include <vector>

#include <rawterm/rawterm.h>

#include "constants.h"
#include "logger.h"
#include "signal_tracker.h"
#include "text_manip.h"
#include "viewport.h"

// TODO: Make sure when you resize you're in the right mode

void Viewport::keypress_read() {
    while (true) {
        if (buffer->quit_buf || resize_flag ||
            SignalTracker<SIGWINCH>::checkAndClear()) {
            break;
        }

        rawterm::sigcont_handler([this]() {
            rawterm::move_cursor({ 1, 1 });
            draw(buffer->current_line - cursor.row + 1);
            cursor.set_pos_abs(cursor.row, cursor.col, buffer->lineno_offset);
        });

        rawterm::Key k = rawterm::process_keypress();
        rawterm::Mod modifier = rawterm::getMod(&k);
        if (buffer->bang_cmd_output) {
            rawterm::clear_screen();
            cursor.set_pos_abs(1, 1, 0);
            draw(buffer->current_line);
            cursor.set_pos_abs(1, cursor.col, buffer->lineno_offset);
            buffer->reset_status_bar(view_size, &cursor);
            buffer->bang_cmd_output = false;
            continue;
        }
        // Insert mode
        if (k.code == 'i' && modifier == rawterm::Mod::None) {
            switch_to_insert();

        } else if (k.code == 'a' && modifier == rawterm::Mod::None) {
            cursor.set_pos_rel(0, 1, buffer->lineno_offset);
            switch_to_insert();

        } else if (k.code == 'A' && modifier == rawterm::Mod::Shift) {
            std::size_t col_pos =
                line_size(buffer->lines[buffer->current_line]) + 1;
            if (col_pos > view_size.horizontal) {
                horizontal_offset = col_pos - view_size.horizontal;
            }
            cursor.set_pos_abs(cursor.row, col_pos, buffer->lineno_offset);
            switch_to_insert();

            // Left
        } else if (k.code == 'h' && modifier == rawterm::Mod::None) {
            std::size_t tab_count =
                line_size(buffer->lines[buffer->current_line]) -
                buffer->lines[buffer->current_line].size();

            if (cursor.col == buffer->lineno_offset && horizontal_offset > 0) {
                horizontal_offset--;
                Cursor tmp = cursor;

                rawterm::clear_screen();
                cursor.set_pos_abs(1, 1, 0);
                draw(buffer->current_line);
                cursor.set_pos_abs(tmp.row, tmp.col, buffer->lineno_offset);
                buffer->reset_status_bar(view_size, &cursor);
            } else if (cursor.col > tab_count) {
                cursor.set_pos_rel(0, -1, buffer->lineno_offset);
                buffer->reset_status_bar(view_size, &cursor);
            }

            // Down
        } else if (k.code == 'j' && modifier == rawterm::Mod::None) {
            cursor_down(1);

            // Up
        } else if (k.code == 'k' && modifier == rawterm::Mod::None) {
            cursor_up(1);

            // Right
        } else if (k.code == 'l' && modifier == rawterm::Mod::None) {
            if (cursor.col + horizontal_offset <
                line_size(buffer->lines[buffer->current_line])) {
                if (cursor.col == view_size.horizontal) {
                    // TODO: test inputting new char in weird position
                    horizontal_offset++;
                    Cursor tmp = cursor;

                    rawterm::clear_screen();
                    cursor.set_pos_abs(1, 1, 0);
                    draw(buffer->current_line);
                    cursor.set_pos_abs(tmp.row, tmp.col, buffer->lineno_offset);
                    buffer->reset_status_bar(view_size, &cursor);

                } else {
                    cursor.set_pos_rel(0, 1, buffer->lineno_offset);
                    buffer->reset_status_bar(view_size, &cursor);
                }
            }

            // vertical movement
        } else if (k.code == '[' && modifier == rawterm::Mod::None) {
            buffer->current_line = 0;
            draw(0);
            cursor.set_pos_abs(1, 1, buffer->lineno_offset);
            buffer->reset_status_bar(view_size, &cursor);

        } else if (k.code == ']' && modifier == rawterm::Mod::None) {
            buffer->current_line = buffer->lines.size() - 1;
            draw(buffer->current_line - view_size.vertical + 1);
            cursor.set_pos_abs(view_size.vertical, 1, buffer->lineno_offset);
            buffer->reset_status_bar(view_size, &cursor);

        } else if (k.code == '{' && modifier == rawterm::Mod::None) {
            int ptr = buffer->current_line - 1;
            while (!(buffer->lines[ptr].empty())) {
                ptr--;
            }
            cursor_up(buffer->current_line - ptr);

        } else if (k.code == '}' && modifier == rawterm::Mod::None) {
            int ptr = buffer->current_line + 1;
            while (!(buffer->lines[ptr].empty())) {
                ptr++;
            }
            cursor_down(ptr - buffer->current_line);

            // Command mode
        } else if (k.code == ';' && modifier == rawterm::Mod::None) {
            switch_to_command();

            // Find and replace
        } else if (k.code == 'f' && modifier == rawterm::Mod::None) {
            buffer->editor->set_mode(Mode::Command);
            buffer->reset_status_bar(view_size, &cursor);

            auto new_loc = buffer->find_cmd();
            if (!(new_loc)) {
                continue;
            }

            cursor.set_pos_abs(buffer->current_line + new_loc->vertical + 1,
                               new_loc->horizontal + 1, buffer->lineno_offset);
            buffer->current_line += new_loc->vertical;
            draw(buffer->current_line - (view_size.vertical / 2) + 1);
            center(buffer->current_line + 1);
            buffer->editor->set_mode(Mode::Read);
            buffer->reset_status_bar(view_size, &cursor);

        } else if (k.code == 'r' && modifier == rawterm::Mod::None) {
            // Replace up to the next whitespace char
            buffer->editor->set_mode(Mode::Command);
            buffer->reset_status_bar(view_size, &cursor);

            int retcode = buffer->replace_cmd();
            if (retcode) {
                buffer->modified = true;
                redraw_line();
            }

            buffer->editor->set_mode(Mode::Read);
            buffer->reset_status_bar(view_size, &cursor);

            // cursor manipulation
        } else if (k.code == 'w' && modifier == rawterm::Mod::None) {

            std::size_t col = find_next_whitespace(
                buffer->lines[buffer->current_line], cursor.col - 1);
            std::size_t line_len =
                line_size(buffer->lines[buffer->current_line]);
            if (col != line_len) {
                cursor.set_pos_abs(cursor.row, col + 2, buffer->lineno_offset);
            } else {
                cursor.set_pos_abs(cursor.row, line_len + 1,
                                   buffer->lineno_offset);
            }
            buffer->reset_status_bar(view_size, &cursor);

        } else if (k.code == 'b' && modifier == rawterm::Mod::None) {
            std::size_t col = find_prev_whitespace(
                buffer->lines[buffer->current_line], cursor.col - 1);
            cursor.set_pos_abs(cursor.row, col, buffer->lineno_offset);
            buffer->reset_status_bar(view_size, &cursor);

        } else if (k.code == 'z' && modifier == rawterm::Mod::None) {
            center(buffer->current_line + 1);

            // Char manipulation / deletion
        } else if (k.code == 'x' && modifier == rawterm::Mod::None) {
            buffer->lines[buffer->current_line].erase(cursor.col - 1, 1);

            redraw_line();
            cursor.set_pos_abs(cursor.row, cursor.col, buffer->lineno_offset);
            buffer->reset_status_bar(view_size, &cursor);
            buffer->modified = true;

        } else if (k.code == 'd' && modifier == rawterm::Mod::None) {
            rawterm::Key k2 = rawterm::process_keypress();
            rawterm::Mod mod2 = rawterm::getMod(&k);
            // dl delete line
            // dw delete word
            // de delete end
            // da delete to start

            if (k2.code == 'l' && mod2 == rawterm::Mod::None) {
                buffer->lines.erase(buffer->lines.begin() +
                                    buffer->current_line);
                if (buffer->lines.size() == 0) {
                    buffer->lines.push_back("");
                }

                rawterm::clear_screen();
                rawterm::move_cursor({ 1, 1 });
                draw(buffer->current_line - cursor.row + 1);
                buffer->modified = true;
                buffer->reset_status_bar(view_size, &cursor);
                cursor.set_pos_abs(cursor.row, 0, buffer->lineno_offset);

            } else if (k2.code == 'w' && mod2 == rawterm::Mod::None) {
                std::string &line = buffer->lines[buffer->current_line];
                if (cursor.col > line.size())
                    return;

                int pos1 = line.substr(0, cursor.col).rfind(' ');
                int pos2 = line.substr(cursor.col, line.size()).find(' ');
                if (pos2 == -1)
                    pos2 = line.size();

                line.erase(pos1 + 1, (cursor.col + pos2) - pos1);

                redraw_line();
                buffer->modified = true;
                cursor.set_pos_abs(cursor.row, pos1 + 1, buffer->lineno_offset);
                buffer->reset_status_bar(view_size, &cursor);

            } else if (k2.code == 'e' && mod2 == rawterm::Mod::None) {
                std::size_t pos =
                    cursor.col - 1 +
                    (buffer->lines[buffer->current_line][0] == '\t' ? TABSTOP
                                                                    : 0);
                buffer->lines[buffer->current_line].erase(
                    pos, buffer->lines[buffer->current_line].size());

                redraw_line();
                buffer->modified = true;
                buffer->reset_status_bar(view_size, &cursor);

            } else if (k2.code == 'a' && mod2 == rawterm::Mod::None) {
                std::size_t pos =
                    cursor.col - 1 +
                    (buffer->lines[buffer->current_line][0] == '\t' ? TABSTOP
                                                                    : 0);
                buffer->lines[buffer->current_line].erase(0, pos);

                redraw_line();
                buffer->modified = true;
                cursor.set_pos_rel(0, -pos, buffer->lineno_offset);
                buffer->reset_status_bar(view_size, &cursor);
            }
        } else if (k.code == 'J' && modifier == rawterm::Mod::Shift) {
            join_lines(buffer->lines, buffer->current_line);

            draw(buffer->current_line - cursor.row + 1);
            buffer->modified = true;
            buffer->reset_status_bar(view_size, &cursor);
        }
    }
}

void Viewport::keypress_write() {
    using rawterm::Mod;

    while (true) {
        buffer->reset_status_bar(view_size, &cursor);
        if (SignalTracker<SIGWINCH>::checkAndClear()) {
            resize_flag = true;
            break;
        }

        rawterm::sigcont_handler([this]() {
            rawterm::move_cursor({ 1, 1 });
            draw(buffer->current_line - cursor.row + 1);
            cursor.set_pos_abs(cursor.row, cursor.col, buffer->lineno_offset);
        });

        rawterm::Key k = rawterm::process_keypress();
        Mod modifier = rawterm::getMod(&k);

        if (modifier == Mod::Escape) {
            break;

        } else if (modifier == Mod::Space) {
            buffer->lines[buffer->current_line].insert(
                cursor.col + horizontal_offset - 1, 1, ' ');
            redraw_line();
            cursor.set_pos_rel(0, 1, buffer->lineno_offset);
            buffer->modified = true;

        } else if (modifier == Mod::Backspace) {
            if (cursor.col > 1) {
                buffer->lines[buffer->current_line].erase(
                    cursor.col + horizontal_offset - 2, 1);

                redraw_line();
                cursor.set_pos_abs(cursor.row, cursor.col - 1,
                                   buffer->lineno_offset);
                buffer->modified = true;
            } else {
                buffer->current_line--;
                int new_curr_pos = buffer->lines[buffer->current_line].size();
                join_lines(buffer->lines, buffer->current_line);

                buffer->modified = true;

                if (buffer->current_line + 1 < buffer->lines.size()) {
                    cursor.set_pos_rel(-1, new_curr_pos, buffer->lineno_offset);
                } else {
                    cursor.set_pos_rel(0, new_curr_pos, buffer->lineno_offset);
                }

                unsigned int draw_point = buffer->current_line - cursor.row + 1;
                (draw_point < buffer->lines.size()) ? draw(draw_point)
                                                    : draw(0);
            }

        } else if (modifier == Mod::Delete) {
            if (cursor.col < line_size(buffer->lines[buffer->current_line])) {
                buffer->lines[buffer->current_line].erase(
                    cursor.col + horizontal_offset - 1, 1);

                redraw_line();
                cursor.set_pos_abs(cursor.row, cursor.col,
                                   buffer->lineno_offset);
                buffer->modified = true;
            } else {
                int new_curr_pos = buffer->lines[buffer->current_line].size();
                join_lines(buffer->lines, buffer->current_line);

                buffer->modified = true;

                if (buffer->current_line < buffer->lines.size()) {
                    cursor.set_pos_rel(-1, new_curr_pos, buffer->lineno_offset);
                } else {
                    cursor.set_pos_rel(0, new_curr_pos, buffer->lineno_offset);
                }

                unsigned int draw_point = buffer->current_line - cursor.row + 1;
                (draw_point < buffer->lines.size()) ? draw(draw_point)
                                                    : draw(0);
            }

        } else if (modifier == Mod::Enter) {
            // TODO: Indent new line to same level as old line

            buffer->split_lines(cursor);
            draw(buffer->current_line - cursor.row + 1);
            buffer->current_line++;

            cursor.set_pos_abs(cursor.row + horizontal_offset + 1, 1,
                               buffer->lineno_offset);
            buffer->modified = true;

        } else if (modifier == Mod::Arrow) {
            switch (k.code) {
            case 'A': // Up
                cursor_up(1);
                break;
            case 'B': // Down
                cursor_down(1);
                break;

            case 'C': // Right
                // TODO: Jump backwards if it's not in the right place when
                // going u/d
                if (cursor.col <
                    line_size(buffer->lines[buffer->current_line])) {
                    cursor.set_pos_rel(0, 1, buffer->lineno_offset);
                }
                break;
            case 'D': // Left
                if (cursor.col > 1) {
                    cursor.set_pos_rel(0, -1, buffer->lineno_offset);
                }
                break;
            }

        } else if (modifier == Mod::Control && k.code == 'i') { // Tab
            buffer->lines[buffer->current_line].insert(cursor.col - 1, "\t");
            buffer->lines[buffer->current_line] =
                filter_whitespace(std::vector<std::string>(
                    { buffer->lines[buffer->current_line] }))[0];
            redraw_line();
            cursor.set_pos_rel(0, TABSTOP, buffer->lineno_offset);
            buffer->modified = true;

        } else {
            std::string *line = &buffer->lines[buffer->current_line];

            if (line->empty()) {
                line->push_back(k.code);
            } else {
                std::size_t pos =
                    cursor.col - 1 + (line->front() == '\t' ? TABSTOP : 0);
                line->insert(pos, std::string(1, k.code));
            }

            redraw_line();
            cursor.set_pos_rel(0, 1, buffer->lineno_offset);
            buffer->modified = true;
        }
    }
}

void Viewport::keypress_command() {
    using rawterm::Mod;
    std::string cmd = ";";
    std::vector<Mod> searchable = { Mod::None, Mod::Shift, Mod::Space };

    while (true) {
        if (SignalTracker<SIGWINCH>::checkAndClear()) {
            resize_flag = true;
            break;
        }

        rawterm::sigcont_handler([this]() {
            rawterm::move_cursor({ 1, 1 });
            draw(buffer->current_line - cursor.row + 1);
            cursor.set_pos_abs(cursor.row, cursor.col, buffer->lineno_offset);
        });

        rawterm::move_cursor({ view_size.horizontal + 2, 1 });
        rawterm::clear_line();

        rawterm::move_cursor({ view_size.horizontal + 2, 1 });
        std::cout << cmd;
        rawterm::move_cursor({ view_size.horizontal + 2, cmd.size() + 1 });

        // Read and handle input
        rawterm::Key k = rawterm::process_keypress();
        rawterm::Mod modifier = rawterm::getMod(&k);

        if (modifier == Mod::Escape) {
            rawterm::move_cursor({ view_size.horizontal + 2, 1 });
            rawterm::clear_line();
            break;
        } else if (modifier == Mod::Enter) {
            rawterm::clear_line();
            buffer->parse_command(cmd);
            break;
        } else if (modifier == Mod::Backspace) {
            cmd = cmd.substr(0, cmd.size() - 1);
        } else if (std::find(searchable.begin(), searchable.end(), modifier) !=
                   searchable.end()) {
            cmd.push_back(k.code);
        }
    }
}
