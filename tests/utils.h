#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <sstream>
#include <string>
#include <vector>

inline std::vector<std::string> lines(const std::string &str) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string line;

    while (std::getline(ss, line)) {
        result.push_back(line);
    }

    return result;
}

#endif // TEST_UTILS_H
