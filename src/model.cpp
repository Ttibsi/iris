#include "model.h"

#include <format>

#include "constants.h"
#include "editor.h"

Model::Model(const Editor* e) : editor(e), gv(Gapvector()) {}

Model::Model(const Editor* e, Gapvector<> g, std::string file) : editor(e), gv(g), filename(file) {}

[[nodiscard]] const std::vector<std::string> Model::render(const View* view) const {
    int line_count = 0;
    std::vector<std::string> ret = {};
    ret.reserve(32);
    std::string placeholder;
    int linenum_offset = std::to_string(gv.line_count()).size() + 1;

    rawterm::Pos dims = view->pane_manager.get_size();

    if (LINE_NUMBERS) {
        placeholder +=
            rawterm::set_foreground(std::format("{:>{}}\u2502", 1, linenum_offset), COLOR_1);
    }

    for (auto&& c : gv) {
        placeholder += c;

        if (c == '\n') {
            if (static_cast<int>(placeholder.size()) > dims.horizontal - linenum_offset) {
                ret.push_back(placeholder.substr(0, dims.horizontal - linenum_offset));
            } else {
                ret.push_back(placeholder);
            }
            placeholder = "";
            line_count++;

            if (LINE_NUMBERS) {
                placeholder += rawterm::set_foreground(
                    std::format("{:>{}}\u2502", line_count + 1, linenum_offset), COLOR_1);
            }

            if (line_count == dims.vertical - 1) {
                break;
            }
        }
    }

    auto sb = render_status_bar(dims.horizontal);
    ret.push_back(sb);
    return ret;
}

[[nodiscard]] const std::string Model::render_status_bar(const int width) const {
    // left = mode | git branch | status (read_only/modified)
    // center = file name
    // right = language | cursor position
    std::string left = " " + editor->get_mode();
    if (!(editor->git_branch.empty())) {
        left += " | " + editor->git_branch;
    }

    if (readonly) {
        left += " | [RO]";
    } else if (modified) {
        left += " | [X]";
    }

    // TODO: file language after highlighting engine
    std::string right = "| " + std::to_string(current_line) + ":" + std::to_string(line_col) + " ";

    // TODO: handle overflows
    unsigned int divide = (width - (left.size() + filename.size() + right.size())) / 2;
    std::string ret = left + std::string(divide - (filename.size() / 2), ' ') + filename +
                      std::string(divide + (filename.size() / 2), ' ') + right;

    if (static_cast<int>(ret.size()) != width) {
        throw std::runtime_error(
            "ret size:" + std::to_string(ret.size()) + " width:" + std::to_string(width));
    }
    auto bg = rawterm::set_background(ret, COLOR_1);
    // auto fg = rawterm::set_foreground(bg, rawterm::Colors::white);
    return bg;
}
