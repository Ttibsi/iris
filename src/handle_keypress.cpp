#include <rawterm/rawterm.h>

#include "viewport.h"

void Viewport::handle_keypress() {
    while (true) {
        Key k = process_keypress();
        if (k.code == 'q' && k.mod.empty()) {
            break;
        } else if (k.code == 'h' && k.mod.empty()) {
            if (cursor.col > 0) {
                cursor.set_pos_rel(0, -1);
            }
        } else if (k.code == 'j' && k.mod.empty()) {
            if (cursor.row == view_size.first &&
                buffer->current_line < buffer->lines.size()) {
                clear_screen();
                std::size_t col = cursor.col;
                cursor.set_pos_rel(0, -col);
                buffer->current_line++;
                draw((buffer->current_line - view_size.first));
                // Need to move the cursor back to it's position after
                // re-printing buffer
                cursor.set_pos_rel(0, col);
                buffer->reset_status_bar(view_size);
            } else if (cursor.row < buffer->lines.size()) {
                if (cursor.row == 0) {
                    cursor.set_pos_rel(2, 0);
                } else {
                    cursor.set_pos_rel(1, 0);
                }
                buffer->current_line++;
                buffer->reset_status_bar(view_size);
            }

        } else if (k.code == 'k' && k.mod.empty()) {
            if (cursor.row == 0 && buffer->current_line > 1) {
                std::size_t col = cursor.col;
                buffer->current_line--;
                clear_screen();
                cursor.set_pos_abs(0, 0);
                draw(buffer->current_line);
                cursor.set_pos_abs(0, col);
                buffer->reset_status_bar(view_size);
            } else if (cursor.row > 0) {
                cursor.set_pos_rel(-1, 0);
                buffer->current_line--;
                buffer->reset_status_bar(view_size);
            }

        } else if (k.code == 'l' && k.mod.empty()) {
            // TODO: Jump backwards if it's not in the right place when going
            // u/d
            if (cursor.col < buffer->line_size(buffer->current_line - 1)) {
                cursor.set_pos_rel(0, 1);
            }
        }
    }
}
