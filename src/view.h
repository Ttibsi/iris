#ifndef VIEW_H
#define VIEW_H

#include <string>
#include <vector>

#include <rawterm/cursor.h>
#include <rawterm/screen.h>

#include "model.h"

struct Controller;

// Namespaced instead of an `enum class` because we want the int value but
// still encapsulated
namespace Draw_Line_dir {
    enum values : short { Prev = -1, None = 0, Next = 1 };
}

struct View {
    Controller* ctrlr_ptr;
    rawterm::Pos view_size;
    std::vector<Model*> view_models = {};  // Indexes into controller.models
    int active_model = 0;                  // 0-indexed
    rawterm::Cursor cur;
    int line_number_offset = 0;
    std::string command_text = ";";
    int prev_cur_hor_pos = -1;

    View(Controller*, const rawterm::Pos);
    void add_model(Model*);
    Model* get_active_model() const;
    void draw_screen();
    [[nodiscard]] const std::string render_screen() const;
    const std::string render_tab_bar() const;
    void draw_line(Draw_Line_dir::values);
    [[nodiscard]] const std::string render_line(const unsigned int) const;
    void draw_status_bar();
    const std::string render_status_bar() const;
    [[maybe_unused]] const rawterm::Pos draw_command_bar();
    [[nodiscard]] int clamp_horizontal_movement(const int);

    void cursor_left();
    [[maybe_unused]] bool cursor_up();
    [[maybe_unused]] bool cursor_down();
    void cursor_right();
};

#endif  // VIEW_H
