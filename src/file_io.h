#ifndef FILE_IO_H
#define FILE_IO_H

#include <optional>
#include <string>

#include "twin_array.h"

[[nodiscard]] std::optional<TwinArray<char>> open_file(const std::string &);

#endif // FILE_IO_H
