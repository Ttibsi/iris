#ifndef ACTION_H
#define ACTION_H

#include <optional>

#include "controller.h"
#include "spdlog/spdlog.h"
#include "view.h"

struct None {};

enum class ActionType {
    // Pass no values
    Backspace,
    MoveCursorLeft,
    MoveCursorUp,
    MoveCursorDown,
    MoveCursorRight,
    Newline,

    // Pass value
    ChangeMode,  // Mode
    InsertChar,  // Char
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
[[maybe_unused]] constexpr std::optional<const U> parse_action(View* v, const Action<T>& action) {
    switch (action.type) {
        case ActionType::Backspace: {
            auto logger = spdlog::get("basic_logger");
            if (logger != nullptr) {
                logger->info("Action called: Backspace");
            }

            v->get_active_model()->backspace();
            return {};
        }

        case ActionType::MoveCursorLeft: {
            auto logger = spdlog::get("basic_logger");
            if (logger != nullptr) {
                logger->info("Action called: MoveCursorLeft");
            }

            v->cursor_left();
            return {};
        }

        case ActionType::MoveCursorUp: {
            if constexpr (std::is_same_v<U, bool>) {
                auto logger = spdlog::get("basic_logger");
                if (logger != nullptr) {
                    logger->info("Action called: MoveCursorUp");
                }
                return v->cursor_up();
            }
        }

        case ActionType::MoveCursorDown: {
            if constexpr (std::is_same_v<U, bool>) {
                auto logger = spdlog::get("basic_logger");
                if (logger != nullptr) {
                    logger->info("Action called: MoveCursorDown");
                }
                return v->cursor_down();
            }
        }

        case ActionType::MoveCursorRight: {
            auto logger = spdlog::get("basic_logger");
            if (logger != nullptr) {
                logger->info("Action called: MoveCursorRight");
            }

            v->cursor_right();
            return {};
        }

        case ActionType::Newline: {
            auto logger = spdlog::get("basic_logger");
            if (logger != nullptr) {
                logger->info("Action called: Newline");
            }

            const int count = v->get_active_model()->newline();
            v->cur.move_down();
            for (int i = 0; i < count; i++) {
                v->cur.move_left();
            }

            return {};
        }

        default: {
            auto logger = spdlog::get("basic_logger");
            if (logger != nullptr) {
                logger->info("Unknown action called");
            }

            return {};
        }

        case ActionType::ChangeMode: {
            auto logger = spdlog::get("basic_logger");
            if (logger != nullptr) {
                logger->info("Action called: ChangeMode");
            }

            if constexpr (std::is_same_v<T, Mode>) {
                v->ctrlr_ptr->set_mode(action.payload);
            }

            return {};
        }

        case ActionType::InsertChar: {
            if constexpr (std::is_same_v<T, char>) {
                auto logger = spdlog::get("basic_logger");
                if (logger != nullptr) {
                    logger->info("Action called: InsertChar");
                }

                v->get_active_model()->insert(action.payload);
                v->cur.move_right();
            }
            return {};
        }

    }  // End of switch case
}

#endif  // ACTION_H
