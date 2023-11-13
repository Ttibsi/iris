#include <algorithm>
#include <string>
#include <vector>

#include <chrono>
#include <thread>

#include <rawterm/rawterm.h>

#include "constants.h"
#include "editor.h"
#include "text_manip.h"
#include "viewport.h"

void Viewport::keypress_read() {
    while (true) {
        if (buffer->quit_buf) {
            break;
        }

        rawterm::Key k = rawterm::process_keypress();
        // Insert mode
        if (k.code == 'i' && rawterm::getMod(&k) == rawterm::Mod::None) {
            buffer->editor->set_mode(Mode::Write);
            buffer->reset_status_bar(view_size, &cursor);
            rawterm::cursor_pipe_blink();
            keypress_write();
            buffer->editor->set_mode(Mode::Read);
            rawterm::cursor_block();
            buffer->reset_status_bar(view_size, &cursor);

            // Left
        } else if (k.code == 'h' && rawterm::getMod(&k) == rawterm::Mod::None) {
            std::size_t tab_count =
                line_size(buffer->lines[buffer->current_line]) -
                buffer->lines[buffer->current_line].size();

            if (cursor.col > tab_count) {
                cursor.set_pos_rel(0, -1);
                buffer->reset_status_bar(view_size, &cursor);
            }

            // Down
        } else if (k.code == 'j' && rawterm::getMod(&k) == rawterm::Mod::None) {
            if (cursor.row == view_size.vertical &&
                buffer->current_line < buffer->lines.size() - 1) {
                // Scroll view
                rawterm::clear_screen();
                std::size_t cursor_col = cursor.col;
                cursor.set_pos_abs(1, 1);
                buffer->current_line++;
                draw(buffer->current_line + 1 - view_size.vertical);
                cursor.set_pos_abs(view_size.vertical, cursor_col);
                buffer->reset_status_bar(view_size, &cursor);
            } else if (cursor.row < view_size.vertical) {
                // Move cursor in view
                buffer->current_line++;
                if (cursor.col >
                    line_size(buffer->lines[buffer->current_line])) {
                    cursor.set_pos_abs(
                        cursor.row + 1,
                        std::max(line_size(buffer->lines[buffer->current_line]),
                                 static_cast<std::size_t>(1)));
                } else {
                    cursor.set_pos_rel(1, 0);
                }

                buffer->reset_status_bar(view_size, &cursor);
            }

            // Up
        } else if (k.code == 'k' && rawterm::getMod(&k) == rawterm::Mod::None) {
            if (cursor.row == 1 && buffer->current_line > 0) {
                // Scroll up
                std::size_t col = cursor.col; // TODO: Save row position too
                buffer->current_line--;
                rawterm::clear_screen();
                cursor.set_pos_abs(1, 1);
                draw(buffer->current_line);
                cursor.set_pos_abs(1, col);
                buffer->reset_status_bar(view_size, &cursor);
            } else if (cursor.row > 1) {
                // Move cursor up
                buffer->current_line--;
                if (cursor.col >
                    line_size(buffer->lines[buffer->current_line])) {
                    cursor.set_pos_abs(
                        cursor.row - 1,
                        std::max(line_size(buffer->lines[buffer->current_line]),
                                 static_cast<std::size_t>(1)));
                } else {
                    cursor.set_pos_rel(-1, 0);
                }
                buffer->reset_status_bar(view_size, &cursor);
            }

            // Right
        } else if (k.code == 'l' && rawterm::getMod(&k) == rawterm::Mod::None) {
            if (cursor.col < line_size(buffer->lines[buffer->current_line])) {
                cursor.set_pos_rel(0, 1);
                buffer->reset_status_bar(view_size, &cursor);
            }

            // Command mode
        } else if (k.code == ';' && rawterm::getMod(&k) == rawterm::Mod::None) {
            buffer->editor->set_mode(Mode::Command);
            buffer->reset_status_bar(view_size, &cursor);
            keypress_command();
            cursor.set_pos_abs(cursor.row, cursor.col);
            buffer->editor->set_mode(Mode::Read);
            buffer->reset_status_bar(view_size, &cursor);
        }
    }
}

