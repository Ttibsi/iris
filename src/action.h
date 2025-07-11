#ifndef ACTION_H
#define ACTION_H

#include <cstdint>
#include <optional>

#include "change.h"
#include "controller.h"
#include "spdlog/spdlog.h"
#include "view.h"

struct None {};

enum class ActionType {
    // Pass no values
    Backspace,
    CenterCurrentLine,
    DelCurrentChar,
    DelCurrentLine,
    EndOfLine,
    JumpNextPara,
    JumpPrevPara,
    JumpNextWord,
    JumpPrevWord,
    MoveCursorLeft,
    MoveCursorUp,
    MoveCursorDown,
    MoveCursorRight,
    MoveLineDown,
    MoveLineUp,
    Newline,
    StartOfLine,
    TabNew,
    TabNext,
    TabPrev,
    ToggleCase,
    TriggerRedo,
    TriggerUndo,

    // Pass value
    ChangeMode,   // Mode
    FindNext,     // Char
    FindPrev,     // Char
    InsertChar,   // Char
    ReplaceChar,  // Char
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

                // Skip if cur at 0,0
                if (v->get_active_model()->current_line == 0 &&
                    v->get_active_model()->current_char == 0) {
                    return Redraw::None;
                }

                // For cursor movement
                std::size_t prev_line_len = 0;
                if (v->get_active_model()->current_line > 0) {
                    prev_line_len = v->get_active_model()
                                        ->buf.at(v->get_active_model()->current_line - 1)
                                        .size();
                }

                // Log for undo
                char prev_char = '\n';
                if (v->get_active_model()->current_char > 0) {
                    prev_char = v->get_active_model()
                                    ->buf.at(v->get_active_model()->current_line)
                                    .at(v->get_active_model()->current_char - 1);
                }

                v->get_active_model()->undo_stack.push_back(Change(
                    ActionType::Backspace, v->get_active_model()->current_line,
                    v->get_active_model()->current_char, prev_char));

                // actual backspace
                Redraw ret = v->get_active_model()->backspace();
                if (ret == Redraw::Screen) {
                    v->cur.move_up();
                    v->cur.move_right(int32_t(prev_line_len));
                } else {
                    v->cur.move_left();
                }

