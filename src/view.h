#ifndef VIEW_H
#define VIEW_H

#include <rawterm/core.h>
#include <rawterm/extras/pane.h>

#include <string>
#include <unordered_map>
#include <vector>

struct View {
    rawterm::PaneManager<std::vector<std::string>> pane_manager;
    std::unordered_map<int, int> current_lines;

    explicit View(const rawterm::Pos&);
};

#endif  // VIEW_H
