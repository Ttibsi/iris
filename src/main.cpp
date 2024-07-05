#include <iostream>
#include <string>

#include <cli11/CLI11.hpp>
#include <rawterm/core.h>

#include "editor.h"
#include "logger.h"

int main(int argc, char* argv[]) {
    CLI::App app {"Iris text editor"};

    std::string file = "";
    int line_num = 0;

    bool print_version = false;
    app.add_option("file", file, "File to open");
    app.add_option("-l,--line", line_num, "Set line number to start on");
    app.add_flag("-v,--version", print_version, "Print version");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return app.exit(e);
    }

    log("Iris Startup");
    rawterm::enter_alt_screen();
    rawterm::enable_raw_mode();
    rawterm::enable_signals();

    Editor e;
    if (!file.empty()) {
        e.init(file);
    }
    e.start_controller();
    log("Iris shutdown");

    rawterm::exit_alt_screen();
    return 0;
}
