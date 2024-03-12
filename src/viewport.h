#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <memory>

#include <rawterm/rawterm.h>

#include "buffer.h"
#include "cursor.h"

struct Viewport {
    rawterm::Pos view_size;
    Cursor cursor;
    std::shared_ptr<Buffer> buf_ptr;

    Viewport(rawterm::Pos);
};

#endif // VIEWPORT_H
