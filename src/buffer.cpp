#include <iostream>
#include <string>
#include <unordered_map>

#include "buffer.h"
#include "constants.h"
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
    std::string copy = data;

    std::unordered_map<std::string, std::string> pairs = {
        { "\n", "\r\n" }, { "\t", std::string(TABSTOP, ' ') }
    };

    for (int i = 0; i < copy.length();) {
        std::string s = &copy.at(i);
        if (auto it = pairs.find({ copy.at(i) }); it != pairs.end()) {
            std::string val = it->second;
            copy.replace(i, 1, val);
            i += val.length();
        } else {
            i++;
        }
    }

    std::cout << copy;
}
