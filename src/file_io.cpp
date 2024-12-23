#include <fstream>

#include "file_io.h"

[[nodiscard]] std::optional<TwinArray<char>>
open_file(const std::string &file) {
    auto ret = TwinArray<char>(1024);
    char ch;

    std::ifstream ifs(file);
    if (ifs.fail()) {
        return {};
    }

    while (ifs.get(ch)) {
        if (ch == '\n') {
            ret.push('\r');
        }
        ret.push(ch);
    }

    // Remove trailing \r\n
    if (ret.peek() == '\n') {
        ret.pop();
        ret.pop();
    }

    for (unsigned int i = 0; i <= ret.size(); i++) {
        ret.move_left();
    }

    return ret;
}
