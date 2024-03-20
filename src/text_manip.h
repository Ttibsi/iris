#ifndef TEXT_MANIP_H
#define TEXT_MANIP_H

#include <optional>
#include <span>
#include <string>
#include <vector>

#include "rawterm/rawterm.h"

struct Response {
    std::string stdout;
    std::string stderr;
    int retcode;

    bool operator==(const Response &rhs) const = default;
};

[[nodiscard]] std::vector<std::string>
    filter_whitespace(std::vector<std::string>);
[[nodiscard]] std::string filter_whitespace(std::string);
[[nodiscard]] Response shell_exec(const std::string &, bool);
[[nodiscard]] std::size_t line_size(const std::string &);
[[nodiscard]] std::size_t count_char(const std::string &, char);
[[nodiscard]] int find_next_whitespace(const std::string &,
                                       const std::size_t &);
[[nodiscard]] int find_prev_whitespace(const std::string &,
                                       const std::size_t &);
[[nodiscard]] bool is_numeric(const std::string &);
[[nodiscard]] std::optional<rawterm::Pos> find_in_text(std::span<std::string>,
                                                       std::string);
void replace_in_text(std::string &line, int pos, const std::string &new_text);
void join_lines(std::vector<std::string> &, unsigned int);
[[nodiscard]] std::string parse_shebang(std::string line);

#endif // TEXT_MANIP_H
