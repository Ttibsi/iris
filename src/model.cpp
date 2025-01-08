#include "model.h"

Model::Model(const int view_height) : buf({""}), filename("") {
    buf.reserve(view_height);
}

// TODO: readonly and modified
// NOTE: Intentional copy of file_chars
Model::Model(std::vector<std::string> file_chars, const std::string& filename)
    : buf(file_chars), filename(filename) {}
