#include <string>

#include <cli11/CLI11.hpp>
#include <rawterm/rawterm.h>

#include "editor.h"
#include "logger.h"
#include "version.h"

const std::string log_file = "iris.log";
// TODO: Describe each function/method for future review

void version() {
    std::cout << "Iris version: " << git_tag() << "\n"
              << "Compiled from commit: " << git_hash() << "\n"
              << "Compiled on date: " << compile_date() << "\n"
              << "Build type: "
              << (release_mode().empty() ? "Default" : release_mode()) << "\n";
}

int main(int argc, char *argv[]) {
    CLI::App app{ "Iris text editor" };

    std::string file = "";
    int line_num = 0;

    bool print_version = false;
    app.add_option("file", file, "File to open");
    app.add_option("-l,--line", line_num, "Set line number to start on");
    app.add_flag("-v,--version", print_version, "Print version");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        return app.exit(e);
    }

    if (print_version) {
        version();
        return 0;
    }

    rawterm::enter_alt_screen();
    rawterm::enable_raw_mode();
    rawterm::enable_signals();

    Editor e(file);
    e.start(file.empty() ? 0 : line_num);

    rawterm::exit_alt_screen();

    return 0;
}
