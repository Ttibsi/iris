#include "controller.h"

#include <rawterm/core.h>

#include "action.h"

// return value - if we continue to check for input or not
[[nodiscard]] ParseInputRet parse_input() {
    auto k = rawterm::process_keypress();
    if (k.has_value()) {
        if (k.value() == rawterm::Key('h')) {
                parse_action<void, None>(Action<void>(ActionType::MoveCursorLeft));
        } else if (k.value() == rawterm::Key('j')) {
                parse_action<void, None>(Action<void>(ActionType::MoveCursorUp));
        } else if (k.value() == rawterm::Key('k')) {
                parse_action<void, None>(Action<void>(ActionType::MoveCursorDown));
        } else if (k.value() == rawterm::Key('l')) {
                parse_action<void, None>(Action<void>(ActionType::MoveCursorRight));
        } else if (k.value() == rawterm::Key('q')) {
            return ParseInputRet::Break;
        }
    }

    return ParseInputRet::None;
}
