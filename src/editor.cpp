#include "editor.h"

#include <rawterm/color.h>

#include "constants.h"
#include "filesystem.h"
#include "logger.h"
#include "model.h"
#include "text_transform.h"

Editor::Editor() : term_size(rawterm::get_term_size()) {
    models.reserve(8);
    views.reserve(8);
    model_view_map.reserve(8);

    views.emplace_back(term_size);

    auto get_git_branch = shell_exec("git rev-parse --abbrev-ref HEAD", true);
    if (get_git_branch.has_value()) {
        git_branch = strip_newline(strip_trailing_whitespace(get_git_branch.value().stdout));
    }
}

void Editor::init(const std::string& file) {
    log("Opening file: " + file);
    std::optional file_chars = open_file(file);
    if (file_chars) {
        log("Creating model & view");
        models.emplace_back(this, *file_chars, file);
    }

    log("Models count: " + std::to_string(models.size()));

    model_view_map.push_back({0, 0, 1});
    auto rendered_content = models.at(0).render(views.at(active_view));
    views.at(active_view).pane_manager.set_content(rendered_content);
    auto bl = rawterm::Region(
        {1, 1},
        {views.at(active_view).pane_manager.get_size().vertical, models.at(0).linenum_offset + 1});

    log("Region size: " + bl.bottom_right.toStr());

    views.at(active_view).pane_manager.set_blacklist_region(bl);
    views.at(active_view).redraw_pane();
    // TODO: else create new empty buffer
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
