#ifndef REGEX_GROUPS_H
#define REGEX_GROUPS_H

#include <regex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "highlighting/languages.h"

enum Token {
    FUNC_CALL,
    NUMBER_LITERAL,
    STRING_LITERAL,
    COMMENT,
    KEYWORD,
    TYPE,
};

// TODO: Move this out
static std::unordered_map<Token, std::string> default_theme = {
    {FUNC_CALL,       "#6666ff"},
    { NUMBER_LITERAL, "#ff2222"},
    { STRING_LITERAL, "#22ff22"},
    { COMMENT,        "#666666"},
    { KEYWORD,        "#09D0EF"},
    { TYPE,           "#C4A000"},
};

static std::unordered_map<Language, std::vector<std::pair<Token, std::regex>>>
    highlight_groups = {
        {
         PYTHON, {
                { NUMBER_LITERAL,
                  std::regex("([0-9])") }, // This has to go first or it
                                           // overwrites other escape codes
                // https://dev.to/xowap/the-string-matching-regex-explained-step-by-step-4lkp
                { STRING_LITERAL, std::regex(R"str(("([^"\\]|\\.)*"))str") },
                { KEYWORD,
                  std::regex("\\b(and|as|assert|async|await|break|class|"
                             "continue|def|del|elif|else|except|finally|for|"
                             "from|global|if|import|in|is|lambda|nonlocal|not|"
                             "or|pass|raise|return|try|while|with|yield)\\b") },
                { COMMENT, std::regex("(# *.+)") },
                { FUNC_CALL, std::regex("(\\w+)(?=\\()") },
                { TYPE, std::regex("\\b(str|int|bool|tuple|list|dict|set|Any|"
                                   "Sequence|Union|None)(?![a-zA-Z])") },

            }, }
};

#endif // REGEX_GROUPS_H
