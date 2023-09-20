#include <iostream>
#include <string>

#include <chrono>
#include <thread>
#include <argparse/argparse.hpp>

#include "editor.h"
#define RAWTERM_IMPLEMENTATION
#include <rawterm/rawterm.h>


int main(int argc, char *argv[]) {
    argparse::ArgumentParser parser("Iris", "v0.1.0");

    parser.add_argument("file").default_value("").help("Specify file to open");

    parser.parse_args(argc, argv);

    std::string file = parser.get<std::string>("file");
    if (file != "") {
        std::cout << "File specified: " << file << "\n";
    } else {
        std::cout << "No file specified\n";
    }

    enter_alt_screen();
    enable_raw_mode();

    Editor e(file);
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    exit_alt_screen();

    return 0;
}
