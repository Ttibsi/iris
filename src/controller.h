#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <string>

#include <rawterm/core.h>

#include "model.h"
#include "text_io.h"
#include "view.h"

enum class Mode { Read, Write, Command };
enum class RedrawType { Line, Screen, None };
enum class QuitAll { Close, Redraw };

struct Redraw {
    RedrawType type;
    int count;

    explicit Redraw(RedrawType t, int num) : type(t), count(num) {}
    explicit Redraw(RedrawType t) : type(t), count(0) {}
};

struct Controller {
    rawterm::Pos term_size;
    std::vector<Model> models = {};
    std::vector<Model> meta_buffers = {};
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
    [[nodiscard]] bool is_readonly_model();
    [[nodiscard]] bool quit_app(bool);
    [[nodiscard]] bool check_for_saved_file(bool);
    void add_model(const std::string&);
    [[nodiscard]] WriteAllData write_all();
    [[nodiscard]] QuitAll quit_all();
    [[nodiscard]] bool display_all_buffers();
};

#endif  // CONTROLLER_H
