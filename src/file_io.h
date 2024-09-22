#ifndef FILE_IO_H
#define FILE_IO_H

#include <optional>
#include <string>
#include <vector>

#include "gapvector.h"

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
[[nodiscard]] std::size_t write_to_file(const std::string&, Gapvector<>);

#endif  // FILE_IO_H
