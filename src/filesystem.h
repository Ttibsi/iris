#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <optional>
#include <string>
#include <vector>

[[nodiscard]] std::optional<std::vector<char>> open_file(const std::string&);

#endif  // FILESYSTEM_H
