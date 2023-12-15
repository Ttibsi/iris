#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <span>
#include <string>

#include "highlighting/languages.h"

std::string parse_colour(std::string);
void highlight(Language, std::span<std::string>);
void highlight_line(Language, std::string &);

#endif // HIGHLIGHTER_H
