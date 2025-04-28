#include "text_io.h"

#include <array>
#include <fstream>
#include <sstream>

#include <rawterm/text.h>

#include "constants.h"

[[nodiscard]] opt_lines_t open_file(const std::string& file) {
    auto ret = std::vector<std::string>();
    std::string line = "";
    char ch;

    std::ifstream ifs(file);
    if (ifs.fail()) {
        return {};
    }

    while (ifs.get(ch)) {
        switch (ch) {
            case '\r':
                break;
            case '\t':
                line += std::string(TAB_SIZE, ' ');
                break;
            case '\n':
                ret.push_back(line);
                line = "";
                break;
            default:
                line.push_back(ch);
        };
    }

    // If there's no newlines in the stream at all, it never gets added to
    // the vector
    if (line.size()) {
        ret.push_back(line);
    }

    return ret;
}

[[nodiscard]] WriteData write_to_file(const Model& model) {
    if (model.filename == "") {
        return WriteData();
    }
    std::ofstream out(model.filename);
    for (auto&& line : model.buf) {
        out << line << "\n";
    }
    return WriteData(static_cast<int>(out.tellp()), int32_t(model.buf.size()));
}

[[nodiscard]] lines_t lines(const std::string& str) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string line;

    while (std::getline(ss, line)) {
        if (line.back() == '\r') {
            line.pop_back();
        }
        result.push_back(rawterm::raw_str(line));
    }

    return result;
}

[[nodiscard]] bool is_letter(const char& c) {
    std::array<char, 52> alphabet = {
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
        's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
        'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};

    return std::find(alphabet.begin(), alphabet.end(), c) != alphabet.end();
}

[[nodiscard]] std::string check_filename(const std::string& filename) {
    std::string err_text = "Iris currently does not support tab-delineated files";

    if (filename == "Makefile" || filename == "makefile") {
        return err_text;
    } else if (filename.ends_with(".go")) {
        return err_text;
    }

    return "";
}
