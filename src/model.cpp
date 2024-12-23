#include "model.h"

Model::Model() : buf(TwinArray<char>()), filename("") {}

// TODO: readonly and modified
Model::Model(TwinArray<char> file_chars, const std::string &filename)
    : buf(file_chars), filename(filename) {};
