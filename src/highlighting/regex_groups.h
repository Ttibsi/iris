#ifndef REGEX_GROUPS_H
#define REGEX_GROUPS_H

#include <regex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "highlighting/languages.h"

enum class Token {
    BOOLEAN,
    COMMENT,
    FUNC_CALL,
    KEYWORD,
    NUMBER_LITERAL,
    STRING_LITERAL,
    TYPE,
    VAR_CALL,
};

// TODO: Handle highlighting of `todo` comments
// TODO: Highlight based on shebang
// TODO: Python: Handle fstrings - don't highlight within {}
// TODO: Bash
// TODO: C++
// TODO: Rust/Go?
// TODO: Lua?

static const std::regex number_literal =
    std::regex("(?:v|\\b)(([0-9]+|(\\.[0-9])+)+)");

static std::unordered_map<Language, std::vector<std::pair<Token, std::regex>>>
    highlight_groups = {
        { Language::PYTHON,
         {
              // This has to go first or it overwrites other escape codes
              { Token::NUMBER_LITERAL, number_literal },
              // https://dev.to/xowap/the-string-matching-regex-explained-step-by-step-4lkp
              { Token::STRING_LITERAL,
                std::regex(R"str(('([^'\\]|\\.)*'))str") },
              { Token::STRING_LITERAL,
                std::regex(R"str(("([^"\\]|\\.)*"))str") },
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
                           "Sequence|Union|None|List|Dict|Tuple|NamedTuple)(?!["
                           "a-zA-Z])\\b") },

          } },
        {
         Language::CMAKE,
         {
                // This has to go first or it overwrites other escape codes
                { Token::NUMBER_LITERAL, number_literal },
                // https://dev.to/xowap/the-string-matching-regex-explained-step-by-step-4lkp
                { Token::STRING_LITERAL,
                  std::regex(R"str((["']([^"\\]|\\.)*["']))str") },
                { Token::COMMENT, std::regex("(# *.+)") },
                { Token::VAR_CALL,
                  std::regex("(\\$\\{[^CMAKE|cmake][a-zA-Z_]+\\})") },
                { Token::KEYWORD,
                  std::regex("((?:CMAKE|cmake)_[a-zA-Z_]+(?!\\())\\b") },
                { Token::FUNC_CALL, std::regex("(\\w+)(?=\\()") },
                { Token::KEYWORD,
                  std::regex("\\b(COPY|NOT|COMMAND|PROPERTY|POLICY|TARGET|"
                             "EXISTS|IS_(DIRECTORY|ABSOLUTE)|DEFINED|OR|AND|IS_"
                             "NEWER_THAN|MATCHES|(STR|VERSION_)?(LESS|GREATER|"
                             "EQUAL|ON|OFF|CXX|SET|VERSION))\\b") },

            }, }
};

#endif // REGEX_GROUPS_H
