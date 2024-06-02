#include "text_transform.h"

[[nodiscard]] std::string strip_trailing_whitespace(std::string str) {
    if (str.find_last_not_of(' ') == str.size()) {
        return str;
    }
    return str.substr(0, str.find_last_not_of(' ') + 1);
}

[[nodiscard]] std::string strip_newline(std::string str) {
    if (str.back() == '\n') {
        str = str.substr(0, str.size() - 1);
    }

    return str;
}
