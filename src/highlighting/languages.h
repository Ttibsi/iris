#ifndef LANGUAGES_H
#define LANGUAGES_H

#include <string>
#include <unordered_map>

enum class Language {
    UNKNOWN, // default value
    CMAKE,
    PYTHON,
    TEXT,
};

inline std::string lang_string(const Language &l) {
    switch (l) {
    case Language::UNKNOWN:
        return "UNKNOWN";
    case Language::CMAKE:
        return "CMAKE";
    case Language::PYTHON:
        return "PYTHON";
    case Language::TEXT:
        return "TEXT";
    }

    return "";
}

static std::unordered_map<std::string, Language> languages = {
    {".cmake", Language::CMAKE },
    { ".py",   Language::PYTHON},
    { ".txt",  Language::TEXT  },
};

#endif // LANGUAGES_H
