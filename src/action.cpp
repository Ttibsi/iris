
#include "action.h"

template <typename T, typename U>
constexpr std::optional<const U> parse_action(const Action<T>& action) {
    switch (action.type) {}
}
