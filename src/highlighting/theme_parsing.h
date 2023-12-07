#ifndef THEME_PARSING_H
#define THEME_PARSING_H

#include <string>
#include <unordered_map>

#include "constants.h"
#include "highlighting/regex_groups.h"

inline std::unordered_map<Token, std::string> get_theme() {
    if (THEME == "default") {
        return {
            {FUNC_CALL,       "#6666ff"},
            { NUMBER_LITERAL, "#ff2222"},
            { STRING_LITERAL, "#22ff22"},
            { COMMENT,        "#666666"},
            { KEYWORD,        "#09D0EF"},
            { TYPE,           "#C4A000"},
            { BOOLEAN,        "#C4A000"},
        };
    } else {
        // TODO:
    }
}

#endif // THEME_PARSING_H
