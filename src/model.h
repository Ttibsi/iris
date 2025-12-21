#ifndef MODEL_H
#define MODEL_H

#include <optional>
#include <stack>
#include <string>
#include <string_view>
#include <vector>

#include <rawterm/screen.h>

#include "change.h"

// Forward declare from controller.h
enum class Redraw;

// Forward declare from view.h
struct View;

enum class ModelType { BUF, META };

struct WordPos {
    std::string text;
    uint_t start_pos;
    uint_t lineno;
};

struct Model {
    ModelType type = ModelType::BUF;
    std::vector<std::string> buf;
    std::string filename;
    unsigned int current_line = 0;  // 0-indexed
    unsigned int current_char = 0;  // 0-indexed
    std::string search_str = "";
    std::size_t vertical_offset = 0;

    // Num of lines offset to view
    unsigned int view_offset = 0;

    bool readonly = false;
    bool unsaved = false;

    std::vector<Change> undo_stack = {};
    std::stack<Change> redo_stack = {};

    Model(std::size_t, std::string_view);
    Model(std::vector<std::string>, std::string_view);
    [[nodiscard]] Redraw backspace();
    [[nodiscard]] std::size_t newline();
    void insert(const char);
    [[nodiscard]] bool lineno_in_scope(const int) const;
    [[nodiscard]] std::optional<int> next_word_pos();
    [[nodiscard]] std::optional<int> prev_word_pos();
    [[nodiscard]] std::optional<unsigned int> next_para_pos();
    [[nodiscard]] std::optional<unsigned int> prev_para_pos();
    void replace_char(const char);
    void toggle_case();
    [[nodiscard]] std::optional<rawterm::Pos> find_next(const char);
    [[nodiscard]] std::optional<rawterm::Pos> find_prev(const char);
    [[nodiscard]] bool undo(const int);
    [[nodiscard]] char get_current_char() const;
    [[nodiscard]] bool redo(const int);
    void move_line_down();
    void move_line_up();
    void set_read_only(std::string_view);
    void delete_current_line();
    [[nodiscard]] const WordPos current_word() const;
    void delete_current_word(const WordPos);
    [[nodiscard]] std::vector<std::string> search_text(const std::string&) const;
    void search_and_replace(const std::string&);
    std::optional<rawterm::Pos> find_next_str(std::string_view);
};

#endif  // MODEL_H
