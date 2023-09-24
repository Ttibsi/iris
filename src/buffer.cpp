#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "buffer.h"
#include "constants.h"
#include "file_manip.h"

Buffer::Buffer(std::pair<int, int> size) {
    this->lines = { "" };
    this->length = 1;
    this->file = "NO FILE";
    this->cursor = Cursor();
    this->readonly = false;
    this->modified = false;
    this->undo = {};
    this->redo = {};
    this->buf_size = size;

    handle_keypress();
}

Buffer::Buffer(std::string file, std::pair<int, int> size) {
    // TODO: What if the given path is a directory?

    this->lines = open_file(file);
    this->length = lines.size();
    this->file = file;
    this->readonly = is_Readonly(file);
    this->modified = false;
    this->undo = {};
    this->redo = {};
    this->buf_size = size;

    display();
    this->cursor = Cursor();

    handle_keypress();
}

void Buffer::display() {
    for (auto it = lines.begin(); it != lines.begin() + buf_size.first; it++) {
        std::cout << *it << "\r\n";
    }
}
