#ifndef BUFFER_H
#define BUFFER_H

#include <optional>
#include <string>
#include <vector>

#include "cursor.h"
#include "highlighting/languages.h"
#include "rawterm/rawterm.h"

struct Editor;
struct Viewport;

struct Buffer {
    Editor *editor;
    Viewport *view;
    std::string file;
    Language lang = Language::UNKNOWN;
    std::vector<std::string> lines;
    bool readonly;
    bool modified;
    std::size_t current_line; // 0-indexed on the current line of lines
    bool quit_buf = false;
    std::size_t lineno_offset = 0;
    bool bang_cmd_output = false;
    std::optional<std::string> git_branch;

    Buffer(Editor *);
    Buffer(Editor *, std::string);
    void init(rawterm::Pos, int);
    std::string render_status_bar(const std::size_t &, Cursor *);
    void reset_status_bar(rawterm::Pos, Cursor *);
    void split_lines(const Cursor &);
    void parse_command(const std::string &);
};

#endif // BUFFER_H
