#include <memory>

#include "buffer.h"
#include "viewport.h"

Viewport::Viewport(rawterm::Pos size, std::shared_ptr<Buffer> b)
    : view_size(size), buf_ptr(b) {}
