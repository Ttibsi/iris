#ifndef TEXT_MANIP_H
#define TEXT_MANIP_H

#include <string>
#include <vector>

void filter_for_sensible_whitespace(std::vector<std::string> &lines);
std::string shell_exec(std::string cmd);

#endif // TEXT_MANIP_H
