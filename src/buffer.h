#ifndef BUFFER_H
#define BUFFER_H

#include <string>
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

    Cursor() : row(0), col(0) { set_pos(0, 0); };
    void set_pos(int r, int c) { move_cursor(r, c); };
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

    Buffer();
    Buffer(std::string file);
    void display();
};

#endif // BUFFER_H
