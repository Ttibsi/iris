#ifndef THEME_PARSING_H
#define THEME_PARSING_H

#include <string>
#include <unordered_map>

#include "constants.h"
#include "highlighting/regex_groups.h"

inline std::unordered_map<Token, std::string> get_theme() {
    if (THEME == "default") {
        return {
            {Token::FUNC_CALL,       "#6666ff"},
            { Token::NUMBER_LITERAL, "#ff2222"},
            { Token::STRING_LITERAL, "#22ff22"},
            { Token::COMMENT,        "#666666"},
            { Token::KEYWORD,        "#09D0EF"},
            { Token::TYPE,           "#C4A000"},
            { Token::BOOLEAN,        "#C4A000"},
            { Token::VAR_CALL,       "#FFEB00"},
        };
    } else {
        // TODO:
        return {};
    }
}

#endif // THEME_PARSING_H
