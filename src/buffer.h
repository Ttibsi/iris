#ifndef BUFFER_H
#define BUFFER_H

#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <rawterm/rawterm.h>

struct Line {
    int start;
    int end;

    Line() : start(0), end(0) {}
};

struct Cursor {
    int row;
    int col;

    Cursor() : row(0), col(0) { set_pos_abs(0, 0); };
    void set_pos_abs(int r, int c) {
        row = r;
        col = c;
        move_cursor(r, c);
    };
    void set_pos_rel(int r, int c) {
        row += r;
        col += c;
        move_cursor(row, col);
    };
    friend std::ostream &operator<<(std::ostream &os, const Cursor &c) {
        os << "Cursor{row: " << c.row << ", col: " << c.col << "}";
        return os;
    };
};

struct Change {
    int start_pos;
    int end_pos;
    std::vector<char> content;
};

struct Buffer {
    std::vector<Line> lines;
    std::string data;
    int length;

    std::string file;
    Cursor cursor;
    bool readonly;
    bool modified;
    std::vector<Change> undo;
    std::vector<Change> redo;
    std::pair<int, int> buf_size;

    Buffer() : Buffer(get_term_size()) {}
    Buffer(std::pair<int, int> size);

    Buffer(std::string file) : Buffer(file, get_term_size()) {}
    Buffer(std::string file, std::pair<int, int> size);
    void display();
    void handle_keypress();
};

#endif // BUFFER_H
