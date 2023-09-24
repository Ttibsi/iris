#ifndef FILE_MANIP_H
#define FILE_MANIP_H

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "text.h"

std::vector<std::string> open_file(std::string f) {
    std::ifstream ifs(f);
    std::string line;
    std::vector<std::string> lines;

    while (std::getline(ifs, line)) {
        lines.push_back(line);
    }

    filter_for_sensible_whitespace(lines);
    return lines;
}

enum class File_Permission { Directory, Readonly, Writable };

File_Permission check_perms(std::string f) {
    namespace fs = std::filesystem;

    auto status = fs::file_status(fs::status(f));
    if (status.type() == fs::file_type::directory)
        return File_Permission::Directory;

    auto perms = status.permissions();
    if (perms == fs::perms::owner_write)
        return File_Permission::Writable;
    return File_Permission::Readonly;
}

bool is_Readonly(std::string f) {
    return check_perms(f) == File_Permission::Readonly;
}

#endif // FILE_MANIP_H
