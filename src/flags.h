#ifndef FLAGS_H
#define FLAGS_H

#include <string>

struct Flags {
    std::string file = "";
    std::size_t lineno = 0;
    bool print_version = false;
    bool readonly = false;

    Flags() {}
    Flags(const std::string& fname) : file(fname) {}
    Flags(const std::string& fname, const std::size_t& line) : file(fname), lineno(line) {}
    Flags(const std::string& fname, bool ro) : file(fname), readonly(ro) {}
    Flags(bool version) : print_version(version) {}
};

#endif  // FLAGS_H
