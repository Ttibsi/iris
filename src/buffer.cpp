#include <algorithm>
#include <regex>
#include <span>
#include <string>
#include <string_view>

#include "buffer.h"
#include "constants.h"
#include "editor.h"
#include "file_manip.h"
#include "highlighting/languages.h"
#include "rawterm/rawterm.h"
#include "text_manip.h"
#include "viewport.h"

Buffer::Buffer(Editor *e)
    : editor(e), file("NO FILE"), lines({ "" }), readonly(false),
      modified(false), current_line(0) {
    if (LINE_NUMBER)
        lineno_offset = 2;
}

// TODO: What if the given path is a directory?
Buffer::Buffer(Editor *e, std::string filename)
    : editor(e), file(filename), lang(get_file_type(filename)),
      lines(open_file(filename)), readonly(is_readonly(filename)),
      modified(false), current_line(0) {

    if (LINE_NUMBER) {
        lineno_offset = std::to_string(lines.size()).size() + 1;
    }
}

void Buffer::init(rawterm::Pos view_size, int line_num) {
    view_size.horizontal -= lineno_offset;
    Viewport v = { this, view_size };
    view = &v;

    if (line_num) {
        view->center(line_num);
    } else {
        view->draw(0);
        view->cursor.set_pos_abs(1, 1, lineno_offset);
    }
    view->keypress_read();
    if (!quit_buf) {
        resize_handle();
    }
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
    if (!(git_branch.has_value())) {
        Response resp =
            shell_exec("git rev-parse --abbrev-ref HEAD 2>/dev/null", true);
        if (!(resp.retcode)) {
            git_branch = resp.stdout;
        } else {
            git_branch = "";
        }
    }

    if (!(git_branch.value().empty())) {
        // Remove spaces
        git_branch.value().erase(std::remove_if(git_branch.value().begin(),
                                                git_branch.value().end(),
                                                isspace),
                                 git_branch.value().end());
        left += git_branch.value() + " |";
    }

    std::string right = "| ";
    right += lang_string(lang) + " | ";
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
    if (c.col == 1) {
        lines.insert(lines.begin() + current_line, "");
        return;
    } else if (c.col >= lines[current_line].size()) {
        lines.insert(lines.begin() + current_line + 1, "");
        return;
    }

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
    if (cmd.size() < 2)
        return;
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
        Response resp = shell_exec(shell_cmd, true);
        if (resp.retcode) {
            std::cout << rawterm::fg(resp.stderr, rawterm::red) << "\r\n\n"
                      << rawterm::bold("Press ENTER to clear");
        } else {
            std::cout << resp.stdout << "\r\n\n"
                      << rawterm::bold("Press ENTER to clear");
        }
        bang_cmd_output = true;

        // Enter a number to go to that line ex `;24`
    } else if (is_numeric(cmd.substr(1, cmd.size()))) {
        unsigned int num = std::stoi(cmd.substr(1, cmd.size()));
        view->center(num);

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

std::optional<rawterm::Pos> Buffer::find_cmd() {
    using rawterm::Mod;
    std::vector<Mod> searchable = { Mod::None, Mod::Shift, Mod::Space };
    std::string prompt = "find > ";
    std::string search = "";

    while (true) {
        rawterm::move_cursor({ view->view_size.horizontal + 2, 1 });
        rawterm::clear_line();

        rawterm::move_cursor({ view->view_size.horizontal + 2, 1 });
        std::cout << prompt << search;
        rawterm::move_cursor({ view->view_size.horizontal + 2,
                               prompt.size() + search.size() + 1 });

        // Read and handle input
        rawterm::Key k = rawterm::process_keypress();
        rawterm::Mod modifier = rawterm::getMod(&k);

        if (modifier == Mod::Escape) {
            rawterm::move_cursor({ view->view_size.horizontal + 2, 1 });
            rawterm::clear_line();
            break;
        } else if (modifier == Mod::Enter) {
            rawterm::move_cursor({ view->view_size.horizontal + 2, 1 });
            rawterm::clear_line();

            std::span<std::string> line_span = lines;
            return find_in_text(
                line_span.subspan(current_line,
                                  line_span.size() - current_line),
                search);

        } else if (modifier == Mod::Backspace) {
            search = search.substr(0, search.size() - 1);
        } else if (std::find(searchable.begin(), searchable.end(), modifier) !=
                   searchable.end()) {
            search.push_back(k.code);
        }
    }

    return {};
}

unsigned int Buffer::replace_cmd() {
    using rawterm::Mod;
    std::vector<Mod> searchable = { Mod::None, Mod::Shift, Mod::Space };
    std::string prompt = "replace > ";
    std::string replace = "";

    while (true) {
        rawterm::move_cursor({ view->view_size.horizontal + 2, 1 });
        rawterm::clear_line();

        rawterm::move_cursor({ view->view_size.horizontal + 2, 1 });
        std::cout << prompt << replace;
        rawterm::move_cursor({ view->view_size.horizontal + 2,
                               prompt.size() + replace.size() + 1 });

        // Read and handle input
        rawterm::Key k = rawterm::process_keypress();
        rawterm::Mod modifier = rawterm::getMod(&k);

        if (modifier == Mod::Escape) {
            rawterm::move_cursor({ view->view_size.horizontal + 2, 1 });
            rawterm::clear_line();
            break;
        } else if (modifier == Mod::Enter) {
            rawterm::move_cursor({ view->view_size.horizontal + 2, 1 });
            rawterm::clear_line();

            replace_in_text(lines[current_line], view->cursor.col - 1, replace);
            return 1;

        } else if (modifier == Mod::Backspace) {
            replace = replace.substr(0, replace.size() - 1);
        } else if (std::find(searchable.begin(), searchable.end(), modifier) !=
                   searchable.end()) {
            replace.push_back(k.code);
        }
    }

    return 0;
}

void Buffer::resize_handle() {
    view->resize_flag = false;
    auto new_term_size = rawterm::get_term_size();
    editor->term_size = new_term_size;

    rawterm::clear_screen();
    init({ new_term_size.vertical - 2, new_term_size.horizontal },
         current_line);
}
