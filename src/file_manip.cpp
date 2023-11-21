#include <filesystem>
#include <fstream>
#include <string.h>
#include <string>
#include <unistd.h>
#include <vector>

#include "file_manip.h"

std::vector<std::string> open_file(const std::string &file) {
    std::ifstream ifs(file);
    std::string line;
    std::vector<std::string> lines;

    // empty file
    if (ifs.peek() == std::ifstream::traits_type::eof()) {
        return { "" };
    }

    while (std::getline(ifs, line)) {
        lines.push_back(line);
    }

    return lines;
}

bool is_readonly(const std::string &file) {
    if (access(file.c_str(), W_OK) == -1) {
        return true;
    }
    return false;
}

std::string filename_only(std::string f) {
    // TODO: Windows
    std::string delim = "/";

    size_t pos = 0;
    std::string token;
    while ((pos = f.find(delim)) != std::string::npos) {
        token = f.substr(0, pos);
        f.erase(0, pos + delim.length());
    }

    return f;
}

std::string get_file_type(const std::string &file) {
    namespace fs = std::filesystem;
    auto path = fs::path(file);

    if (file == "NO FILE") {
        return "TEXT";
    } else if (!(path.extension().empty())) {
        return path.extension();
    } else {
        return "UNKNOWN";
    }
}

std::size_t write_to_file(const std::string &file,
                          const std::vector<std::string> &lines) {
    if (file == "NO FILE") {
        return -1;
    }
    std::ofstream out(file);

    for (auto &&line : lines) {
        out << line << "\n";
    }

    return out.tellp();
}
