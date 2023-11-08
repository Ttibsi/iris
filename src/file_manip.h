#ifndef FILE_MANIP_H
#define FILE_MANIP_H

#include <string>
#include <vector>

std::vector<std::string> open_file(const std::string &);
bool is_readonly(const std::string &);
std::string filename_only(std::string);
std::string get_file_type(const std::string &);

#endif // FILE_MANIP_H
