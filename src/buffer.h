#ifndef BUFFER_H
#define BUFFER_H

#include <string>
#include <vector>

#include "cursor.h"
#include "rawterm/rawterm.h"

struct Editor;

struct Buffer {
    Editor *editor;
    std::string file;
    std::vector<std::string> lines;
    bool readonly;
    bool modified;
    std::size_t current_line; // 0-indexed on the current line of lines
    bool quit_buf = false;
    std::size_t lineno_offset = 0;

    Buffer(Editor *);
    Buffer(Editor *, std::string);
    void init(rawterm::Pos);
    std::string render_status_bar(const std::size_t &, Cursor *);
    void reset_status_bar(rawterm::Pos, Cursor *);
    void split_lines(const Cursor &);
    void parse_command(const std::string &);
};

#endif // BUFFER_H
