#ifndef MODEL_H
#define MODEL_H

#include <string>
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

    Model(const int);
    Model(std::vector<std::string>, const std::string&);
    [[nodiscard]] Redraw backspace();
    [[nodiscard]] int newline();
    void insert(const char);
};

#endif  // MODEL_H
