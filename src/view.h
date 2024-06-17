#ifndef VIEW_H
#define VIEW_H

#include <string>
#include <unordered_map>
#include <vector>

#include <rawterm/core.h>
#include <rawterm/extras/pane.h>

struct View {
    rawterm::PaneManager<std::vector<std::string>> pane_manager;
    std::unordered_map<int, int> current_lines;

    explicit View(const rawterm::Pos&);
    void move_cur_left();
    void move_cur_up();
    void move_cur_down();
    void move_cur_right();
};

#endif  // VIEW_H
