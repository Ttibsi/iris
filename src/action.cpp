
#include "action.h"
#include "logger.h"

template <typename T, typename U>
constexpr std::optional<const U> parse_action(const Action<T>& action) {
    switch (action.type) {
            case ActionType::MoveCursorLeft:
                log("Action called: MoveCursorLeft");
            case ActionType::MoveCursorLeft:
                log("Action called: MoveCursorLeft");
            case ActionType::MoveCursorDown:
                log("Action called: MoveCursorDown");
            case ActionType::MoveCursorRight:
                log("Action called: MoveCursorRight");
            default:
                log(Level::WARNING, "Unknown action called " + action.type);
    }
}
