#include <iostream>

#include "controller.h"
#include "version.h"

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"

#include <cli11/CLI11.hpp>
#include <rawterm/core.h>

int main(int argc, char *argv[]) {
    CLI::App app{ "Iris text editor" };

    std::string file = "";
    bool print_version = false;

    app.add_option("file", file, "File to open");
    app.add_flag("-v,--version", print_version, "Print version");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        return app.exit(e);
    }

    try {
        auto logger = spdlog::basic_logger_mt("basic_logger", "iris.log");
    } catch (const spdlog::spdlog_ex &ex) {
        std::cout << "Log init failed: " << ex.what() << std::endl;
    }
    spdlog::set_pattern("[%H:%M:%S %z] [thread %t] [%l] %v");

    if (print_version) {
        std::cout << version();
        return 0;
    }

    spdlog::get("basic_logger")->info("Iris startup");
    rawterm::enter_alt_screen();
    rawterm::enable_raw_mode();
    rawterm::enable_signals();

    try {
        Controller c;
        c.create_view(file);
        c.start_action_engine();
    } catch (const std::exception &e) {
        rawterm::exit_alt_screen();
        spdlog::get("basic_logger")->error(e.what());
        throw e;
    }

    rawterm::exit_alt_screen();
    return 0;
}
