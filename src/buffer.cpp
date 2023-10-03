#include "buffer.h"
#include "constants.h"
#include "editor.h"
#include "file_manip.h"
#include "text_manip.h"
#include "viewport.h"

// TODO: opening rawterm_wrapper.h seems to have weird encoding
Buffer::Buffer(Editor *e)
    : editor(e), file("NO FILE"), lines({ "" }), readonly(false),
      modified(false), current_line(1) {}

// TODO: What if the given path is a directory?
// TODO: Handle opening an empty file - it segfaults otherwise
Buffer::Buffer(Editor *e, std::string filename)
    : editor(e), file(filename), lines(open_file(filename)),
      readonly(is_readonly(filename)), modified(false), current_line(1) {}

void Buffer::start(std::pair<std::size_t, std::size_t> view_size) {
    Viewport view = { this, view_size };
    view.draw(0);
    view.cursor.set_pos_abs(0, 0);
    view.handle_keypress();
}

std::string Buffer::render_status_bar(std::size_t width) {
    // Left - Mode, filename, modified/readonly, git branch
    // Right - file type, current/total line number

    std::string left = " " + editor->get_mode() + " | " + filename_only(file);
    if (modified) {
        left += " | [X] | ";
    } else if (readonly) {
        left += " | [RO] | ";
    } else {
        left += " | [ ] | ";
    }
    std::string git_branch =
        shell_exec("git rev-parse --abbrev-ref HEAD 2>/dev/null");
    if (!(git_branch.empty())) {
        left += git_branch + " |";
    }

    std::string right = "| ";
    right += get_file_type(file) + " | ";
    right +=
        std::to_string(current_line) + "/" + std::to_string(lines.size()) + " ";

    std::string ret =
        left + std::string(width - 1 - left.length() - right.length(), ' ') +
        right;
    return inverse(ret);
}

void Buffer::reset_status_bar(std::pair<std::size_t, std::size_t> dimensions) {
    save_cursor_position();
    // go to statusline pos
    move_cursor(dimensions.first + 1, 0);
    std::cout << render_status_bar(dimensions.second);
    load_cursor_position();
}

std::size_t Buffer::line_size(std::size_t idx) {
    std::size_t ret = lines[idx].size();
    if (lines[idx].find("\t") != std::string::npos) {
        ret += (TABSTOP - 1);
    }

    return ret;
}
