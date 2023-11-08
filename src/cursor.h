#ifndef CURSOR_H
#define CURSOR_H

#include <iostream>
#include <string>

#include <rawterm/rawterm.h>

// NOTE: Cursor represents the location in the terminal that the cursor is
// This isn't the location in the file, or the location in the string vector
// Cursor should be 1-indexed
struct Cursor {
    std::size_t row;
    std::size_t col;

    Cursor() : row(1), col(1) { set_pos_abs(1, 1); }
    void set_pos_abs(std::size_t r, std::size_t c);
    void set_pos_rel(std::size_t r, std::size_t c);
    friend std::ostream &operator<<(std::ostream &os, const Cursor &c);
};

// TODO: Bounds checking
// TODO: Do we want these two to take in a rawterm::Pos intead?
inline void Cursor::set_pos_abs(std::size_t r, std::size_t c) {
    row = r;
    col = c;
    rawterm::move_cursor({ r, c });
};

inline void Cursor::set_pos_rel(std::size_t r, std::size_t c) {
    row += r;
    col += c;
    rawterm::move_cursor({ row, col });
};

inline std::ostream &operator<<(std::ostream &os, const Cursor &c) {
    os << "Cursor{row: " << c.row << ", col: " << c.col << "}";
    return os;
};

#endif // CURSOR_H
