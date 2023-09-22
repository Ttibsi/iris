#include <iostream>
#include <string>

#include "buffer.h"
#include "file_manip.h"

Buffer::Buffer() {
    this->lines = { Line() };
    this->data = "";
    this->length = 1;
    this->file = "NO FILE";
    this->cursor = Cursor();
    this->readonly = false;
    this->modified = false;
    this->undo = {};
    this->redo = {};
}

Buffer::Buffer(std::string file) {
    // TODO: What if the given path is a directory?

    this->data = open_file(file);
    this->file = file;
    this->readonly = is_Readonly(file);
    this->modified = false;
    this->undo = {};
    this->redo = {};

    std::vector<Line> lines;
    Line l = Line();
    for (std::size_t i = 0; i < this->data.length(); i++) {
        if (this->data.at(i) == '\n') {
            l.end = i;
            lines.push_back(l);

            if (!(i == this->data.length() - 1)) {
                l = Line();
                l.start = i++;
            }
        }
    }

    this->lines = lines;
    this->length = lines.size();
    display();
    this->cursor = Cursor();
}

void Buffer::display() {
    // TODO: TABS
    std::string copy = data;
    size_t startPos = 0;

    while ((startPos = copy.find("\n", startPos)) != std::string::npos) {
        copy.replace(startPos, 1, "\r\n");
        startPos += 2;
    }

    std::cout << copy;
}
