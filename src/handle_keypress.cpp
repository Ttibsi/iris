#include <algorithm>
#include <string>
#include <vector>

#include <rawterm/rawterm.h>

#include "constants.h"
#include "text_manip.h"
#include "viewport.h"

void Viewport::keypress_read() {
    while (true) {
        if (buffer->quit_buf) {
            break;
        }

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
            cursor.set_pos_abs(cursor.row, col_pos, buffer->lineno_offset);
            switch_to_insert();

            // Left
        } else if (k.code == 'h' && modifier == rawterm::Mod::None) {
            std::size_t tab_count =
                line_size(buffer->lines[buffer->current_line]) -
                buffer->lines[buffer->current_line].size();

            if (cursor.col > tab_count) {
                cursor.set_pos_rel(0, -1, buffer->lineno_offset);
                buffer->reset_status_bar(view_size, &cursor);
            }

            // Down
        } else if (k.code == 'j' && modifier == rawterm::Mod::None) {
            if (cursor.row == view_size.vertical &&
                buffer->current_line < buffer->lines.size() - 1) {
                // Scroll view
                rawterm::clear_screen();
                std::size_t cursor_col = cursor.col;
                cursor.set_pos_abs(1, 1, 0);
                buffer->current_line++;
                draw(buffer->current_line + 1 - view_size.vertical);
                cursor.set_pos_abs(view_size.vertical, cursor_col,
                                   buffer->lineno_offset);
                buffer->reset_status_bar(view_size, &cursor);
            } else if (cursor.row < view_size.vertical) {
                // Move cursor in view
                if (cursor.col >
                    line_size(buffer->lines[buffer->current_line]) +
                        buffer->lineno_offset) {
                    cursor.set_pos_abs(
                        cursor.row + 1,
                        std::max(line_size(buffer->lines[buffer->current_line]),
                                 static_cast<std::size_t>(1)),
                        buffer->lineno_offset);

                    buffer->current_line++;
                    buffer->reset_status_bar(view_size, &cursor);

                } else if (cursor.row < buffer->lines.size()) {
                    cursor.set_pos_rel(1, 0, buffer->lineno_offset);
                    buffer->current_line++;
                    buffer->reset_status_bar(view_size, &cursor);
                }
            }

            // Up
        } else if (k.code == 'k' && modifier == rawterm::Mod::None) {
            if (cursor.row == 1 && buffer->current_line > 0) {
                // Scroll up
                std::size_t col = cursor.col; // TODO: Save row position too
                buffer->current_line--;
                rawterm::clear_screen();
                cursor.set_pos_abs(1, 1, 0);
                draw(buffer->current_line);
                cursor.set_pos_abs(1, col, buffer->lineno_offset);
                buffer->reset_status_bar(view_size, &cursor);
            } else if (cursor.row > 1) {
                // Move cursor up
                buffer->current_line--;
                if (cursor.col >
                    line_size(buffer->lines[buffer->current_line])) {
                    cursor.set_pos_abs(
                        cursor.row - 1,
                        std::max(line_size(buffer->lines[buffer->current_line]),
                                 static_cast<std::size_t>(1)),
                        buffer->lineno_offset);
                } else {
                    cursor.set_pos_rel(-1, 0, buffer->lineno_offset);
                }
                buffer->reset_status_bar(view_size, &cursor);
            }

            // Right
        } else if (k.code == 'l' && modifier == rawterm::Mod::None) {
            if (cursor.col < line_size(buffer->lines[buffer->current_line]) +
                                 1 + buffer->lineno_offset) {
                cursor.set_pos_rel(0, 1, buffer->lineno_offset);
                buffer->reset_status_bar(view_size, &cursor);
            }

            // Command mode
        } else if (k.code == ';' && modifier == rawterm::Mod::None) {
            buffer->editor->set_mode(Mode::Command);
            buffer->reset_status_bar(view_size, &cursor);
            keypress_command();
            cursor.set_pos_abs(cursor.row, cursor.col, buffer->lineno_offset);
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

        } else if (k.code == 'b' && modifier == rawterm::Mod::None) {
            std::size_t col = find_prev_whitespace(
                buffer->lines[buffer->current_line], cursor.col - 1);
            cursor.set_pos_abs(cursor.row, col, buffer->lineno_offset);

            // Char manipulation
        } else if (k.code == 'x' && modifier == rawterm::Mod::None) {
            buffer->lines[buffer->current_line].erase(cursor.col - 1, 1);

            redraw_line();
            cursor.set_pos_abs(cursor.row, cursor.col, buffer->lineno_offset);
            buffer->modified = true;
        }
    }
}

