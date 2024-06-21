#ifndef ACTION_H
#define ACTION_H

#include <optional>

#include "editor.h"
#include "logger.h"

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
constexpr std::optional<const U> parse_action(Editor* e, const Action<T>& action) {
    switch (action.type) {
        case ActionType::MoveCursorLeft:
            log("Action called: MoveCursorLeft");
            e->views.at(e->active_view).move_cur_left();
            return {};
        case ActionType::MoveCursorUp:
            log("Action called: MoveCursorUp");
            e->views.at(e->active_view).move_cur_up();
            return {};
        case ActionType::MoveCursorDown:
            log("Action called: MoveCursorDown");
            e->views.at(e->active_view).move_cur_down();
            return {};
        case ActionType::MoveCursorRight:
            log("Action called: MoveCursorRight");
            e->views.at(e->active_view).move_cur_right();
            return {};
        default:
            log(Level::WARNING, "Unknown action called");
            return {};
    }
}

#endif  // ACTION_H
