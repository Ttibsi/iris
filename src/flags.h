#ifndef FLAGS_H
#define FLAGS_H

#include <string>

struct Flags {
    std::string file = "";
    std::size_t lineno = 0;
    bool print_version = false;
    bool readonly = false;
};

#endif  // FLAGS_H
