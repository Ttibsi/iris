#include "editor.h"

Editor::Editor(std::string file)
    : active_buffer(0), mode(Mode::Read), clipboard(""),
      term_size(rawterm::get_term_size()) {

    rawterm::clear_screen();

    Buffer b = file.empty() ? Buffer(this) : Buffer(this, file);
    buffers = { b };
}

void Editor::start(int line_num) {
    buffers[0].init({ term_size.vertical - 2, term_size.horizontal }, line_num);
}

void Editor::set_mode(Mode m) { mode = m; }

[[nodiscard]] std::string Editor::get_mode() {
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
