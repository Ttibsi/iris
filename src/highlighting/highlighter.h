#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <span>
#include <string>
#include <string_view>

#include "highlighting/languages.h"

[[nodiscard]] const std::string parse_colour(std::string) noexcept;
void highlight(const Language &, std::span<std::string>);
void highlight_line(const Language &, std::string &);

#endif // HIGHLIGHTER_H
