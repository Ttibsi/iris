#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <format>
#include <span>

#include <rawterm/rawterm.h>

#include "buffer.h"
#include "constants.h"
#include "cursor.h"
#include "editor.h"
#include "highlighting/highlighter.h"
#include "text_manip.h"

struct Viewport {
    Buffer *buffer;
    rawterm::Pos view_size;
    Cursor cursor;

    Viewport(Buffer *, rawterm::Pos);
    void draw(const std::size_t &);
    void redraw_line();
    void switch_to_insert();
    void center(unsigned int);
    void cursor_up(unsigned int);
    void cursor_down(unsigned int);

    // handle_keypress.cpp
    void keypress_read();
    void keypress_write();
    void keypress_command();
};

inline Viewport::Viewport(Buffer *b, rawterm::Pos size)
    : buffer(b), view_size(size), cursor(Cursor(0)) {}

inline void Viewport::draw(const std::size_t &start_point) {
    // start_point = the 0th line to print
    rawterm::clear_screen();
    rawterm::move_cursor({ 1, 1 });

    std::vector<std::string> lines = filter_whitespace(buffer->lines);
    auto start = std::min(start_point, lines.size() - 1);
    std::size_t end =
        std::max(std::min(view_size.vertical, lines.size()),
                 static_cast<unsigned long>(1));

    std::span<std::string> line_span = lines;
    highlight(buffer->lang, line_span.subspan(start, view_size.vertical));

    int idx;
    if (LINE_NUMBER) {
        idx = start_point + 1;
    }

    for (auto it = start; it < start + end; ++it) {
        if (it < lines.size()) {
            if (LINE_NUMBER) {
                std::cout << std::format("{:>{}}", idx,
                                         buffer->lineno_offset - 1)
                          << "\u2502";
                idx++;
            }
            std::cout << lines[it] << "\r\n";
        } else {
            std::cout << "\r\n";
        }
    }

    if (view_size.vertical > end) {
        for (unsigned long i = end; i < view_size.vertical; i++) {
            if (i == 1) {
                std::cout << "\r\n";
            } else {
                std::cout << "~\r\n";
            }
        }
    }

    std::cout << buffer->render_status_bar(
        view_size.horizontal + buffer->lineno_offset, &cursor);
    std::cout << "\r\n";
}

inline void Viewport::redraw_line() {
    rawterm::clear_line();
    rawterm::move_cursor({ cursor.row, 1 });
    std::vector<std::string> lines =
        filter_whitespace({ buffer->lines[buffer->current_line] });

    highlight_line(buffer->lang, lines[0]);

    if (LINE_NUMBER) {
        std::cout << std::format("{:>{}}", buffer->current_line + 1,
                                 buffer->lineno_offset - 1)
                  << "\u2502";
    }

    std::cout << lines[0] << std::flush;
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

inline void Viewport::center(unsigned int line_num) {
    rawterm::clear_screen();

    if (line_num < view_size.vertical / 2) {
        draw(0);
        cursor.set_pos_abs(line_num, 1, buffer->lineno_offset);

    } else if (line_num < buffer->lines.size()) {
        draw(line_num - (view_size.vertical / 2));
        cursor.set_pos_abs(view_size.vertical / 2, 1, buffer->lineno_offset);
    }

    buffer->current_line = line_num - 1;
    buffer->reset_status_bar(view_size, &cursor);
}

inline void Viewport::cursor_down(unsigned int count) {
    for (unsigned int i = 0; i < count; i++) {
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
            if (cursor.col > line_size(buffer->lines[buffer->current_line]) +
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
    }
}

inline void Viewport::cursor_up(unsigned int count) {
    for (unsigned int i = 0; i < count; i++) {
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
            if (cursor.col > line_size(buffer->lines[buffer->current_line])) {
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
    }
}
#endif // VIEWPORT_H
