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
    std::pair<std::size_t, std::size_t> term_size;

    Editor(std::string file);
    void start();
    void switch_mode(Mode m);
    std::string get_mode();
};

inline Editor::Editor(std::string file)
    : active_buffer(0), mode(Mode::Read), clipboard(""),
      term_size(get_term_size()) {
    clear_screen();

    Buffer b = file.empty() ? Buffer(this) : Buffer(this, file);
    buffers = { b };
}

inline void Editor::start() {
    std::pair<std::size_t, std::size_t> view_size{ term_size.first - 2,
                                                   term_size.second };

    buffers[0].start(view_size);
}

inline void Editor::switch_mode(Mode m) { mode = m; }

inline std::string Editor::get_mode() {
    switch (mode) {
    case Mode::Read:
        return "READ";
    case Mode::Write:
        return "WRITE";
    }

    return "";
}

#endif // EDITOR_H
