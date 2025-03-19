#ifndef MODEL_H
#define MODEL_H

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <rawterm/screen.h>

// Forward declare from controller.h
enum class Redraw;

struct Model {
    std::vector<std::string> buf;
    std::string filename;
    unsigned int current_line = 0;  // 0-indexed
    unsigned int current_char = 0;  // 0-indexed

    // Num of lines offset to view
    unsigned int view_offset = 0;

    bool readonly = false;
    bool modified = false;

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
};

#endif  // MODEL_H
