#include <algorithm>
#include <string_view>

#include "buffer.h"
#include "constants.h"
#include "editor.h"
#include "file_manip.h"
#include "text_manip.h"
#include "viewport.h"

Buffer::Buffer(Editor *e)
    : editor(e), file("NO FILE"), lines({ "" }), readonly(false),
      modified(false), current_line(0) {}

// TODO: What if the given path is a directory?
// TODO: Handle opening an empty file - it segfaults otherwise
Buffer::Buffer(Editor *e, std::string filename)
    : editor(e), file(filename), lines(open_file(filename)),
      readonly(is_readonly(filename)), modified(false), current_line(0) {}

void Buffer::init(rawterm::Pos view_size) {
    Viewport view = { this, view_size };
    view.draw(0);
    view.cursor.set_pos_abs(1, 1);
    view.keypress_read();
}

std::string Buffer::render_status_bar(const std::size_t &width, Cursor *c) {
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

    // NOTE: No branch/git = empty string
    std::string git_branch =
        shell_exec("git rev-parse --abbrev-ref HEAD 2>/dev/null");
    if (!(git_branch.empty())) {
        left += git_branch + " |";
    }

    std::string right = "| ";
    right += get_file_type(file) + " | ";
    if (CURSOR_STATUS) {
        right += "Cursor: (" + std::to_string(c->row) + ":" +
                 std::to_string(c->col) + ") | ";
    }
    right += std::to_string(current_line + 1) + "/" +
             std::to_string(lines.size()) + " ";

    std::string ret =
        left + std::string(width - left.length() - right.length(), ' ') + right;

    return rawterm::inverse(ret);
}

void Buffer::reset_status_bar(rawterm::Pos dimensions, Cursor *c) {
    // go to statusline pos
    rawterm::move_cursor({ dimensions.vertical + 1, 0 });
    std::cout << render_status_bar(dimensions.horizontal, c);

    // Restore cursor pos
    rawterm::move_cursor({ c->row, c->col });
}

void Buffer::split_lines(const Cursor &c) {
    lines[current_line].insert(c.col - 1, "\n");
    const std::string &line = lines[current_line];

    std::string l1;
    std::string l2;
    if (line.find("\n") != std::string::npos) {
        l1 = line.substr(0, line.find("\n"));
        l2 = line.substr(line.find("\n") + 1, line.size());
    }

    lines[current_line] = l1;
    lines.insert(lines.begin() + current_line + 1, l2);
}

void Buffer::parse_command(const std::string &cmd) {
    using namespace std::literals;
    if (cmd.starts_with(";w"sv)) {
        if (!(cmd.size() == 2)) {
            file = cmd.substr(3, cmd.size());
        }

        int bytes = write_to_file(file, lines);
        std::cout << "\"" << file << "\": " << bytes << " bytes written";
        modified = false;
    } else if (cmd == ";q"sv) {
        if (!(modified)) {
            quit_buf = true;
        } else {
            std::cout << "File unsaved - do ;q! to discard changes";
        }
    } else if (cmd == ";q!"sv) {
        quit_buf = true;
    }

    return;
}
