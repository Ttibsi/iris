#ifndef TEXT_IO_H
#define TEXT_IO_H

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "model.h"

using lines_t = std::vector<std::string>;
using opt_lines_t = std::optional<std::vector<std::string>>;

struct WriteData {
    int bytes = 0;
    int lines = 0;
    bool valid = false;

    WriteData() {}
    WriteData(int file_bytes, int file_lines) : bytes(file_bytes), lines(file_lines) {
        valid = true;
    }
};

struct WriteAllData {
    int files = 0;
    bool valid = false;

    WriteAllData() {}
    WriteAllData(int file_count, bool is_valid) : files(file_count), valid(is_valid) {}
};

struct Response {
    std::string out = "";
    std::string err = "";
};

[[nodiscard]] opt_lines_t open_file(const std::string&);
[[nodiscard]] unsigned int get_file_size(const std::string&);
[[nodiscard]] WriteData write_to_file(Model*);
[[nodiscard]] lines_t lines(const std::string&);
[[nodiscard]] bool is_letter(const char&);
[[nodiscard]] std::string check_filename(const std::string&);
[[nodiscard]] bool file_exists(std::string_view);
[[nodiscard]] std::optional<Response> shell_exec(std::string);
[[nodiscard]] std::vector<std::string> split_by(const std::string&, const char);

#endif  // TEXT_IO_H
