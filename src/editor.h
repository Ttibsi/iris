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
    std::size_t active_buffer;
    Mode mode;
    std::string clipboard;
    rawterm::Pos term_size;

    Editor(std::string file);
    void start();
    void set_mode(Mode m);
    std::string get_mode();
};

inline Editor::Editor(std::string file)
    : active_buffer(0), mode(Mode::Read), clipboard(""),
      term_size(rawterm::get_term_size()) {

    rawterm::clear_screen();
    Buffer b = file.empty() ? Buffer(this) : Buffer(this, file);
    buffers = { b };
}

inline void Editor::start() {
    buffers[0].init({ term_size.vertical - 2, term_size.horizontal });
}

inline void Editor::set_mode(Mode m) { mode = m; }

inline std::string Editor::get_mode() {
    switch (mode) {
    case Mode::Read:
        return "READ";
    case Mode::Write:
        return "WRITE";
    default:
        return "";
    }
}

#endif // EDITOR_H
