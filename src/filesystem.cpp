#include "filesystem.h"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <stdexcept>

[[nodiscard]] std::optional<std::vector<char>> open_file(const std::string& file) {
    std::vector<char> ret;
    ret.reserve(1024);
    char ch;

    std::ifstream ifs(file);
    if (ifs.fail()) {
        // throw std::runtime_error(strerror(errno));
        return {};
    }

    while (ifs.get(ch)) {
        if (ch == '\n') {
            ret.push_back('\r');
        }
        ret.push_back(ch);
    }
    return ret;
}

[[nodiscard]] std::optional<Response> shell_exec(const std::string& cmd, bool output) {
    namespace fs = std::filesystem;
    std::string tmp_dir = fs::temp_directory_path();
    int retcode = std::system(
        (cmd + ">" + tmp_dir + "/iris_cmd_out.txt 2> " + tmp_dir + "/iris_cmd_err.txt").c_str());

    if (output) {
        auto stdout_contents = open_file(tmp_dir + "/iris_cmd_out.txt");

        if (retcode) {
            auto stderr_contents = open_file(tmp_dir + "/iris_cmd_err.txt");
            std::string ret = "";
            for (auto& l : stderr_contents.value()) {
                ret += l;
            }
            return Response {"", ret, retcode};
        }

        std::string ret = "";
        for (auto& l : stdout_contents.value()) {
            ret += l;
        }

        return Response {ret, "", 0};

    } else {
        return {};
    }
}
