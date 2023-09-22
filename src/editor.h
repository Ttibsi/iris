#ifndef EDITOR_H
#define EDITOR_H

#include <string>
#include <utility>
#include <vector>

#include <rawterm/rawterm.h>

#include "buffer.h"

enum class Mode { Read, Write };

struct Editor {
    std::vector<Buffer> buffers;
    int active_buffer;
    Mode mode;
    std::string clipboard;
    std::pair<int, int> term_size;

    Editor(std::string file);
    ~Editor();
};

Editor::Editor(std::string file) {
    clear_screen();
    Buffer b = file.empty() ? Buffer() : Buffer(file);

    // Create buffer
    this->buffers = { b };
    this->active_buffer = 0;
    this->mode = Mode::Read;
    this->clipboard = "";
    this->term_size = get_term_size();
}

Editor::~Editor() {}

#endif // EDITOR_H
