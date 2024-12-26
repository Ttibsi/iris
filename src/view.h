#ifndef VIEW_H
#define VIEW_H

#include <vector>

#include "model.h"

#include <rawterm/cursor.h>
#include <rawterm/screen.h>

struct Controller;

struct View {
    Controller *ctrlr_ptr;
    rawterm::Pos view_size;
    std::vector<Model *> viewable_models = {}; // Indexes into controller.models
    int active_model;
    rawterm::Cursor cur;
    int line_number_offset = 0;

    View(Controller *, const rawterm::Pos);
    void add_model(Model *);
    Model *get_active_model() const;
    void draw_screen();
    const std::string render_screen() const;
    const std::string render_tab_bar() const;
    void draw_status_bar();
    const std::string render_status_bar() const;
};

#endif // VIEW_H
