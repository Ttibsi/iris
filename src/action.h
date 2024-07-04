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
            e->views.at(e->active_view).redraw_pane();
            return {};
        case ActionType::MoveCursorUp:
            log("Action called: MoveCursorUp");
            e->views.at(e->active_view).move_cur_up();
            log(e->views.at(e->active_view).pane_manager.get_cur_pos());
            e->views.at(e->active_view).redraw_pane();
            return {};
        case ActionType::MoveCursorDown:
            log("Action called: MoveCursorDown");
            e->views.at(e->active_view).move_cur_down();
            log(e->views.at(e->active_view).pane_manager.get_cur_pos());
            e->views.at(e->active_view).redraw_pane();
            log(e->views.at(e->active_view).pane_manager.get_cur_pos());
            return {};
        case ActionType::MoveCursorRight:
            log("Action called: MoveCursorRight");
            e->views.at(e->active_view).move_cur_right();
            e->views.at(e->active_view).redraw_pane();
            return {};
        default:
            log(Level::WARNING, "Unknown action called");
            return {};
    }
}

#endif  // ACTION_H
