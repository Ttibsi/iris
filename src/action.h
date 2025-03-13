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
    DelCurrentChar,
    EndOfLine,
    JumpNextPara,
    JumpPrevPara,
    JumpNextWord,
    JumpPrevWord,
    MoveCursorLeft,
    MoveCursorUp,
    MoveCursorDown,
    MoveCursorRight,
    Newline,
    StartOfLine,

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

                Redraw ret = v->get_active_model()->backspace();
                v->cur.move_left();

                return ret;
            }
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

        case ActionType::EndOfLine: {
            auto logger = spdlog::get("basic_logger");
            if (logger != nullptr) {
                logger->info("Action called: EndOfLine");
            }

            v->cursor_end_of_line();
        } break;

        case ActionType::JumpNextPara: {
            auto logger = spdlog::get("basic_logger");
            if (logger != nullptr) {
                logger->info("Action called: JumpNextPara");
            }

            std::optional<int> count = v->get_active_model()->next_para_pos();
            if (count.has_value()) {
                for (int i = 0; i < count.value(); i++) {
                    v->cursor_down();
                }
            }

        } break;

        case ActionType::JumpPrevPara: {
            auto logger = spdlog::get("basic_logger");
            if (logger != nullptr) {
                logger->info("Action called: JumpPrevPara");
            }

            std::optional<int> count = v->get_active_model()->prev_para_pos();
            if (count.has_value()) {
                for (int i = 0; i < count.value(); i++) {
                    v->cursor_up();
                }
            }

        } break;

        case ActionType::JumpNextWord: {
            auto logger = spdlog::get("basic_logger");
            if (logger != nullptr) {
                logger->info("Action called: JumpNextWord");
            }

            std::optional<int> count = v->get_active_model()->next_word_pos();
            if (count.has_value()) {
                for (int i = 0; i < count.value(); i++) {
                    v->cursor_right();
                }
            }

        } break;

        case ActionType::JumpPrevWord: {
            auto logger = spdlog::get("basic_logger");
            if (logger != nullptr) {
                logger->info("Action called: JumpPrevWord");
            }

            std::optional<int> count = v->get_active_model()->prev_word_pos();
            if (count.has_value()) {
                for (int i = 0; i < count.value(); i++) {
                    v->cursor_left();
                }
            }

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

        case ActionType::StartOfLine: {
            auto logger = spdlog::get("basic_logger");
            if (logger != nullptr) {
                logger->info("Action called: Newline");
            }

            v->cursor_start_of_line();

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
