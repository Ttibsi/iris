#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <span>
#include <string>

#include "highlighting/regex_groups.h"

void highlight(Language, std::span<std::string>);
void highlight_line(Language, std::string &);

#endif // HIGHLIGHTER_H
