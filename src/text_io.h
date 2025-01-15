#ifndef TEXT_IO_H
#define TEXT_IO_H

#include <optional>
#include <string>
#include <vector>

using lines_t = std::vector<std::string>;
using opt_lines_t = std::optional<std::vector<std::string>>;

[[nodiscard]] opt_lines_t open_file(const std::string&);
[[nodiscard]] lines_t lines(const std::string& str);

#endif  // TEXT_IO_H