                return ret;
            }
        } break;

        case ActionType::CenterCurrentLine: {
            v->center_current_line();
        } break;

        case ActionType::DelCurrentChar: {
            if constexpr (std::is_same_v<U, Redraw>) {
                auto logger = spdlog::get("basic_logger");
                if (logger != nullptr) {
                    logger->info("Action called: DelCurrentChar");
                }

                v->get_active_model()->undo_stack.push_back(Change(
                    ActionType::DelCurrentChar, v->get_active_model()->current_line,
                    v->get_active_model()->current_char,
                    v->get_active_model()->get_current_char()));
                v->cursor_right();
                Redraw ret = v->get_active_model()->backspace();
                v->cur.move_left();

                return ret;
            }
        } break;

        case ActionType::DelCurrentLine: {
            v->get_active_model()->undo_stack.push_back(Change(
                ActionType::DelCurrentLine, v->get_active_model()->current_line,
                v->get_active_model()->current_char,
                v->get_active_model()->buf.at(v->get_active_model()->current_line)));

            v->get_active_model()->delete_current_line();
            v->change_model_cursor();
            v->draw_screen();
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

            std::optional<unsigned int> count = v->get_active_model()->next_para_pos();
            if (count.has_value()) {
                v->cursor_down(count.value());
            }

        } break;

        case ActionType::JumpPrevPara: {
            auto logger = spdlog::get("basic_logger");
            if (logger != nullptr) {
                logger->info("Action called: JumpPrevPara");
            }

            std::optional<unsigned int> count = v->get_active_model()->prev_para_pos();
            if (count.has_value()) {
                v->cursor_up(count.value());
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

        case ActionType::MoveLineDown: {
            auto logger = spdlog::get("basic_logger");
            if (logger != nullptr) {
                logger->info("Action called: MoveLineDown");
            }

            v->get_active_model()->move_line_down();
            v->cursor_down();
            return {};
        } break;

        case ActionType::MoveLineUp: {
            auto logger = spdlog::get("basic_logger");
            if (logger != nullptr) {
                logger->info("Action called: MoveLineUp");
            }

            v->get_active_model()->move_line_up();
            v->cursor_up();
            return {};
        } break;

        case ActionType::Newline: {
            auto logger = spdlog::get("basic_logger");
            if (logger != nullptr) {
                logger->info("Action called: Newline");
            }

            v->get_active_model()->undo_stack.push_back(Change(
                ActionType::Newline, v->get_active_model()->current_line,
                v->get_active_model()->current_char));

            const std::size_t count = v->get_active_model()->newline();
            if (!(v->cur.vertical == v->view_size.vertical - 2)) {
                v->cur.move_down();
            } else {
                v->get_active_model()->view_offset++;
            }

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

        case ActionType::TabNew: {
            auto logger = spdlog::get("basic_logger");
            if (logger != nullptr) {
                logger->info("Action called: TabNew");
            }

            v->tab_new();

        } break;
        case ActionType::TabNext: {
            auto logger = spdlog::get("basic_logger");
            if (logger != nullptr) {
                logger->info("Action called: TabNext");
            }

            v->tab_next();
        } break;
        case ActionType::TabPrev: {
            auto logger = spdlog::get("basic_logger");
            if (logger != nullptr) {
                logger->info("Action called: TabPrev");
            }

            v->tab_prev();
        } break;

        case ActionType::ToggleCase: {
            auto logger = spdlog::get("basic_logger");
            if (logger != nullptr) {
                logger->info("Action called: Newline");
            }

            v->get_active_model()->toggle_case();

            v->get_active_model()->undo_stack.push_back(Change(
                ActionType::ToggleCase, v->get_active_model()->current_line,
                v->get_active_model()->current_char));

        } break;

        case ActionType::TriggerRedo: {
            if constexpr (std::is_same_v<U, bool>) {
                auto logger = spdlog::get("basic_logger");
                if (logger != nullptr) {
                    logger->info("Action called: TriggerRedo");
                }

                return v->get_active_model()->redo(v->view_size.horizontal);
            }

            return {};
        } break;

        case ActionType::TriggerUndo: {
            if constexpr (std::is_same_v<U, bool>) {
                auto logger = spdlog::get("basic_logger");
                if (logger != nullptr) {
                    logger->info("Action called: TriggerUndo");
                }

                return v->get_active_model()->undo(v->view_size.horizontal);
            }

            return {};
        } break;

        case ActionType::ChangeMode: {
            if constexpr (std::is_same_v<T, Mode>) {
                auto logger = spdlog::get("basic_logger");
                if (logger != nullptr) {
                    logger->info("Action called: ChangeMode");
                }

                v->ctrlr_ptr->set_mode(action.payload);
            }

            return {};
        } break;

        case ActionType::FindNext: {
            if constexpr (std::is_same_v<T, char>) {
                auto ret = v->get_active_model()->find_next(action.payload);
                if (ret.has_value()) {
                    for (int i = 0; i < ret.value().vertical; i++) {
                        v->cursor_down();
                    }

                    if (ret.value().horizontal > int32_t(v->get_active_model()->current_char)) {
                        std::size_t diff =
                            uint32_t(ret.value().horizontal) - v->get_active_model()->current_char;
                        for (std::size_t i = 0; i < diff; i++) {
                            v->cursor_right();
                        }
                    } else {
                        uint32_t diff =
                            v->get_active_model()->current_char - uint32_t(ret.value().horizontal);
                        for (unsigned int i = 0; i < diff; i++) {
                            v->cursor_left();
                        }
                    }

                    v->draw_status_bar();
                }
            }
        } break;

        case ActionType::FindPrev: {
            if constexpr (std::is_same_v<T, char>) {
                auto ret = v->get_active_model()->find_prev(action.payload);
                if (ret.has_value()) {
                    for (int i = 0; i < ret.value().vertical; i++) {
                        v->cursor_up();
                    }

                    if (ret.value().horizontal > int32_t(v->get_active_model()->current_char)) {
                        uint32_t diff =
                            uint32_t(ret.value().horizontal) - v->get_active_model()->current_char;
                        for (uint32_t i = 0; i < diff; i++) {
                            v->cursor_right();
                        }
                    } else {
                        uint32_t diff =
                            v->get_active_model()->current_char - uint32_t(ret.value().horizontal);
                        for (uint32_t i = 0; i < diff; i++) {
                            v->cursor_left();
                        }
                    }
                }
            }
        } break;

        case ActionType::InsertChar: {
            if constexpr (std::is_same_v<T, char>) {
                auto logger = spdlog::get("basic_logger");
                if (logger != nullptr) {
                    logger->info("Action called: InsertChar");
                }

                v->get_active_model()->insert(action.payload);
                v->cur.move_right();

                v->get_active_model()->undo_stack.push_back(Change(
                    ActionType::InsertChar, v->get_active_model()->current_line,
                    v->get_active_model()->current_char, action.payload));
            }
            return {};
        } break;

        case ActionType::ReplaceChar: {
            if constexpr (std::is_same_v<T, char>) {
                auto logger = spdlog::get("basic_logger");
                if (logger != nullptr) {
                    logger->info("Action called: ReplaceChar");
                }

                v->get_active_model()->undo_stack.push_back(Change(
                    ActionType::ReplaceChar, v->get_active_model()->current_line,
                    v->get_active_model()->current_char,
                    v->get_active_model()->get_current_char()));
                v->get_active_model()->replace_char(action.payload);
            }
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
