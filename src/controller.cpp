#include "editor.h"

#include <rawterm/core.h>

#include "action.h"

void Editor::start_controller() {
    log("Starting Controller");
    bool break_loop = false;
    bool redraw = false;

    while (!(break_loop)) {
        if (redraw) {
            views.at(active_view).pane_manager.draw_all();
            redraw = false;
        }

        auto k = rawterm::process_keypress();
        if (k.has_value()) {
            if (k.value() == rawterm::Key('h')) {
                parse_action<void, None>(this, Action<void>{ActionType::MoveCursorLeft});
            } else if (k.value() == rawterm::Key('j')) {
                parse_action<void, None>(this, Action<void>{ActionType::MoveCursorUp});
            } else if (k.value() == rawterm::Key('k')) {
                parse_action<void, None>(this, Action<void>{ActionType::MoveCursorDown});
            } else if (k.value() == rawterm::Key('l')) {
                parse_action<void, None>(this, Action<void>{ActionType::MoveCursorRight});
            } else if (k.value() == rawterm::Key('q')) {
                log("Breaking out from Controller");
                break_loop = true;
            }
        }
    }
}
