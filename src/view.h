#ifndef VIEW_H
#define VIEW_H

#include <rawterm/core.h>
#include <rawterm/cursor.h>
#include <rawterm/screen.h>

#include <string>
#include <vector>
#include "model.h"

struct file_pos_pair {
    std::string filename;
    rawterm::Pos pos;
};

struct Controller;

struct View {
    Controller* ctrlr_ptr;
    rawterm::Pos view_size;
    std::vector<file_pos_pair> open_files;
    std::vector<Model*> viewable_models;
    int active_model;
    rawterm::Cursor cur;
    int line_number_offset = 0;

    View(Controller*, const rawterm::Pos);
    Model* get_active_model();
    void add_model(Model*);
    void close_model();
    void change_view_forward();
    void change_view_backward();
    void render_screen();
    const std::string generate_tab_bar() const;
    void draw_status_bar();
    const std::string render_status_bar() const;
    void render_line();
    void set_status(const std::string& msg);
    void cursor_left();
    void cursor_up();
    void cursor_down();
    void cursor_right();

    // TODO: methods for cmd buffer (to name)
};

#endif  // VIEW_H
