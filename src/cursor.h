#ifndef CURSOR_H
#define CURSOR_H

#include <iostream>
#include <string>

#include <rawterm/rawterm.h>

struct Cursor {
    std::size_t row;
    std::size_t col;

    Cursor() : row(0), col(0) { set_pos_abs(0, 0); }
    void set_pos_abs(std::size_t r, std::size_t c);
    void set_pos_rel(std::size_t r, std::size_t c);
    friend std::ostream &operator<<(std::ostream &os, const Cursor &c);
};

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
