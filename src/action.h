#ifndef ACTION_H
#define ACTION_H

#include <optional>

struct None {};

enum class ActionType {};

template <typename T>
struct Action {
    const ActionType type;
    const T payload;
};

template <>
struct Action<void> {
    const ActionType type;
};

template <typename T, typename U>
constexpr std::optional<const U> parse_action(const Action<T>&);

#endif  // ACTION_H
