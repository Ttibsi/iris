#ifndef EDITOR_H
#define EDITOR_H

#include <rawterm/core.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "gapvector.h"
#include "view.h"

enum class Mode { Read, Write, Command };

// Model-View map
struct MVMapItem {
    unsigned int model;
    unsigned int view;
    unsigned int pane_in_view;
};

struct Editor {
    std::vector<Gapvector<>> models = {};
    std::vector<View> views = {};
    std::vector<MVMapItem> model_view_map = {};
    unsigned int active_view = 0;  // index into views
    rawterm::Pos term_size;
    Mode mode;

    Editor();
    void init(const std::string&);
    void start_controller();
    std::vector<std::string> model_to_view() const;
    void set_mode(Mode);
    [[nodiscard]] std::string get_mode() const;
    [[nodiscard]] int get_current_model() const;
};

#endif  // EDITOR_H
