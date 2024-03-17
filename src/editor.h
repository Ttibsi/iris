#ifndef EDITOR_H
#define EDITOR_H

#include <string>
#include <vector>

#include <rawterm/rawterm.h>

#include "buffer.h"

enum class Mode { Read, Write, Command };

struct Editor {
    std::vector<Buffer> buffers;
    int active_buffer;
    Mode mode;
    std::string clipboard;
    rawterm::Pos term_size;

    Editor(std::string);
    void start(int);
    void set_mode(Mode m);
    [[nodiscard]] std::string get_mode();
};

#endif // EDITOR_H
