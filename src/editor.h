#ifndef EDITOR_H
#define EDITOR_H

#include <string>
#include <vector>

#include <rawterm/rawterm.h>

#include "buffer.h"
#include "viewport.h"

enum class Mode { Read, Write, Command };

struct Editor {
    std::vector<Buffer> buffers; // TODO: Check we still need this
    std::vector<Viewport> viewports;
    int active_viewport;
    Mode mode;
    rawterm::Pos term_size;
    // std::string clipboard;

    Editor(std::string);
    void start(int);
    void set_mode(Mode m);
    std::string get_mode();
};

#endif // EDITOR_H
