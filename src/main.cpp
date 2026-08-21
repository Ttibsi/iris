#include <memory>
#include <print>
#include <string>
// To link stacktrace, we need `-lstdc++exp
#include <stacktrace>

#include <cli11/CLI11.hpp>
#include <rawterm/core.h>
#include <rawterm/cursor.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include "controller.h"
#include "flags.h"
#include "text_io.h"
#include "version.h"

// TODO: command to toggle line numbers
// TODO: detect if `;e` is opening an already-open file and switch to that buf instead
// NOTE: Maybe post a message in the command bar too
// TODO: A way of detecting if the file is already open in another iris instance

void exit_app() {
    rawterm::exit_alt_screen();
    rawterm::Cursor::cursor_block();
    rawterm::disable_raw_mode();
}

[[noreturn]] constexpr void handler() noexcept {
    exit_app();

    std::shared_ptr<spdlog::logger> err_log = spdlog::get("basic_logger");
    auto log_msg = [&](std::string_view sv) {
        std::println("{}", sv);
        err_log->error("{}", sv);
    };

    const std::string bt = std::to_string(std::stacktrace::current());

    const auto eptr = std::current_exception();
    if (eptr) {
        try {
            std::rethrow_exception(eptr);
        } catch (const std::exception& e) { log_msg(e.what()); }
    }

    log_msg(std::format("\n{}", bt));
    std::exit(-1);
}

int main(int argc, char* argv[]) {
    CLI::App app {"Iris text editor"};
    Flags flags;

    app.add_option("file", flags.file, "File to open");
    app.add_option("-l,--line", flags.lineno, "Set line number to start on");
    app.add_flag("-v,--version", flags.print_version, "Print version");
    app.add_flag("-r,--readonly", flags.readonly, "Force file to be open in readonly mode");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) { return app.exit(e); }

    if (flags.print_version) {
        std::println("{}", version());
        return 0;
    }

    try {
        auto logger = spdlog::basic_logger_mt("basic_logger", "iris.log");
    } catch (const spdlog::spdlog_ex& ex) { std::println("Log init failed: {}", ex.what()); }
    spdlog::set_pattern("[%H:%M:%S %z] [thread %t] [%l] %v");

    spdlog::get("basic_logger")->info("Iris startup");
    rawterm::enter_alt_screen();
    rawterm::enable_raw_mode();

    std::set_terminate(&handler);

    Controller c;
    c.create_view(flags);
    c.start_action_engine();

    exit_app();
    return 0;
}
