#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <format>

#include <rawterm/rawterm.h>

#include "buffer.h"
#include "constants.h"
#include "cursor.h"
#include "editor.h"
#include "highlighting/highlighter.h"
#include "text_manip.h"

struct Viewport {
    Buffer *buffer;
    rawterm::Pos view_size;
    Cursor cursor;
    bool resize_flag = false;
    int horizontal_offset = 0;

    Viewport(Buffer *, rawterm::Pos);
    void draw(const std::size_t &);
    void redraw_line();
    void switch_to_insert();
    void switch_to_command();
    void center(unsigned int);
    void cursor_up(unsigned int);
    void cursor_down(unsigned int);

    // handle_keypress.cpp
    void keypress_read();
    void keypress_write();
    void keypress_command();
};

#endif // VIEWPORT_H
