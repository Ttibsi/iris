#include <rawterm/rawterm.h>

#include "text_manip.h"
#include "viewport.h"

void Viewport::handle_keypress() {
    while (true) {

        rawterm::Key k = rawterm::process_keypress();
        if (k.code == 'q' && rawterm::getMod(&k) == rawterm::Mod::None) {
            break;

            // Left
        } else if (k.code == 'h' && rawterm::getMod(&k) == rawterm::Mod::None) {

            if (cursor.col > tab_count(buffer->lines[buffer->current_line])) {
                cursor.set_pos_rel(0, -1);
                buffer->reset_status_bar(view_size, &cursor);
            }

            // Down
        } else if (k.code == 'j' && rawterm::getMod(&k) == rawterm::Mod::None) {
            if (cursor.row == view_size.vertical &&
                buffer->current_line < buffer->lines.size()) {
                // Scroll view
                rawterm::clear_screen();
                std::size_t cursor_col = cursor.col;
                cursor.set_pos_abs(1, 1); // TODO: Save row position too
                buffer->current_line++;
                draw(buffer->current_line + 1 - view_size.vertical);
                cursor.set_pos_abs(view_size.vertical, cursor_col);
                buffer->reset_status_bar(view_size, &cursor);
            } else if (cursor.row < buffer->lines.size()) {
                // Move cursor
                buffer->current_line++;
                if (cursor.col > buffer->lines[buffer->current_line].size()) {
                    cursor.set_pos_abs(
                        cursor.row + 1,
                        std::max(buffer->lines[buffer->current_line].size(),
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
                if (cursor.col > buffer->lines[buffer->current_line].size()) {
                    cursor.set_pos_abs(
                        cursor.row - 1,
                        std::max(buffer->lines[buffer->current_line].size(),
                                 static_cast<std::size_t>(1)));
                } else {
                    cursor.set_pos_rel(-1, 0);
                }
                buffer->reset_status_bar(view_size, &cursor);
            }

            // Right
        } else if (k.code == 'l' && rawterm::getMod(&k) == rawterm::Mod::None) {
            // TODO: Jump backwards if it's not in the right place when going
            // u/d
            if (cursor.col < buffer->line_size(buffer->current_line)) {
                cursor.set_pos_rel(0, 1);
                buffer->reset_status_bar(view_size, &cursor);
            }
        }
    }
}
