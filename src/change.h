#ifndef CHANGE_H
#define CHANGE_H

#include <optional>

// Forward declare to prevent circular header includes
enum class ActionType;

struct Change {
    const ActionType action;
    char payload;
    const std::optional<unsigned int> line_pos;
    const std::optional<unsigned int> char_pos;

    Change(ActionType a) : action(a), payload('\0') {}
    Change(ActionType a, unsigned int ln, unsigned int ch)
        : action(a), payload('\0'), line_pos(ln), char_pos(ch) {}
    Change(ActionType a, char c, unsigned int ln, unsigned int ch)
        : action(a), payload(c), line_pos(ln), char_pos(ch) {}
};

#endif  // CHANGE_H
