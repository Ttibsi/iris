#ifndef FILE_MANIP_H
#define FILE_MANIP_H

#include "highlighting/languages.h"
#include <string>
#include <vector>

[[nodiscard]] std::vector<std::string> open_file(const std::string &);
[[nodiscard]] bool is_readonly(const std::string &);
[[nodiscard]] std::string filename_only(std::string);
[[nodiscard]] Language get_file_type(const std::string &);
[[nodiscard]] std::size_t write_to_file(const std::string &,
                                        std::vector<std::string>);
[[nodiscard]] std::string get_shebang(const std::string &);

#endif // FILE_MANIP_H
