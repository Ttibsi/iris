#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <string>
#include <string_view>

extern const std::string log_file;

enum class Level { INFO, WARNING, ERROR };

[[nodiscard]] inline std::string_view level_str(const Level &l) noexcept {
    switch (l) {
    case Level::INFO:
        return "[INFO]";
    case Level::WARNING:
        return "[WARNING]";
    case Level::ERROR:
        return "[ERROR]";
    }
    return "";
}

inline void log(Level lvl, std::string msg) {
    if (log_file.empty()) {
        // TODO: IDK, throw an exception? I wish there was a more graceful
        // way to handle this
    }

    std::ofstream out;
    out.open(log_file, std::ios::app);
    out << level_str(lvl) << " " << msg << "\n";
    out.close();
}

inline void log(std::string msg) { log(Level::INFO, msg); }

#endif // LOGGER_H
