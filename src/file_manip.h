#ifndef FILE_MANIP_H
#define FILE_MANIP_H

#include <string>
#include <vector>

std::vector<std::string> open_file(std::string file);
bool is_readonly(std::string file);
std::string filename_only(std::string f);
std::string get_file_type(std::string file);

#endif // FILE_MANIP_H
