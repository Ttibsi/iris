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

std::vector<std::string> filter_whitespace(std::vector<std::string>);
std::string filter_whitespace(std::string);
Response shell_exec(const std::string &, bool);
std::size_t line_size(const std::string &);
std::size_t count_char(const std::string &, char);
int find_next_whitespace(const std::string &, const std::size_t &);
int find_prev_whitespace(const std::string &, const std::size_t &);
bool is_numeric(const std::string &);
std::optional<rawterm::Pos> find_in_text(std::span<std::string>, std::string);
void replace_in_text(std::string &line, int pos, const std::string &new_text);
void join_lines(std::vector<std::string> &, unsigned int);

#endif // TEXT_MANIP_H
