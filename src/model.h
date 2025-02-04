#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <string_view>
#include <vector>

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

    Model(const int, std::string_view);
    Model(std::vector<std::string>, std::string_view);
    [[nodiscard]] Redraw backspace();
    [[nodiscard]] int newline();
    void insert(const char);
    [[nodiscard]] bool lineno_in_scope(const int) const;
};

#endif  // MODEL_H
