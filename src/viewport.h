#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <utility>

#include <rawterm/rawterm.h>

#include "buffer.h"
#include "cursor.h"

struct Viewport {
    Buffer *buffer;
    rawterm::Pos view_size;
    Cursor cursor;

    Viewport(Buffer *, rawterm::Pos);
    void draw(const std::size_t &);
    void handle_keypress();
};

inline Viewport::Viewport(Buffer *b, rawterm::Pos size)
    : buffer(b), view_size(size), cursor(Cursor()) {}

inline void Viewport::draw(const std::size_t &start_point) {
    // start_point = the 0th line to print
    auto start = buffer->lines.begin() + start_point;
    for (auto it = start; it != start + view_size.vertical; ++it) {
        std::cout << *it << "\r\n";
    }

    std::cout << buffer->render_status_bar(view_size.horizontal);
    std::cout << "\r\n"; // TODO: Command palette
}

#endif // VIEWPORT_H
