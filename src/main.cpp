#include <iostream>
#include <string>

#include <argparse/argparse.hpp>
#include <rawterm/rawterm.h>

#include "editor.h"

int main(int argc, char *argv[]) {
    argparse::ArgumentParser parser("Iris", "v0.1.0");

    parser.add_argument("file").default_value("").help("Specify file to open");

    parser.parse_args(argc, argv);

    std::string file = parser.get<std::string>("file");

    enter_alt_screen();
    enable_raw_mode();

    Editor e(file);
    exit_alt_screen();

    return 0;
}
