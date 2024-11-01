#include "file_io.h"

#include <cstring>
#include <filesystem>
#include <fstream>

[[nodiscard]] std::optional<Gapvector<>> open_file(const std::string& file) {
    auto ret = Gapvector(1024);
    char ch;

    std::ifstream ifs(file);
    if (ifs.fail()) {
        return {};
    }

    while (ifs.get(ch)) {
        if (ch == '\n') {
            ret.push_back('\r');
        }
        ret.push_back(ch);
    }

    // Remove trailing \r\n
    if (ret.back() == '\n') {
        ret.pop_back();
        ret.pop_back();
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
            if (stderr_contents.has_value()) {
                return Response {"", stderr_contents.value().to_str(), retcode};
            }
            return Response {"", "No STDERR contents found", retcode};
        }

        return Response {stdout_contents.value().to_str(), "", 0};

    } else {
        return {};
    }
}

[[nodiscard]] std::size_t write_to_file(const std::string& file, Gapvector<> chars) {
    if (file == "NO FILE") {
        return -1;
    }
    std::ofstream out(file);

    for (auto&& c : chars) {
        if (c == '\r') {
            continue;
        }
        out << c;
    }

    return out.tellp();
}
