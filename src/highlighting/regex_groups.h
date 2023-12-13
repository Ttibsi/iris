#ifndef REGEX_GROUPS_H
#define REGEX_GROUPS_H

#include <regex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "highlighting/languages.h"

enum class Token {
    FUNC_CALL,
    NUMBER_LITERAL,
    STRING_LITERAL,
    COMMENT,
    KEYWORD,
    BOOLEAN,
    TYPE,
};

// TODO: Python: `int_ret` highlights as a type
// TODO: Python: Handle fstrings - don't highlight within {}
// TODO: Python: Handle strings within strings: `"hello 'world'"`
// TODO: Bash
// TODO: C++
// TODO: CMake
// TODO: Rust/Go?
// TODO: Lua?
static std::unordered_map<Language, std::vector<std::pair<Token, std::regex>>>
    highlight_groups = {
        {
         Language::PYTHON,
         {
                { Token::NUMBER_LITERAL,
                  std::regex("([0-9])") }, // This has to go first or it
                                           // overwrites other escape codes
                // https://dev.to/xowap/the-string-matching-regex-explained-step-by-step-4lkp
                { Token::STRING_LITERAL,
                  std::regex(R"str((["']([^"\\]|\\.)*["']))str") },
                { Token::KEYWORD,
                  std::regex("\\b(and|as|assert|async|await|break|class|"
                             "continue|def|del|elif|else|except|finally|for|"
                             "from|global|if|import|in|is|lambda|nonlocal|not|"
                             "or|pass|raise|return|try|while|with|yield)\\b") },
                { Token::COMMENT, std::regex("(# *.+)") },
                { Token::BOOLEAN, std::regex("\\b(True|False)\\b") },
                { Token::FUNC_CALL, std::regex("(\\w+)(?=\\()") },
                { Token::TYPE,
                  std::regex("\\b(str|int|bool|tuple|list|dict|set|Any|"
                             "Sequence|Union|None|List|Dict|Tuple|NamedTuple)(?![a-zA-Z])") },

            }, }
};

#endif // REGEX_GROUPS_H
