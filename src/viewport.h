#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <format>

#include <rawterm/rawterm.h>

#include "buffer.h"
#include "constants.h"
#include "cursor.h"
#include "editor.h"
#include "text_manip.h"

struct Viewport {
    Buffer *buffer;
    rawterm::Pos view_size;
    Cursor cursor;

    Viewport(Buffer *, rawterm::Pos);
    void draw(const std::size_t &);
    void redraw_line();
    void keypress_read();
    void keypress_write();
    void keypress_command();
    void switch_to_insert();
};

inline Viewport::Viewport(Buffer *b, rawterm::Pos size)
    : buffer(b), view_size(size), cursor(Cursor(0)) {}

inline void Viewport::draw(const std::size_t &start_point) {
    // start_point = the 0th line to print
    std::vector<std::string> lines = filter_whitespace(buffer->lines);
    auto start = lines.begin() + std::min(start_point, lines.size() - 1);
    std::size_t end =
        std::max(std::min(view_size.vertical, buffer->lines.size()),
                 static_cast<unsigned long>(1));

    int idx;
    if (LINE_NUMBER) {
        idx = start_point + 1;
    }

    for (auto it = start; it < start + end; ++it) {
        if (LINE_NUMBER) {
            std::cout << std::format("{:>{}}", idx, buffer->lineno_offset - 1)
                      << "\u2502";
            idx++;
        }
        std::cout << *it;
    }

    if (view_size.vertical > end) {
        for (unsigned long i = end; i < view_size.vertical; i++) {
            std::cout << "~\r\n";
        }
    }

    std::cout << buffer->render_status_bar(
        view_size.horizontal + buffer->lineno_offset, &cursor);
    std::cout << "\r\n";
}

inline void Viewport::redraw_line() {
    rawterm::clear_line();
    rawterm::move_cursor({ cursor.row, 1 });
    if (LINE_NUMBER) {
        std::cout << std::format("{:>{}}", buffer->current_line + 1,
                                 buffer->lineno_offset - 1)
                  << "\u2502";
    }

    std::cout << buffer->lines[buffer->current_line];
}

inline void Viewport::switch_to_insert() {
    if (buffer->readonly)
        return; // can't edit a readonly file

    buffer->editor->set_mode(Mode::Write);
    buffer->reset_status_bar(view_size, &cursor);
    rawterm::cursor_pipe_blink();

    keypress_write();

    buffer->editor->set_mode(Mode::Read);
    rawterm::cursor_block();
    buffer->reset_status_bar(view_size, &cursor);
}

#endif // VIEWPORT_H
