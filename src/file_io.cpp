#include "file_io.h"

#include <fstream>

#include "constants.h"

[[nodiscard]] opt_lines_t open_file(const std::string& file) {
    auto ret = std::vector<std::string>(1024);
    std::string line = "";
    char ch;

    std::ifstream ifs(file);
    if (ifs.fail()) {
        return {};
    }

    while (ifs.get(ch)) {
        switch (ch) {
            case '\r':
                break;
            case '\t':
                line += std::string(TAB_SIZE, ' ');
                break;
            case '\n':
                ret.push_back(line);
                line = "";
                break;
            default:
                line.push_back(ch);
        };
    }

    return ret;
}
