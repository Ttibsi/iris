#include "text_io.h"

#include <fstream>
#include <sstream>

#include <rawterm/text.h>

#include "constants.h"

[[nodiscard]] opt_lines_t open_file(const std::string& file) {
    auto ret = std::vector<std::string>();
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

[[nodiscard]] std::size_t write_to_file(const Model& model) {
    if (model.filename == "") {
        return -1;
    }
    std::ofstream out(model.filename);
    for (auto&& line : model.buf) {
        out << line << "\n";
    }
    return out.tellp();
}

[[nodiscard]] lines_t lines(const std::string& str) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string line;

    while (std::getline(ss, line)) {
        if (line.back() == '\r') {
            line.pop_back();
        }
        result.push_back(rawterm::raw_str(line));
    }

    return result;
}