#ifndef CHANGE_H
#define CHANGE_H

#include <optional>

// Forward declare to prevent circular header includes
enum class ActionType;

// TODO: Remove optional from payload?
struct Change {
    const ActionType action;
    const std::optional<char> payload;
    const std::optional<unsigned int> line_pos;
    const std::optional<unsigned int> char_pos;

    Change(ActionType a) : action(a) {}
    Change(ActionType a, unsigned int ln, unsigned int ch)
        : action(a), payload({}), line_pos(ln), char_pos(ch) {}
    Change(ActionType a, char c, unsigned int ln, unsigned int ch)
        : action(a), payload(c), line_pos(ln), char_pos(ch) {}
};

#endif  // CHANGE_H
