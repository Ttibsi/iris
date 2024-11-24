#ifndef FILE_IO_H
#define FILE_IO_H

#include <optional>
#include <string>

#include "gapbuffer.h"

struct Response {
    std::string stdout;
    std::string stderr;
    int retcode;

    [[nodiscard]] constexpr bool operator==(const Response& rhs) const {
        return stdout == rhs.stdout && stderr == rhs.stderr && retcode == rhs.retcode;
    }
};

[[nodiscard]] std::optional<Gapbuffer> open_file(const std::string&);
// [[nodiscard]] std::optional<Response> shell_exec(const std::string&, bool);
[[nodiscard]] std::size_t write_to_file(const std::string&, Gapbuffer);

#endif  // FILE_IO_H
