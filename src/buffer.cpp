#include <algorithm>
#include <regex>
#include <string_view>

#include "buffer.h"
#include "constants.h"
#include "editor.h"
#include "file_manip.h"
#include "rawterm/rawterm.h"
#include "text_manip.h"
#include "viewport.h"

// lineno_offset has +3 because that's the byte length of \u2502

Buffer::Buffer(Editor *e)
    : editor(e), file("NO FILE"), lines({ "" }), readonly(false),
      modified(false), current_line(0) {
    if (LINE_NUMBER)
        lineno_offset = 2;
}

// TODO: What if the given path is a directory?
Buffer::Buffer(Editor *e, std::string filename)
    : editor(e), file(filename), lines(open_file(filename)),
      readonly(is_readonly(filename)), modified(false), current_line(0) {

    if (LINE_NUMBER) {
        lineno_offset = std::to_string(lines.size()).size() + 1;
    }
}

void Buffer::init(rawterm::Pos view_size, int line_num) {
    view_size.horizontal -= lineno_offset;
    Viewport view = { this, view_size };

    if (line_num) {
        view.center(line_num);
    } else {
        view.draw(0);
        view.cursor.set_pos_abs(1, 1, lineno_offset);
    }
    view.keypress_read();
}

std::string Buffer::render_status_bar(const std::size_t &width, Cursor *c) {
    // Left - Mode, filename, modified/readonly, git branch
    // Right - file type, current/total line number

    std::string left = " " + editor->get_mode() + " | " + filename_only(file);
    if (readonly) {
        left += " | [RO] | ";
    } else if (modified) {
        left += " | [X] | ";
    } else {
        left += " | [ ] | ";
    }

    // NOTE: No branch/git = empty string
    std::string git_branch =
        shell_exec("git rev-parse --abbrev-ref HEAD 2>/dev/null", true);
    if (!(git_branch.empty())) {
        git_branch.erase(
            std::remove_if(git_branch.begin(), git_branch.end(), isspace),
            git_branch.end());
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
    rawterm::clear_line();
    std::cout << render_status_bar(dimensions.horizontal + lineno_offset, c);

    // Restore cursor pos
    rawterm::move_cursor({ c->row, c->col + lineno_offset });
}

void Buffer::split_lines(const Cursor &c) {
    std::size_t pos =
        c.col - 1 + (lines[current_line][0] == '\t' ? TABSTOP : 0);
    lines[current_line].insert(pos, "\n");
    const std::string &line = lines[current_line];

    std::string l1;
    std::string l2;

    if (line.find("\n") != std::string::npos) {
        l1 = line.substr(0, line.find("\n"));
        l2 = std::regex_replace(line.substr(line.find("\n") + 1, line.size()),
                                std::regex("\n"), "");
    }

    lines[current_line] = l1;
    lines.insert(lines.begin() + current_line + 1, l2);
}

void Buffer::parse_command(const std::string &cmd) {
    using namespace std::literals;

    // Bang shell commands (ie `;!ls -la`)
    if (cmd.starts_with(";!"sv)) {
        // NOTE: Run cmd without output
        std::string shell_cmd = "";
        shell_cmd += cmd.substr(2, cmd.size());
        shell_exec(shell_cmd, false);

    } else if (cmd.starts_with(";.!"sv)) {
        // TODO: Wrie the output of a command to a buffer

        std::string shell_cmd = "";
        shell_cmd += cmd.substr(3, cmd.size());
        std::string ret = shell_exec(shell_cmd, true);
        std::cout << ret << "\r\n\n" << rawterm::bold("Press ENTER to clear");
        bang_cmd_output = true;

    } else if (cmd.starts_with(";wq"sv)) {
        int bytes = write_to_file(file, lines);
        if (bytes == -1) {
            std::cout << "FAILED: No filename specified";
        } else {
            std::cout << "\"" << file << "\": " << bytes << " bytes written";
            modified = false;
            quit_buf = true;
        }

    } else if (cmd.starts_with(";w"sv)) {
        if (!(cmd.size() == 2)) {
            file = cmd.substr(3, cmd.size());
        }

        int bytes = write_to_file(file, lines);
        if (bytes == -1) {
            std::cout << "FAILED: No filename specified";
        } else {
            std::cout << "\"" << file << "\": " << bytes << " bytes written";
            modified = false;
        }
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
