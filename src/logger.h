#ifndef LOGGER_H
#define LOGGER_H

#include <chrono>
#include <concepts>
#include <cstdlib>
#include <format>
#include <fstream>
#include <string>
#include <string_view>

using namespace std::chrono;

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

[[nodiscard]] inline const std::string formatted_time(const time_point<system_clock>& time) {
    const auto now = round<seconds>(time);
    return std::format("[{:%y-%m-%d %H:%M:%S}]", now);
}

template <typename T>
concept Streamable = requires(std::ostream& out, T in) {
    { out << in } -> std::convertible_to<std::ostream&>;
};

template <Streamable T = std::string_view>
inline void log(Level lvl, T msg) {
    if (std::getenv("RAWTERM_DEBUG") != nullptr) {
        return;
    }

    std::ofstream out;
    out.open(log_file, std::ios::app);
    out << formatted_time(system_clock::now()) << level_str(lvl) << " " << msg << "\n";
    out.close();
}

template <Streamable T = std::string_view>
inline void log(T msg) {
    log(Level::INFO, msg);
}

#endif  // LOGGER_H
