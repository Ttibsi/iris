#ifndef VIEW_H
#define VIEW_H

#include <optional>
#include <string>
#include <vector>

#include <rawterm/color.h>
#include <rawterm/cursor.h>
#include <rawterm/screen.h>

#include "model.h"

using uint_t = unsigned int;

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
    std::size_t active_model = 0;          // 0-indexed
    rawterm::Cursor cur;
    int line_number_offset = 0;
    std::string command_text = ";";
    std::string git_branch = "";

    View(Controller*, const rawterm::Pos);
    void add_model(Model*);
    Model* get_active_model() const;
    void draw_screen();
    [[nodiscard]] const std::string render_screen() const;
    void draw_tab_bar();
    const std::string render_tab_bar() const;
    void draw_line(Draw_Line_dir::values);
    [[nodiscard]] const std::string render_line(const uint) const;
    void draw_status_bar();
    const std::string render_status_bar() const;
    [[maybe_unused]] const rawterm::Pos draw_command_bar();
    void display_message(std::string, std::optional<rawterm::Color>);
    [[nodiscard]] std::size_t clamp_horizontal_movement(const int);
    void cursor_left();
    [[maybe_unused]] bool cursor_up(unsigned int count = 1);
    [[maybe_unused]] bool cursor_down(unsigned int count = 1);
    void cursor_right();
    void cursor_end_of_line();
    void cursor_start_of_line();
    void center_current_line();
    void set_current_line(const unsigned int);
    void get_git_branch();
    void tab_new();
    void tab_next();
    void tab_prev();
    [[nodiscard]] uint_t visible_tab_bar() const;
    [[maybe_unused]] int set_lineno_offset(Model*);
    void update_cursor_on_model_change();
    void change_model_cursor();
    bool set_buffer(const std::size_t, const std::size_t);
};

#endif  // VIEW_H
