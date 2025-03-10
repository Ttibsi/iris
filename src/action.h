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
    EndOfLine,
    MoveCursorLeft,
    MoveCursorUp,
    MoveCursorDown,
    MoveCursorRight,
    Newline,
    DelCurrentChar,

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
            if constexpr (std::is_same_v<U, Redraw>) {
                auto logger = spdlog::get("basic_logger");
                if (logger != nullptr) {
                    logger->info("Action called: Backspace");
                }

                return v->get_active_model()->backspace();
            }
        } break;

        case ActionType::EndOfLine: {
            auto logger = spdlog::get("basic_logger");
            if (logger != nullptr) {
                logger->info("Action called: EndOfLine");
            }

            v->cursor_end_of_line();
        } break;

        case ActionType::MoveCursorLeft: {
            auto logger = spdlog::get("basic_logger");
            if (logger != nullptr) {
                logger->info("Action called: MoveCursorLeft");
            }

            v->cursor_left();
            return {};
        } break;

        case ActionType::MoveCursorUp: {
            if constexpr (std::is_same_v<U, bool>) {
                auto logger = spdlog::get("basic_logger");
                if (logger != nullptr) {
                    logger->info("Action called: MoveCursorUp");
                }
                return v->cursor_up();
            }
        } break;

        case ActionType::MoveCursorDown: {
            if constexpr (std::is_same_v<U, bool>) {
                auto logger = spdlog::get("basic_logger");
                if (logger != nullptr) {
                    logger->info("Action called: MoveCursorDown");
                }
                return v->cursor_down();
            }
        } break;

        case ActionType::MoveCursorRight: {
            auto logger = spdlog::get("basic_logger");
            if (logger != nullptr) {
                logger->info("Action called: MoveCursorRight");
            }

            v->cursor_right();
            return {};
        } break;

        case ActionType::Newline: {
            auto logger = spdlog::get("basic_logger");
            if (logger != nullptr) {
                logger->info("Action called: Newline");
            }

            const std::size_t count = v->get_active_model()->newline();
            v->cur.move_down();
            for (std::size_t i = 0; i < count; i++) {
                v->cur.move_left();
            }

            return {};
        } break;

        case ActionType::DelCurrentChar: {
            if constexpr (std::is_same_v<U, Redraw>) {
                auto logger = spdlog::get("basic_logger");
                if (logger != nullptr) {
                    logger->info("Action called: DelCurrentChar");
                }

                v->cursor_right();
                Redraw ret = v->get_active_model()->backspace();
                v->cur.move_left();

                return ret;
            }
        } break;

        case ActionType::ChangeMode: {
            auto logger = spdlog::get("basic_logger");
            if (logger != nullptr) {
                logger->info("Action called: ChangeMode");
            }

            if constexpr (std::is_same_v<T, Mode>) {
                v->ctrlr_ptr->set_mode(action.payload);
            }

            return {};
        } break;

        case ActionType::InsertChar: {
            if constexpr (std::is_same_v<T, char>) {
                auto logger = spdlog::get("basic_logger");
                if (logger != nullptr) {
                    logger->info("Action called: InsertChar");
                }

                v->get_active_model()->insert(action.payload);
                v->prev_cur_hor_pos = -1;
                v->cur.move_right();
            }
            return {};
        } break;

        default: {
            auto logger = spdlog::get("basic_logger");
            if (logger != nullptr) {
                logger->info("Unknown action called");
            }

            return {};
        } break;

    }  // End of switch case

    return {};
}

#endif  // ACTION_H
