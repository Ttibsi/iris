#include <iostream>
#include <string>

#include <cli11/CLI11.hpp>
#include <cpptrace/from_current.hpp>
#include <rawterm/core.h>

#include "controller.h"
#include "logger.h"
#include "version.h"

// TODO: Fix these bugs before it can be used properly
// Double check drawing of chevron on truncated lines
// backspace on newline error
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

    if (print_version) {
        std::cout << version();
        return 0;
    }

    log("Iris Startup");
    rawterm::enter_alt_screen();
    rawterm::enable_raw_mode();
    rawterm::enable_signals();

    CPPTRACE_TRY {
        Controller c;
        c.create_view(file);
        c.start_action_engine();
    }
    CPPTRACE_CATCH(const std::exception& e) {
        rawterm::exit_alt_screen();
        log(Level::WARNING, cpptrace::demangle(typeid(e).name()) + " " + e.what());
        cpptrace::from_current_exception().print();
    }

    rawterm::exit_alt_screen();
    return 0;
}
