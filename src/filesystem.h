#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <optional>
#include <string>
#include <vector>

struct Response {
    std::string stdout;
    std::string stderr;
    int retcode;

    [[nodiscard]] constexpr bool operator==(const Response& rhs) const {
        return stdout == rhs.stdout && stderr == rhs.stderr && retcode == rhs.retcode;
    }
};

[[nodiscard]] std::optional<std::vector<char>> open_file(const std::string&);
[[nodiscard]] std::optional<Response> shell_exec(const std::string&, bool);

#endif  // FILESYSTEM_H
