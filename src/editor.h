#ifndef EDITOR_H
#define EDITOR_H

#include <string>
#include <unordered_map>
#include <vector>

#include <rawterm/core.h>

#include "gapvector.h"
#include "model.h"
#include "view.h"

enum class Mode { Read, Write, Command };

// Model-View map
struct MVMapItem {
    unsigned int model;
    unsigned int view;
    unsigned int pane_in_view;
};

struct Editor {
    std::vector<Model> models = {};
    std::vector<View> views = {};
    std::vector<MVMapItem> model_view_map = {};
    unsigned int active_view = 0;  // index into views
    rawterm::Pos term_size;
    Mode mode;
    std::string git_branch;

    Editor();
    void init(const std::string&);
    void set_mode(Mode);
    [[nodiscard]] std::string get_mode() const;
    [[nodiscard]] int get_current_model() const;

    // controller.cpp
    void start_controller();
};

#endif  // EDITOR_H
