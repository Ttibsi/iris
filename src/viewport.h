#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <memory>

#include <rawterm/rawterm.h>

#include "cursor.h"
#include <buffer.h>

struct Viewport {
    rawterm::Pos view_size;
    Cursor cursor;
    std::shared_ptr<Buffer> buf_ptr;
};

#endif // VIEWPORT_H
