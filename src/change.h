#ifndef CHANGE_H
#define CHANGE_H

#include <optional>
#include <string>

using uint_t = unsigned int;

// Forward declare to prevent circular header includes
enum class ActionType;

struct Change {
    const ActionType action;
    const uint_t line_pos;
    const uint_t char_pos;
    std::optional<char> payload;
    std::optional<const std::string> text;

    Change(const ActionType a, const uint_t ln, const uint_t ch)
        : action(a), line_pos(ln), char_pos(ch) {}
    Change(const ActionType a, const uint_t ln, const uint_t ch, const char c)
        : action(a), line_pos(ln), char_pos(ch), payload(c) {}
    Change(const ActionType a, const uint_t ln, const uint_t ch, const std::string str)
        : action(a), line_pos(ln), char_pos(ch), text(str) {}
};

#endif  // CHANGE_H
