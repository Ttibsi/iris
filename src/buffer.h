#ifndef BUFFER_H
#define BUFFER_H

#include <string>
#include <vector>

struct Buffer {
    std::string file;
    // Language lang = Language::UNKNOWN; // TODO: highlighting
    std::vector<std::string> lines;
    bool readonly;
    bool modified;
    int current_line; // 0-indexed on the current line of lines

    Buffer();
};

#endif // BUFFER_H
