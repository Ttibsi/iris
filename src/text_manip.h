#ifndef TEXT_MANIP_H
#define TEXT_MANIP_H

#include <string>
#include <vector>

void filter_for_sensible_whitespace(std::vector<std::string> &);
std::string shell_exec(const std::string &);

#endif // TEXT_MANIP_H
