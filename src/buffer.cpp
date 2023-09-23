#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "buffer.h"
#include "constants.h"
#include "file_manip.h"

Buffer::Buffer(std::pair<int, int> size) {
    this->lines = { Line() };
    this->data = "";
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

    this->data = open_file(file);
    this->file = file;
    this->readonly = is_Readonly(file);
    this->modified = false;
    this->undo = {};
    this->redo = {};
    this->buf_size = size;

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

    handle_keypress();
}

void Buffer::display() {
    std::string copy = data;
    int count;

    std::unordered_map<std::string, std::string> pairs = {
        { "\n", "\r\n" }, { "\t", std::string(TABSTOP, ' ') }
    };

    for (int i = 0; i < copy.length();) {
        std::string s = &copy.at(i);
        if (auto it = pairs.find({ copy.at(i) }); it != pairs.end()) {
            std::string val = it->second;
            copy.replace(i, 1, val);
            i += val.length();
            count += (val.length() - it->first.length());
        } else {
            i++;
        }
    }

    // WARN: Not sure this is working -- too high
    std::string_view sv = copy.substr(0, lines[buf_size.first - 1].end - count);
    std::cout << sv;
}
