#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <rawterm/core.h>

#include "model.h"
#include "view.h"

enum class Mode { Read, Write, Command };
enum class Redraw { Line, Screen, None };

struct Controller {
    rawterm::Pos term_size;
    std::vector<Model> models = {};
    View view;
    Mode mode = Mode::Read;
    bool quit_flag = false;

    Controller();
    void set_mode(Mode m);
    [[nodiscard]] const std::string get_mode() const;
    void create_view(const std::string&, const unsigned long);
    void start_action_engine();
    bool enter_command_mode();
    bool parse_command();
};

#endif  // CONTROLLER_H