void Viewport::keypress_write() {
    using rawterm::Mod;

    while (true) {
        buffer->reset_status_bar(view_size, &cursor);
        rawterm::Key k = rawterm::process_keypress();
        Mod modifier = rawterm::getMod(&k);

        if (modifier == Mod::Escape) {
            break;

        } else if (modifier == Mod::Space) {
            buffer->lines[buffer->current_line].insert(cursor.col - 1, 1, ' ');
            redraw_line();
            cursor.set_pos_rel(0, 1, buffer->lineno_offset);
            buffer->modified = true;

        } else if (modifier == Mod::Backspace) {
            // TODO: backspace newline char (merge two lines)
            if (cursor.col > 1) {
                buffer->lines[buffer->current_line].erase(cursor.col - 2, 1);

                redraw_line();
                cursor.set_pos_abs(cursor.row, cursor.col - 1,
                                   buffer->lineno_offset);
                buffer->modified = true;
            }

        } else if (modifier == Mod::Delete) {
            // TODO: backspace newline char (merge two lines)
            if (cursor.col < line_size(buffer->lines[buffer->current_line])) {
                buffer->lines[buffer->current_line].erase(cursor.col - 1, 1);

                redraw_line();
                cursor.set_pos_abs(cursor.row, cursor.col,
                                   buffer->lineno_offset);
                buffer->modified = true;
            }

        } else if (modifier == Mod::Enter) {
            // TODO: Indent new line to same level as old line

            buffer->split_lines(cursor);
            rawterm::clear_screen();
            rawterm::move_cursor({ 1, 1 });
            draw(buffer->current_line - cursor.row + 1);
            buffer->current_line++;

            cursor.set_pos_abs(cursor.row + 1, 1, buffer->lineno_offset);
            buffer->modified = true;

        } else if (modifier == Mod::Arrow) {
            switch (k.code) {
            case 'A': // Up
                if (cursor.row == 1 && buffer->current_line > 1) {
                    std::size_t col = cursor.col; // TODO: Save row position too
                    buffer->current_line--;
                    rawterm::clear_screen();
                    cursor.set_pos_abs(1, 1, buffer->lineno_offset);
                    draw(buffer->current_line);
                    cursor.set_pos_abs(1, col, buffer->lineno_offset);
                } else if (cursor.row > 1) {
                    // Move cursor up
                    buffer->current_line--;
                    if (cursor.col >
                        line_size(buffer->lines[buffer->current_line])) {
                        cursor.set_pos_abs(
                            cursor.row - 1,
                            std::max(
                                line_size(buffer->lines[buffer->current_line]),
                                static_cast<std::size_t>(1)),
                            buffer->lineno_offset);
                    } else {
                        cursor.set_pos_rel(-1, 0, buffer->lineno_offset);
                    }
                }
                break;
            case 'B': // Down
                if (cursor.row == view_size.vertical &&
                    buffer->current_line < buffer->lines.size() - 1) {
                    // Scroll view
                    rawterm::clear_screen();
                    std::size_t cursor_col = cursor.col;
                    cursor.set_pos_abs(1, 1, buffer->lineno_offset);
                    buffer->current_line++;
                    draw(buffer->current_line + 1 - view_size.vertical);
                    cursor.set_pos_abs(view_size.vertical, cursor_col,
                                       buffer->lineno_offset);
                } else if (cursor.row < view_size.vertical) {
                    // Move cursor in view
                    if (cursor.col >
                        line_size(buffer->lines[buffer->current_line])) {
                        cursor.set_pos_abs(
                            cursor.row + 1,
                            std::max(
                                line_size(buffer->lines[buffer->current_line]),
                                static_cast<std::size_t>(1)),
                            buffer->lineno_offset);
                        buffer->current_line++;
                    } else if (cursor.row < buffer->lines.size()) {
                        cursor.set_pos_rel(1, 0, buffer->lineno_offset);
                        buffer->current_line++;
                    }
                }
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
        rawterm::move_cursor({ view_size.horizontal + 2, 1 });
        rawterm::clear_line();

        rawterm::move_cursor({ view_size.horizontal + 2, 1 });
        std::cout << cmd;
        rawterm::move_cursor({ view_size.horizontal + 2, cmd.size() + 1 });

        // Read and handle input
        rawterm::Key k = rawterm::process_keypress();
        rawterm::Mod modifier = rawterm::getMod(&k);

        if (modifier == Mod::Escape) {
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
