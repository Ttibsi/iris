#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <utility>

#include <rawterm/rawterm.h>

#include "buffer.h"
#include "cursor.h"
#include "text_manip.h"

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
    std::vector<std::string> lines = filter_whitespace(buffer->lines);
    auto start = lines.begin() + std::min(start_point, lines.size() - 1);
    std::size_t end =
        std::max(std::min(view_size.vertical, buffer->lines.size()),
                 static_cast<unsigned long>(1));

    for (auto it = start; it < start + end; ++it) {
        std::cout << *it;
    }

    if (view_size.vertical > end) {
        for (unsigned long i = end; i < view_size.vertical; i++) {
            std::cout << "\r\n";
        }
    }

    std::cout << buffer->render_status_bar(view_size.horizontal, &cursor);
    std::cout << "\r\n"; // TODO: Command palette
}

#endif // VIEWPORT_H
