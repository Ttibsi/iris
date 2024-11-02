#ifndef ACTION_H
#define ACTION_H

#include <optional>

#include "controller.h"
#include "logger.h"
#include "view.h"

struct None {};

enum class ActionType {
    // Pass no values
    Backspace,
    MoveCursorLeft,
    MoveCursorDown,
    MoveCursorRight,
    MoveCursorUp,
    Newline,
    SaveFile,

    // Pass value
    ChangeMode,  // Mode
    InsertChar,  // char
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
            v->cursor_left();
            return {};

        case ActionType::MoveCursorUp:
            log("Action called: MoveCursorUp");
            v->cursor_up();
            return {};

        case ActionType::MoveCursorDown:
            log("Action called: MoveCursorDown");
            v->cursor_down();
            return {};

        case ActionType::MoveCursorRight:
            log("Action called: MoveCursorRight");
            v->cursor_right();
            return {};

        case ActionType::ChangeMode:
            log("Action called: ChangeMode");
            if constexpr (std::is_same_v<T, Mode>) {
                v->ctrlr_ptr->set_mode(action.payload);
                v->draw_status_bar();
            }
            return {};

        case ActionType::SaveFile:
            log("Action called: SaveFile");
            v->get_active_model()->save_file();
            return {};

        case ActionType::Backspace: {
            log("Action called: Backspace");
            // TODO: backspace on \n should also delete \r
            Model* active = v->get_active_model();
            if (active->current_char_in_line == 1 && active->current_line == 1) {
                return {};
            }

            if (active->current_char_in_line == 1 && active->current_line > 1) {
                // At the start of the line, move cursor up
                int prev_line_len = active->buf.line(active->get_abs_pos() - 1).size();
                active->buf.erase(active->buf.begin() + active->get_abs_pos() - 2, 2);
                v->cursor_up();
                for (int i = 0; i <= prev_line_len - 1; i++) {
                    v->cursor_right();
                }
                active->line_count--;
                v->render_screen();
            } else {
                // Move cursor backwards
                active->buf.erase(active->buf.begin() + active->get_abs_pos() - 1);
                v->cursor_left();
                v->render_line();
            }
            return {};
        }

        case ActionType::Newline: {
            log("Action called: Newline");
            Model* active = v->get_active_model();
            active->buf.insert(active->buf.begin() + active->get_abs_pos(), "\r\n");
            v->cursor_down();
            while (active->current_char_in_line > 1) {
                v->cursor_left();
            }
            active->line_count++;
            v->render_screen();
            return {};
        }

        case ActionType::InsertChar:
            log("Action called: InsertChar");
            if constexpr (std::is_same_v<T, char>) {
                v->get_active_model()->insert_char(action.payload);
                v->cursor_right();
            }
            return {};

        default:
            log(Level::WARNING, "Unknown action called");
            return {};
    }
}

#endif  // ACTION_H
