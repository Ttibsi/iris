#ifndef TEXT_MANIP_H
#define TEXT_MANIP_H

#include <string>
#include <vector>

std::vector<std::string> filter_whitespace(std::vector<std::string>);
std::string shell_exec(const std::string &);
std::size_t line_size(const std::string &);
std::size_t count_char(std::string line, char c);
int find_next_whitespace(const std::string &, const std::size_t &);
int find_prev_whitespace(const std::string &, const std::size_t &);

#endif // TEXT_MANIP_H
