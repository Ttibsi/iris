#include "text_io.h"

#include <array>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>

#include <rawterm/text.h>
#include <sys/stat.h>
#include <unistd.h>

#include "constants.h"
#include "spdlog/spdlog.h"
#include "view.h"

[[nodiscard]] opt_lines_t open_file(const std::string& file) {
    if (!get_file_size(file)) {
        return {};
    }

    auto ret = std::vector<std::string>();
    std::string line = "";
    char ch;

    std::ifstream ifs(file);
    if (ifs.fail()) {
        return {};
    }

    while (ifs.get(ch)) {
        switch (ch) {
            case '\r':
                break;
            case '\t':
                line += std::string(TAB_SIZE, ' ');
                break;
            case '\n':
                ret.push_back(line);
                line = "";
                break;
            default:
                line.push_back(ch);
        };
    }

    // If there's no newlines in the stream at all, it never gets added to
    // the vector
    if (line.size()) {
        ret.push_back(line);
    }

    return ret;
}

[[nodiscard]] unsigned int get_file_size(const std::string& file) {
    namespace fs = std::filesystem;
    fs::path p(file);

    try {
        return uint_t(fs::file_size(p));
    } catch (const fs::filesystem_error&) {
        return 0;
    }
}

[[nodiscard]] WriteData write_to_file(Model* model) {
    if (model->filename == "") {
        return WriteData();
    }
    std::ofstream out(model->filename);
    for (auto&& line : model->buf) {
        out << line << "\n";
    }

    model->unsaved = false;
    return WriteData(static_cast<int>(out.tellp()), int32_t(model->buf.size()));
}

[[nodiscard]] lines_t lines(const std::string& str) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string line;

    while (std::getline(ss, line)) {
        if (line.back() == '\r') {
            line.pop_back();
        }
        result.push_back(rawterm::raw_str(line));
    }

    return result;
}

[[nodiscard]] bool is_letter(const char& c) {
    std::array<char, 52> alphabet = {
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
        's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
        'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};

    return std::find(alphabet.begin(), alphabet.end(), c) != alphabet.end();
}

[[nodiscard]] std::string check_filename(const std::string& filename) {
    std::string err_text = "Iris currently does not support tab-delineated files";

    if (filename == "Makefile" || filename == "makefile") {
        return err_text;
    } else if (filename.ends_with(".go")) {
        return err_text;
    }

    return "";
}

// https://stackoverflow.com/a/12774387
[[nodiscard]] bool file_exists(std::string_view name) {
    struct stat buffer;
    return (stat(name.data(), &buffer) == 0);
}

// https://www.rozmichelle.com/pipes-forks-dups/
// https://markbailey34.medium.com/fork-exec-and-pipe-ls-in-c-16dde5dc179a
[[nodiscard]] std::optional<Response> shell_exec(std::string cmd) {
    // Convert to a C interface (`char* array`)
    std::vector<char*> cstrs;
    cstrs.push_back(const_cast<char*>("sh"));
    cstrs.push_back(const_cast<char*>("-c"));
    cstrs.push_back(const_cast<char*>(cmd.c_str()));
    cstrs.push_back(nullptr);

    // Setup file descriptors and fork
    int fds[2];
    pipe(fds);

    // to get stderr, we need to pipe twice
    int err_fds[2];
    pipe(err_fds);

    pid_t pid = fork();

    if (pid == 0) {
        // child process
        close(fds[0]);                // close read
        dup2(fds[1], STDOUT_FILENO);  // set stdout to write

        close(err_fds[0]);
        dup2(err_fds[1], STDERR_FILENO);  // get stderr?

        close(fds[1]);  // we are done modifying it so close it
        close(err_fds[1]);

        execv("/bin/sh", cstrs.data());
        _exit(1);  // Kill the child process if it fails

    } else if (pid < 0) {
        // error
        close(fds[0]);
        close(fds[1]);
        close(err_fds[0]);
        close(err_fds[1]);
        return {};

    } else {
        // parent process
        close(fds[1]);
        close(err_fds[1]);

        char buf[1024];
        char err[1024];
        long bytes_read = 0;
        Response resp;

        while ((bytes_read = read(fds[0], buf, sizeof(buf))) > 0) {
            resp.out.append(buf, std::size_t(bytes_read));
        }

        bytes_read = 0;
        while ((bytes_read = read(err_fds[0], err, sizeof(err))) > 0) {
            resp.err.append(err, std::size_t(bytes_read));
        }

        auto logger = spdlog::get("basic_logger");
        if (logger != nullptr) {
            logger->info("Shell cmd run: `" + cmd + "`");
        }

        if (!resp.out.empty() && resp.out.at(resp.out.size() - 1) == '\n') {
            resp.out = resp.out.substr(0, resp.out.size() - 1);
        }

        return resp;
    }
}

[[nodiscard]] std::vector<std::string> split_by(const std::string& text, const char delim) {
    std::vector<std::string> result;
    std::stringstream ss(text);
    std::string item;

    while (getline(ss, item, delim)) {
        result.push_back(item);
    }

    return result;
}
