#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string.h>
#include <string>
#include <unistd.h>
#include <vector>

#include "file_manip.h"
#include "text_manip.h"

[[nodiscard]] std::vector<std::string> open_file(const std::string &file) {
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

[[nodiscard]] bool is_readonly(const std::string &file) {
    if (access(file.c_str(), W_OK) == -1) {
        return true;
    }
    return false;
}

[[nodiscard]] std::string filename_only(std::string f) {
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

[[nodiscard]] std::string get_shebang(const std::string &filename) {
    std::ifstream ifs(filename);
    std::string line;
    std::getline(ifs, line);

    if (line.at(0) == '#' && line.at(1) == '!') {
        return line;
    }
    return "";
}

[[nodiscard]] Language get_file_type(const std::string &file) {
    namespace fs = std::filesystem;
    auto path = fs::path(file);

    if (file == "NO FILE") {
        return Language::TEXT;
    } else if (!(path.extension().empty())) {
        if (filename_only(file) == "CMakeLists.txt" ||
            path.extension() == "cmake")
            return Language::CMAKE;

        std::string path_str = path.extension();
        return languages[path_str.substr(1, path_str.size())];
    } else {
        std::string lang = parse_shebang(get_shebang(file));
        if (!(lang.empty())) {
            return languages[lang];
        }
        return Language::UNKNOWN;
    }
}

[[nodiscard]] std::size_t write_to_file(const std::string &file,
                                        std::vector<std::string> lines) {
    if (file == "NO FILE") {
        return -1;
    }
    std::ofstream out(file);

    for (auto &&line : lines) {
        line.erase(
            std::remove_if(line.begin(), line.end(),
                           [](char x) { return x == '\r' || x == '\n'; }),
            line.end());

        out << line << "\n";
    }

    return out.tellp();
}
