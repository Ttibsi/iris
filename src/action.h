#ifndef ACTION_H
#define ACTION_H

#include <optional>
#include <stdexcept>

#include "logger.h"
#include "view.h"

struct None {};

enum class ActionType {
    MoveCursorLeft,
    MoveCursorDown,
    MoveCursorRight,
    MoveCursorUp,
};

template <typename T>
struct Action {
    const ActionType type;
    const T payload;
};

template <>
struct Action<void> {
    const ActionType type;
};

template <typename T, typename U>
constexpr std::optional<const U> parse_action(View* v, const Action<T>& action) {
    switch (action.type) {
        case ActionType::MoveCursorLeft:
            log("Action called: MoveCursorLeft");
            return {};
        case ActionType::MoveCursorUp:
            log("Action called: MoveCursorUp");
            return {};
        case ActionType::MoveCursorDown:
            log("Action called: MoveCursorDown");
            return {};
        case ActionType::MoveCursorRight:
            log("Action called: MoveCursorRight");
            return {};
        default:
            log(Level::WARNING, "Unknown action called");
            return {};
    }
}

#endif  // ACTION_H
