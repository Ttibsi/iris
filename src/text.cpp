#include <string>
#include <unordered_map>
#include <vector>

#include "constants.h"

void filter_for_sensible_whitespace(std::vector<std::string> &lines) {
    std::unordered_map<std::string, std::string> pairs = {
        { "\t", std::string(TABSTOP, ' ') }
    };

    for (auto line : lines) {
        for (std::pair<std::string, std::string> p : pairs) {
            if (line.find(p.first) != std::string::npos) {
                line.replace(line.find(p.first), 1, p.second);
            }
        }
        line + "\r\n";
    }
}
