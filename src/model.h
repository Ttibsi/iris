#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>

#include "gapvector.h"
#include "view.h"

struct Editor;

struct Model {
    const Editor* editor;
    Gapvector<> gv;  // TODO: Investigate char32_t to allow for unicode
    const std::string filename;
    int linenum_offset = 0;
    int current_line = 1;
    int line_col = 1;
    bool modified = false;
    bool readonly = false;

    Model(const Editor*);
    Model(const Editor*, Gapvector<>, std::string);
    [[nodiscard]] std::vector<std::string> render(const View*, const rawterm::Pos&);
    [[nodiscard]] std::vector<std::string> render(const View*);
    [[nodiscard]] const std::string render_status_bar(const int) const;
};

#endif  // MODEL_H
