#include "filesystem.h"

#include <cstring>
#include <fstream>
#include <stdexcept>

[[nodiscard]] std::optional<std::vector<char>> open_file(const std::string& file) {
    std::vector<char> ret;
    ret.reserve(1024);
    char ch;

    std::ifstream ifs(file);
    if (ifs.fail()) {
        throw std::runtime_error(strerror(errno));
    }

    while (ifs.get(ch)) {
        ret.push_back(ch);
    }
    return ret;
}
