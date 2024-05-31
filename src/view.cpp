#include "view.h"

View::View(const rawterm::Pos& term_size)
    : pane_manager(rawterm::PaneManager<std::vector<std::string>>(term_size)) {}
