#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <rawterm/core.h>

#include "view.h"

enum class Mode { Read, Write, Command };

struct Controller {
    rawterm::Pos term_size;
    View view;
    Mode mode = Mode::Read;
    std::string git_branch = "";

    Controller();
    void set_mode(Mode m);
    [[nodiscard]] const std::string get_mode() const;
    void create_view(const std::string &);
    void start_action_engine();
};

#endif // CONTROLLER_H
