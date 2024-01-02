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
    COMMENT_TODO,
    COMMON_COMMANDS,
    FLAGS,
    FUNC_CALL,
    KEYWORD,
    NUMBER_LITERAL,
    STRING_LITERAL,
    TYPE,
    VAR_CALL,
};

// TODO: Highlight based on shebang
// TODO: Highlight trailing whitespace
// TODO: Python: Handle fstrings - don't highlight within {}
// TODO: C++
// TODO: Rust/Go?
// TODO: Lua?

const std::regex number_literal =
    std::regex("(?:[vV]|\\b)(([0-9]+|(\\.[0-9])+)+)");

using hl_group_t =
    std::unordered_map<Language, std::vector<std::pair<Token, std::regex>>>;
inline hl_group_t highlight_groups = {
    {Language::BASH,
     {
          { Token::NUMBER_LITERAL, number_literal },
          { Token::COMMENT, std::regex("^(# *[^ TODO:].+)") },
          { Token::COMMENT_TODO, std::regex("(# TODO: +.+)") },
          { Token::BOOLEAN, std::regex("\\b(true|false)\\b") },
          { Token::FUNC_CALL, std::regex("^[A-Za-z0-9_-]+\\(\\)") },
          { Token::STRING_LITERAL, std::regex(R"str(('([^'\\]|\\.)*'))str") },
          { Token::STRING_LITERAL, std::regex(R"str(("([^"\\]|\\.)*"))str") },
          { Token::FLAGS, std::regex("( -[A-Za-z]+| --[A-Za-z-]+)") },
          { Token::KEYWORD,
            std::regex("\\b(break|case|continue|do|done|elif|else|esac|exit|fi|"
                       "for|function|if|in|read|return|select|shift|then|time|"
                       "until|while|declare|eval|exec|export|let|local)\\b") },
          { Token::KEYWORD, std::regex("\\b-(eq|ne|gt|lt|ge|le|ef|ot|nt)\\b") },
          { Token::VAR_CALL,
            std::regex("(\\$[a-zA-Z-_0-9]+|\\$\\([a-zA-Z-_0-9]+\\))") },
          { Token::COMMON_COMMANDS,
            std::regex(
                "\\b(awk|cat|cd|ch(grp|mod|own)|cp|cut|echo|env|grep|head|"
                "install|ln|make|mkdir|mv|popd|printf|pushd|rm|rmdir|sed|set|"
                "sort|sudo|tail|tar|touch|umask|unset)\\b") },

      }},
    { Language::CMAKE,
     {
          // This has to go first or it overwrites other escape codes
          { Token::NUMBER_LITERAL, number_literal },
          // https://dev.to/xowap/the-string-matching-regex-explained-step-by-step-4lkp
          { Token::STRING_LITERAL,
            std::regex(R"str((["']([^"\\]|\\.)*["']))str") },
          { Token::COMMENT, std::regex("^(# *[^ TODO:].+)") },
          { Token::COMMENT_TODO, std::regex("(# TODO: +.+)") },
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

      }},
    { Language::PYTHON,
     {
          // This has to go first or it overwrites other escape codes
          { Token::NUMBER_LITERAL, number_literal },
          // https://dev.to/xowap/the-string-matching-regex-explained-step-by-step-4lkp
          { Token::STRING_LITERAL, std::regex(R"str(('([^'\\]|\\.)*'))str") },
          { Token::STRING_LITERAL, std::regex(R"str(("([^"\\]|\\.)*"))str") },
          { Token::KEYWORD,
            std::regex("\\b(and|as|assert|async|await|break|class|"
                       "continue|def|del|elif|else|except|finally|for|"
                       "from|global|if|import|in|is|lambda|nonlocal|not|"
                       "or|pass|raise|return|try|while|with|yield)\\b") },
          { Token::COMMENT_TODO, std::regex("(# TODO: +(?:.+))") },
          { Token::COMMENT, std::regex("(# ?.+)") },
          { Token::BOOLEAN, std::regex("\\b(True|False)\\b") },
          { Token::FUNC_CALL, std::regex("(\\w+)(?=\\()") },
          { Token::TYPE,
            std::regex("\\b(str|int|bool|tuple|list|dict|set|Any|"
                       "Sequence|Union|None|List|Dict|Tuple|NamedTuple)(?!["
                       "a-zA-Z])\\b") },

      }}
};

#endif // REGEX_GROUPS_H
