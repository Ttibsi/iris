#ifndef CURSOR_H
#define CURSOR_H

#include <iostream>

#include <rawterm/rawterm.h>

// NOTE: Cursor represents the location in the terminal that the cursor is
// This isn't the location in the file, or the location in the string vector
// Cursor should be 1-indexed
struct Cursor {
    std::size_t row;
    std::size_t col;

    Cursor(std::size_t offset) : row(1), col(1) { set_pos_abs(1, 1, offset); }
    void set_pos_abs(std::size_t, std::size_t, std::size_t) noexcept;
    void set_pos_rel(std::size_t, std::size_t, std::size_t) noexcept;
    friend std::ostream &operator<<(std::ostream &os, const Cursor &c);
};

inline void Cursor::set_pos_abs(std::size_t r, std::size_t c,
                                std::size_t offset) noexcept {
    if (r >= 1 && c >= 1) {
        row = r;
        col = c;
        rawterm::move_cursor({ r, c + offset });
    }
};

inline void Cursor::set_pos_rel(std::size_t r, std::size_t c,
                                std::size_t offset) noexcept {
    if (row + r >= 1) {
        row += r;
    }
    if (col + c >= 1) {
        col += c;
    }
    rawterm::move_cursor({ row, col + offset });
};

inline std::ostream &operator<<(std::ostream &os, const Cursor &c) {
    os << "Cursor{row: " << c.row << ", col: " << c.col << "}";
    return os;
};

#endif // CURSOR_H
