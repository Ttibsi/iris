#ifndef TEXT_TRANSFORM_H
#define TEXT_TRANSFORM_H

#include <string>

[[nodiscard]] std::string strip_trailing_whitespace(std::string);
[[nodiscard]] std::string strip_newline(std::string);

#endif  // TEXT_TRANSFORM_H
