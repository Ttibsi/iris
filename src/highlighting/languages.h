#ifndef LANGUAGES_H
#define LANGUAGES_H

#include <string>
#include <unordered_map>

enum class Language {
    UNKNOWN, // default value
    PYTHON,
};

static std::unordered_map<std::string, Language> languages = {
    {".py", Language::PYTHON},
};

#endif // LANGUAGES_H
