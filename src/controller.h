#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <string>
#include <vector>

#include <rawterm/core.h>

#include "model.h"
#include "view.h"

enum class Mode { Read, Write, Command };

struct Controller {
    rawterm::Pos term_size;
    std::vector<Model> models = {};
    View view;
    Mode mode = Mode::Read;
    std::string git_branch;

    Controller();
    void set_mode(Mode m);
    const std::string get_mode() const;
    void create_view(const std::string& file);
    void start_action_engine();
};

#endif  // CONTROLLER_H
