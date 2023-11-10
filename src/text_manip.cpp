#include <algorithm>
#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "constants.h"
#include "text_manip.h"

std::vector<std::string> filter_whitespace(std::vector<std::string> lines) {
    std::unordered_map<std::string, std::string> pairs = {
        {"\t", std::string(TABSTOP, ' ')}
    };

    for (auto &line : lines) {
        for (std::pair<std::string, std::string> p : pairs) {
            if (line.find(p.first) != std::string::npos) {
                line.replace(line.find(p.first), 1, p.second);
            }
        }
        line += "\r\n";
    }

    return lines;
}

// https://stackoverflow.com/a/478960
std::string shell_exec(const std::string &cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"),
                                                  pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    std::erase_if(result, [](auto ch) { return (ch == '\n' || ch == '\r'); });

    return result;
}

std::size_t line_size(const std::string &line) {
    return line.size() + std::ranges::count(line, '\t') * TABSTOP;
}
