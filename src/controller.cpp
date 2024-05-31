#include "controller.h"

#include <rawterm/core.h>

#include "action.h"

// return value - if we continue to check for input or not
[[nodiscard]] ParseInputRet parse_input() {
    auto k = rawterm::process_keypress();
    if (k.has_value()) {
        if (k.value() == rawterm::Key('h')) {
        } else if (k.value() == rawterm::Key('j')) {
        } else if (k.value() == rawterm::Key('k')) {
        } else if (k.value() == rawterm::Key('l')) {
        } else if (k.value() == rawterm::Key('q')) {
            return ParseInputRet::Break;
        }
    }

    return ParseInputRet::None;
}
