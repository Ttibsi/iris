#ifndef BUFFER_H
#define BUFFER_H

#include <string>
#include <vector>

struct Editor;

struct Buffer {
    Editor *editor;
    std::string file;
    std::vector<std::string> lines;
    bool readonly;
    bool modified;
    std::size_t current_line;

    Buffer(Editor *e);
    Buffer(Editor *e, std::string filename);
    void start(std::pair<std::size_t, std::size_t> view_size);
    std::string render_status_bar(std::size_t width);
    std::size_t line_size(std::size_t idx);
    void reset_status_bar(std::pair<std::size_t, std::size_t> dimensions);
};

#endif // BUFFER_H
