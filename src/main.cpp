#include <iostream>
#include <string>

#include <cli11/CLI11.hpp>
#include <rawterm/rawterm.h>

#include "editor.h"

// TODO: Work our what copies could be references
// TODO: Describe each function/method for future review

int main(int argc, char *argv[]) {
    CLI::App app{ "Iris text editor" };

    std::string file = "";
    app.add_option("file", file, "File to open");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        return app.exit(e);
    }

    rawterm::enter_alt_screen();
    rawterm::enable_raw_mode();

    Editor e(file);
    e.start();

    rawterm::exit_alt_screen();

    return 0;
}
