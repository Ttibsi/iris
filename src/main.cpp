#include <string>

#include <cli11/CLI11.hpp>
#include <rawterm/rawterm.h>

#include "editor.h"

// TODO: Describe each function/method for future review

int main(int argc, char *argv[]) {
    CLI::App app{ "Iris text editor" };

    std::string file = "";
    int line_num = 0;
    app.add_option("file", file, "File to open");
    app.add_option("-l,--line", line_num, "Set line number to start on");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        return app.exit(e);
    }

    rawterm::enter_alt_screen();
    rawterm::enable_raw_mode();

    Editor e(file);
    e.start(file.empty() ? 0 : line_num);

    rawterm::exit_alt_screen();

    return 0;
}