void Viewport::keypress_write() {
    using rawterm::Mod;

    while (true) {
        rawterm::Key k = rawterm::process_keypress();
        Mod modifier = rawterm::getMod(&k);

        if (modifier == Mod::Escape) {
            break;

        } else if (modifier == Mod::Space) {
            buffer->lines[buffer->current_line].insert(cursor.col - 1, 1, ' ');
            rawterm::clear_line();
            rawterm::move_cursor({ cursor.row, 1 });
            std::cout << buffer->lines[buffer->current_line];
            cursor.set_pos_rel(0, 1);
            buffer->modified = true;

        } else if (modifier == Mod::Backspace) {
            // TODO: backspace newline char (merge two lines)
            if (cursor.col > 1) {
                buffer->lines[buffer->current_line].erase(cursor.col - 2, 1);

                std::size_t cursor_col = cursor.col;
                rawterm::clear_line();
                cursor.set_pos_abs(cursor.row, 1);
                std::cout << buffer->lines[buffer->current_line];
                cursor.set_pos_abs(cursor.row, cursor_col - 1);
                buffer->modified = true;
            }

        } else if (modifier == Mod::Delete) {
            // TODO: backspace newline char (merge two lines)
            if (cursor.col < line_size(buffer->lines[buffer->current_line])) {
                buffer->lines[buffer->current_line].erase(cursor.col - 1, 1);

                std::size_t cursor_col = cursor.col;
                rawterm::clear_line();
                cursor.set_pos_abs(cursor.row, 1);
                std::cout << buffer->lines[buffer->current_line];
                cursor.set_pos_abs(cursor.row, cursor_col);
                buffer->modified = true;
            }

            // segfault
        } else if (modifier == Mod::Enter) {
            // TODO: Indent new line to same level as old line
            buffer->split_lines(cursor);
            redraw();
            buffer->current_line++;
            buffer->reset_status_bar(view_size, &cursor);
            cursor.set_pos_rel(1, -cursor.col);
            buffer->modified = true;

        } else if (modifier == Mod::Arrow) {
            switch (k.code) {
            case 'A': // Up
                if (cursor.row == 1 && buffer->current_line > 1) {
                    std::size_t col = cursor.col; // TODO: Save row position too
                    buffer->current_line--;
                    rawterm::clear_screen();
                    cursor.set_pos_abs(1, 1);
                    draw(buffer->current_line);
                    cursor.set_pos_abs(1, col);
                    buffer->reset_status_bar(view_size, &cursor);
                } else if (cursor.row > 1) {
                    // Move cursor up
                    buffer->current_line--;
                    if (cursor.col >
                        line_size(buffer->lines[buffer->current_line])) {
                        cursor.set_pos_abs(
                            cursor.row - 1,
                            std::max(
                                line_size(buffer->lines[buffer->current_line]),
                                static_cast<std::size_t>(1)));
                    } else {
                        cursor.set_pos_rel(-1, 0);
                    }
                }
                break;
            case 'B': // Down
                if (cursor.row == view_size.vertical &&
                    buffer->current_line < buffer->lines.size() - 1) {
                    // Scroll view
                    rawterm::clear_screen();
                    std::size_t cursor_col = cursor.col;
                    cursor.set_pos_abs(1, 1);
                    buffer->current_line++;
                    draw(buffer->current_line + 1 - view_size.vertical);
                    cursor.set_pos_abs(view_size.vertical, cursor_col);
                    buffer->reset_status_bar(view_size, &cursor);
                } else if (cursor.row < view_size.vertical) {
                    // Move cursor in view
                    buffer->current_line++;
                    if (cursor.col >
                        line_size(buffer->lines[buffer->current_line])) {
                        cursor.set_pos_abs(
                            cursor.row + 1,
                            std::max(
                                line_size(buffer->lines[buffer->current_line]),
                                static_cast<std::size_t>(1)));
                    } else {
                        cursor.set_pos_rel(1, 0);
                    }
                }
                buffer->reset_status_bar(view_size, &cursor);
                break;

            case 'C': // Right
                // TODO: Jump backwards if it's not in the right place when
                // going u/d
                if (cursor.col <
                    line_size(buffer->lines[buffer->current_line])) {
                    cursor.set_pos_rel(0, 1);
                }
                break;
            case 'D': // Left
                if (cursor.col > 1) {
                    cursor.set_pos_rel(0, -1);
                }
                break;
            }

            // segfault
        } else if (modifier == Mod::Control && k.code == 'i') { // Tab
            buffer->lines[buffer->current_line].insert(cursor.col, "\t");
            redraw();
            cursor.set_pos_rel(0, TABSTOP);
            buffer->modified = true;

        } else {
            std::size_t col = cursor.col;
            std::string *line = &buffer->lines[buffer->current_line];

            if (line->empty()) {
                line->push_back(k.code);
            } else {
                std::size_t pos =
                    cursor.col + (line->front() == '\t' ? TABSTOP : 1);
                line->insert(pos, std::string(1, k.code));
            }

            rawterm::clear_line();
            cursor.set_pos_abs(cursor.row, 1);
            std::cout << buffer->lines[buffer->current_line];
            cursor.set_pos_abs(cursor.row, col + 1);
        }
    }
}

void Viewport::keypress_command() {
    using rawterm::Mod;
    std::string cmd = ";";
    std::vector<Mod> searchable = { Mod::None, Mod::Shift, Mod::Space };

    while (true) {
        // Print out current command string
        rawterm::move_cursor({ view_size.horizontal + 2, 1 });
        // std::cout << std::string(view_size.vertical, ' ');
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
