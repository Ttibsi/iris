#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <utility>

#include <rawterm/rawterm.h>

#include "buffer.h"
#include "cursor.h"

struct Viewport {
    Buffer *buffer;
    std::pair<std::size_t, std::size_t> view_size;
    Cursor cursor;

    Viewport(Buffer *b, std::pair<std::size_t, std::size_t> size);
    void draw(std::size_t start_point);
    void handle_keypress();
};

inline Viewport::Viewport(Buffer *b, std::pair<std::size_t, std::size_t> size)
    : buffer(b), view_size(size), cursor(Cursor()) {}

inline void Viewport::draw(std::size_t start_point) {
    // start_point = the 0th line to print
    auto start = buffer->lines.begin() + start_point;
    for (auto it = start; it != start + view_size.first; ++it) {
        std::cout << *it << "\r\n";
    }

    std::cout << buffer->render_status_bar(view_size.second);
    std::cout << "\r\n"; // TODO: Command palette
}

#endif // VIEWPORT_H
