#ifndef CHANGE_H
#define CHANGE_H

#include <optional>

// Forward declare to prevent circular header includes
enum class ActionType;

struct Change {
    const ActionType action;
    const std::optional<char> payload;
    const std::optional<int> line_pos;
    const std::optional<int> char_pos;
};

#endif  // CHANGE_H
