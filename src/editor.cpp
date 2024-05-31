#include "editor.h"

#include <rawterm/color.h>

#include <format>

#include "constants.h"
#include "controller.h"
#include "filesystem.h"
#include "logger.h"

Editor::Editor() : term_size(rawterm::get_term_size()), mode(Mode::Read) {
    models.reserve(8);
    views.reserve(8);
    model_view_map.reserve(8);

    views.push_back(View(term_size));
}

void Editor::init(const std::string& file) {
    log("Opening file: " + file);
    auto file_chars = open_file(file);
    if (file_chars.has_value()) {
        log("Creating model & view");
        models.push_back(Gapvector(file_chars.value().begin(), file_chars.value().end()));

        log("Models count: " + std::to_string(models.size()));

        model_view_map.push_back({0, 0, 1});
        views.at(active_view).pane_manager.set_content(model_to_view());
    }
}

void Editor::start_controller() {
    log("Starting Controller");
    while (true) {
        switch (parse_input()) {
            case ParseInputRet::None:
                continue;
            case ParseInputRet::Break:
                log("Breaking out from Controller");
                return;
            case ParseInputRet::Redraw:
                views.at(active_view).pane_manager.draw_all();
        }
    }
}

std::vector<std::string> Editor::model_to_view() const {
    int line_count = 0;
    std::vector<std::string> ret;
    std::string placeholder;
    int linenum_offset = std::to_string(models.at(get_current_model()).line_count()).size() + 1;

    rawterm::Pos dims = views.at(active_view).pane_manager.get_size();

    if (LINE_NUMBERS) {
        placeholder +=
            rawterm::set_foreground(std::format("{:>{}}\u2502", 1, linenum_offset), COLOR_1);
    }

    for (auto&& c : models.at(get_current_model())) {
        placeholder += c;

        if (c == '\n') {
            if (static_cast<int>(placeholder.size()) > dims.horizontal - 3) {
                ret.push_back(placeholder.substr(0, dims.horizontal - 3));
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
    // TODO: statusbar

    return ret;
}

void Editor::set_mode(Mode m) {
    mode = m;
}

[[nodiscard]] std::string Editor::get_mode() const {
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

[[nodiscard]] int Editor::get_current_model() const {
    auto item =
        std::find_if(model_view_map.begin(), model_view_map.end(), [this](const MVMapItem& mv) {
            return mv.view == active_view &&
                   mv.pane_in_view == views.at(active_view).pane_manager.active();
        });

    return item->model;
}
