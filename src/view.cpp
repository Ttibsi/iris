#include "view.h"
#include <stdexcept>

View::View(const rawterm::Pos& term_size)
    : pane_manager(rawterm::PaneManager<std::vector<std::string>>(term_size)) {}

void View::move_cur_left() {
    try {
        pane_manager.move_cursor_left();
    } catch(const std::out_of_range& e) {
        // TODO: Scroll view
    }
}

void View::move_cur_up() {
    try {
        pane_manager.move_cursor_up();
    } catch(const std::out_of_range& e) {
        // TODO: Scroll view
    }
}

void View::move_cur_down() {
    try {
        pane_manager.move_cursor_down();
    } catch(const std::out_of_range& e) {
        // TODO: Scroll view
    }
}

void View::move_cur_right() {
    try {
        pane_manager.move_cursor_right();
    } catch(const std::out_of_range& e) {
        // TODO: Scroll view
    }
}
