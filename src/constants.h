#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

#include <rawterm/color.h>

const bool LINE_NUMBERS = true;
const int TAB_SIZE = 4;
const std::size_t LINE_BORDER = 100;

const rawterm::Color COLOR_UI_BG = rawterm::Colors::gray;
const rawterm::Color COLOR_DARK_YELLOW = rawterm::Color("#FFdd33");
// Should match ansi color code 92
const rawterm::Color COLOR_GREEN = rawterm::Color("#5ffa68");

const std::string COLOR_ALERT = "\x1b[41m\x1b[37m";
const std::string WHITESPACE = " \t\n\r\f\v";

#endif  // CONSTANTS_H
