#include "model.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <format>
#include <print>
#include <string>
#include <utility>

#include "constants.h"
#include "editor.h"

Model::Model(const Editor* e) : editor(e), gv(Gapvector()) {}

Model::Model(const Editor* e, Gapvector<> g, std::string file) : editor(e), gv(g), filename(file) {}

[[nodiscard]] std::vector<std::string> Model::render(const View& view, const rawterm::Pos& offset) {
    int line_count = 0;
    std::vector<std::string> ret = {};
    ret.reserve(32);
    std::string placeholder;
    linenum_offset = std::to_string(gv.line_count()).size() + 1;

    rawterm::Pos dims = view.pane_manager.get_size();

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

    for (auto i = line_count; i < dims.vertical - 1; i++) {
        ret.push_back("\n");
    }
    ret.push_back(render_status_bar(dims.horizontal));
    assert(ret.size() == static_cast<unsigned long>(dims.vertical));
    return ret;
}

[[nodiscard]] std::vector<std::string> Model::render(const View& view) {
    return render(view, {0, 0});
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
    std::int32_t space_per_elem = std::max(0, width / 3);
    std::string file_path = filename.size() > static_cast<std::size_t>(space_per_elem)
                                ? std::filesystem::path(filename).filename().string()
                                : filename;
    file_path = file_path.substr(
        std::max(0, static_cast<int32_t>(file_path.size()) - space_per_elem), space_per_elem);
    left.resize(space_per_elem, ' ');
    right.resize(space_per_elem, ' ');

    std::size_t wsc = std::max(1, width - (space_per_elem * 3));
    auto whitespace = std::string(wsc, ' ');

    right = std::format("{:>} {:>}", file_path, right);
    left.resize(width - right.size(), ' ');
    std::string ret = std::format("{:<}{:>}", left, right);

    if (static_cast<int>(ret.size()) != width)
        throw std::runtime_error(std::format("ret size:{} width:{}", ret.size(), width));

    return rawterm::set_background(ret, COLOR_1);
}
