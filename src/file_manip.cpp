#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "file_manip.h"
#include "text_manip.h"

std::vector<std::string> open_file(const std::string &file) {
    std::ifstream ifs(file);
    std::string line;
    std::vector<std::string> lines;

    while (std::getline(ifs, line)) {
        lines.push_back(line);
    }

    return lines;
}

enum class File_Permission { Directory, Readonly, Writable };

File_Permission check_perms(const std::string &file) {
    namespace fs = std::filesystem;

    auto status = fs::file_status(fs::status(file));
    if (status.type() == fs::file_type::directory)
        return File_Permission::Directory;

    auto perms = fs::status(file).permissions();
    if ((perms & fs::perms::owner_write) != fs::perms::none)
        return File_Permission::Writable;
    return File_Permission::Readonly;
}

bool is_readonly(const std::string &file) {
    return check_perms(file) == File_Permission::Readonly;
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
