#ifndef EDITOR_H
#define EDITOR_H

#include <string>
#include <vector>

#include <rawterm/rawterm.h>

#include "buffer.h"
#include "viewport.h"

enum class Mode { Read, Write, Command };

struct Editor {
    std::vector<Buffer> buffers;
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

inline Editor::Editor(std::string file)
    :
    : active_buffer(0), mode(Mode::Read), term_size(rawterm::get_term_size()) {
    rawterm::clear_screen();

    // TODO: Create a viewport
    Buffer b = file.empty() ? Buffer(this) : Buffer(this, file);
    buffers = { b };
}

inline void Editor::start(int line_num) {
    // TODO: Connect viewport and buffer
    buffers[0].init({ term_size.vertical - 2, term_size.horizontal }, line_num);
}

inline void Editor::set_mode(Mode m) { mode = m; }

[[nodiscard]] inline std::string Editor::get_mode() {
    switch (mode) {
    case Mode::Read:
        return "READ";
    case Mode::Write:
        return "WRITE";
    case Mode::Command:
        return "COMMAND";
    default:
        return "";
    }
}

#endif // EDITOR_H
