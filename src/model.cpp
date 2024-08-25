#include "model.h"

#include <algorithm>

Model::Model() : buf(Gapvector()) {};

// TODO: readonly and modified
Model::Model(const std::vector<char>& file_chars, const std::string& filename)
    : buf(Gapvector(file_chars.begin(), file_chars.end())),
      file_name(filename),
      line_count(std::count(buf.begin(), buf.end(), '\n')) {};
