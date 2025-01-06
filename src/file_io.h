#ifndef FILE_IO_H
#define FILE_IO_H

#include <optional>
#include <string>
#include <vector>

using lines_t = std::vector<std::string>;
using opt_lines_t = std::optional<std::vector<std::string>>;

[[nodiscard]] opt_lines_t open_file(const std::string&);

#endif // FILE_IO_H
