#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"
#include "version.h"
#include <cli11/CLI11.hpp>
#include <iostream>

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
    spdlog::set_pattern("[%H:%M:%S %z] [%n] [thread %t] [%l] %v");

    if (print_version) {
        std::cout << version();
        return 0;
    }
}
