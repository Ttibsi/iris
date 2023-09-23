#include "buffer.h"

void Buffer::handle_keypress() {
    while (true) {
        Key k = process_keypress();
        if (k.code == 'q' && k.mod.empty()) {
            break;
        } else if (k.code == 'h' && k.mod.empty()) {
            if (cursor.col > 0) {
                cursor.set_pos_rel(0, -1);
            }
        } else if (k.code == 'j' && k.mod.empty()) {
            if (cursor.row < length) {
                cursor.set_pos_rel(1, 0);
            }
        } else if (k.code == 'k' && k.mod.empty()) {
            if (cursor.row > 0) {
                cursor.set_pos_rel(-1, 0);
            }
        } else if (k.code == 'l' && k.mod.empty()) {
            if (cursor.col < lines[cursor.row].end) {
                cursor.set_pos_rel(0, 1);
            }
        }
    }
}
