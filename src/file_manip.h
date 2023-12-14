#ifndef FILE_MANIP_H
#define FILE_MANIP_H

#include "highlighting/languages.h"
#include <string>
#include <vector>

std::vector<std::string> open_file(const std::string &);
bool is_readonly(const std::string &);
std::string filename_only(std::string);
Language get_file_type(const std::string &);
std::size_t write_to_file(const std::string &, std::vector<std::string>);

#endif // FILE_MANIP_H
