#include <iostream>
#include <string>

#include <cli11/CLI11.hpp>
#include <rawterm/core.h>

#include "controller.h"
#include "logger.h"
#include "version.h"

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

    // TODO: When we upgrade to c++23, use std::stacktrace here
    // CPPTRACE_TRY {
    try {
        Controller c;
        c.create_view(file);
        c.start_action_engine();
        // } CPPTRACE_CATCH(const std::exception& e) {
    } catch (const std::exception& e) {
        rawterm::exit_alt_screen();
        log(Level::ERROR, e.what());
        throw e;
        //     log(Level::ERROR, cpptrace::demangle(typeid(e).name()) + " " + e.what());
        //     cpptrace::from_current_exception().print();
    }

    rawterm::exit_alt_screen();
    return 0;
}
