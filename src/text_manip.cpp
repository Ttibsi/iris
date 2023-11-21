#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "constants.h"
#include "file_manip.h"
#include "text_manip.h"

std::vector<std::string> filter_whitespace(std::vector<std::string> lines) {
    std::unordered_map<char, std::string> pairs = {
        {'\t', std::string(TABSTOP, ' ')},
    };

    for (auto &line : lines) {
        for (std::pair<char, std::string> p : pairs) {
            // if (line.find(p.first) != std::string::npos) {
            if (std::binary_search(line.begin(), line.end(), p.first)) {
                line.replace(line.find(p.first), 1, p.second);
            }
        }
        line += "\r\n";
    }

    return lines;
}

std::string shell_exec(const std::string &cmd, bool output) {
    namespace fs = std::filesystem;
    std::string tmp_dir = fs::temp_directory_path();
    int retcode = std::system((cmd + ">" + tmp_dir + "/iris_cmd_out.txt 2> " +
                               tmp_dir + "/iris_cmd_err.txt")
                                  .c_str());

    if (output) {
        std::vector<std::string> stdout_contents =
            open_file(tmp_dir + "/iris_cmd_out.txt");

        if (retcode) {
            std::vector<std::string> stderr_contents =
                open_file(tmp_dir + "/iris_cmd_err.txt");
            std::string ret = "";
            for (auto &l : stderr_contents) {
                ret += l;
            }
            return ret;
        }

        std::string ret = "";
        for (auto &l : stdout_contents) {
            ret += l;
        }
        return ret;

    } else {
        return "";
    }
}

std::size_t count_char(const std::string &line, char c) {
    std::size_t ret = 0;
    for (const char &l : line) {
        if (l == c)
            ret++;
    }
    return ret;
}

std::size_t line_size(const std::string &line) {
    return line.size() + count_char(line, '\t') * TABSTOP;
}

int find_next_whitespace(const std::string &curr_line,
                         const std::size_t &curr_pos) {
    // curr_poss is 0-indexed - the string index
    int ret = 0;

    for (std::size_t i = curr_pos + 1; i <= curr_line.size(); i++) {
        if (std::isspace(curr_line[i])) {
            ret += i;
            return ret;
        }
    }

    return curr_line.size();
}

int find_prev_whitespace(const std::string &curr_line,
                         const std::size_t &curr_pos) {
    // curr_poss is 0-indexed - the string index
    int ret = 0;

    for (std::size_t i = curr_pos; i > 0; i--) {
        if (std::isspace(curr_line[i])) {
            ret += i;
            return ret;
        }
    }

    return 1;
}
