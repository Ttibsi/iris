#include "model.h"

Model::Model() : buf(Gapvector()) {};

// TODO: readonly and modified
Model::Model(const std::vector<char>& file_chars, const std::string& filename)
    : buf(Gapvector(file_chars.begin(), file_chars.end())), file_name(filename) {};
