#include <print>
#include <string>

#include <cli11/CLI11.hpp>
#include <rawterm/core.h>
#include <rawterm/cursor.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include "controller.h"
#include "text_io.h"
#include "version.h"

// TODO: A way of detecting if the file is already open in another iris
// instance
int main(int argc, char* argv[]) {
    CLI::App app {"Iris text editor"};

    std::string file = "";
    unsigned int lineno = 0;
    bool print_version = false;

    app.add_option("file", file, "File to open");
    app.add_option("-l,--line", lineno, "Set line number to start on");
    app.add_flag("-v,--version", print_version, "Print version");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return app.exit(e);
    }

    if (print_version) {
        std::println("{}", version());
        return 0;
    }

    auto filename_error = check_filename(file);
    if (!(filename_error.empty())) {
        std::println("{}", filename_error);
        return 0;
    }

    try {
        auto logger = spdlog::basic_logger_mt("basic_logger", "iris.log");
    } catch (const spdlog::spdlog_ex& ex) {
        std::println("Log init failed: {}", ex.what());
    }
    spdlog::set_pattern("[%H:%M:%S %z] [thread %t] [%l] %v");

    spdlog::get("basic_logger")->info("Iris startup");
    rawterm::enter_alt_screen();
    rawterm::enable_raw_mode();

    try {
        std::println("Setup...");
        Controller c;
        c.create_view(file, lineno);
        c.start_action_engine();
    } catch (const std::exception& e) {
        rawterm::exit_alt_screen();
        rawterm::Cursor::cursor_block();
        spdlog::get("basic_logger")->info(e.what());
        std::println("{}", e.what());
    }

    rawterm::exit_alt_screen();
    return 0;
}
