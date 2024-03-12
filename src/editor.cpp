#include "editor.h"
#include "buffer.h"
#include "viewport.h"

Editor::Editor(std::string file)
    : active_buffer(0), mode(Mode::Read), term_size(rawterm::get_term_size()) {
    rawterm::clear_screen();

    // NOTE: Potentially we don't need to store the buffers, we just
    // use the shared_ptrs instead - when a new viewport is created, it also
    // takes a new shared_ptr to the same buffer until we change it (or
    // unless it's something else)
    auto b = std::make_shared<Buffer>(file.empty() ? Buffer(this)
                                                   : Buffer(this, file));
    Viewport v = Viewport({ term_size.vertical - 1, term_size.horizontal }, b);

    viewports = { v };
}

void Editor::start(int line_num) {
    // TODO: Connect viewport and buffer
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
