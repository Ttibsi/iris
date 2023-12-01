#include <regex>

#include "highlighter.h"

std::string parse_colour(std::string raw) {
    std::string ret = "";

    int placeholder;
    placeholder = std::stoi(raw.substr(1,2), 0, 16);
    ret += std::to_string(placeholder) + ";";

    placeholder = std::stoi(raw.substr(3,2), 0, 16);
    ret += std::to_string(placeholder) + ";";

    placeholder = std::stoi(raw.substr(5,2), 0, 16);
    ret += std::to_string(placeholder) + "m";

    return ret;
}

void highlight(Language language, std::span<std::string> lines) {
    for (unsigned int i = 0; i < lines.size(); i++) {
        if (lines[i].empty()) { continue; }
        highlight_line(language, lines[i]);
    }
}

void highlight_line(Language language, std::string& line) {
    const std::string close_suffix = "[0m";

    for (auto&& re : highlight_groups[language]) {
        std::smatch match;
        std::regex_search(line, match, re.second);

        // TODO: Make sure this reads from the `THEME` constant instead of
        // hardcoding the default here. I want to make the default theme 
        // its own file
        std::string ansi_colour = "\x1b[38;2;" + parse_colour(default_theme[re.first]);
        std::string result_text = ansi_colour + "$1\x1B[0m";
        line = std::regex_replace(line, re.second, result_text);
    }

    // Check the line hasn't got nested highlighting
    int open_count = 0;
    for (unsigned int j = 0; j < line.size(); j++) {
        if (static_cast<int>(line.at(j)) == 27) { // 27 = \x1b
            // close 
            if (line.substr(j + 1, close_suffix.size()) == close_suffix) {
                if (open_count > 1) {
                    line.replace(j, line.substr(j, line.size()).find("m") + 1, "");
                }
                open_count--;
            } else {
                open_count++;
                if (open_count > 1) {
                    line.replace(j, line.substr(j, line.size()).find("m") + 1, "");
                }
            }
        }
    }
}