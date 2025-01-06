#ifndef ACTION_H
#define ACTION_H

#include <optional>

#include "controller.h"
#include "view.h"

#include "spdlog/spdlog.h"

struct None {};

enum class ActionType {
    // Pass no values

    // Pass value
};

template <typename T> struct Action {
    const ActionType type;
    const T payload;
};

template <> struct Action<void> {
    const ActionType type;
};

template <typename T, typename U> 
[[maybe_unused]] constexpr std::optional<const U> parse_action(View *v, const Action<T> &action) {
    switch (action.type) {
        default:
            auto logger = spdlog::get("basic_logger");
            if (logger != nullptr) {
                logger->info("Unknown action called");
            }

            return {};
    }
}

#endif // ACTION_H

