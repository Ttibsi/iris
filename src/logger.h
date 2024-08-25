#ifndef LOGGER_H
#define LOGGER_H

#include <cstdlib>
#include <fstream>
#include <string>
#include <string_view>

inline const std::string log_file = "iris.log";

enum class Level { INFO, WARNING, ERROR };

[[nodiscard]] inline std::string_view level_str(const Level& l) noexcept {
    switch (l) {
        case Level::INFO:
            return "[INFO]";
        case Level::WARNING:
            return "[WARNING]";
        case Level::ERROR:
            return "[ERROR]";
        default:
            return "";
    }
}

// TODO: Add date/time stamp to log
inline void log(Level lvl, std::string_view msg) {
    if (std::getenv("RAWTERM_DEBUG") != nullptr) {
        return;
    }

    std::ofstream out;
    out.open(log_file, std::ios::app);
    out << level_str(lvl) << " " << msg << "\n";
    out.close();
}

inline void log(std::string_view msg) {
    log(Level::INFO, msg);
}

#endif  // LOGGER_H
