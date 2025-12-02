#ifndef ENUMERATE_H
#define ENUMERATE_H

#include <generator>
#include <span>
#include <tuple>

template <typename T>
using enum_generator_t = std::generator<std::tuple<std::size_t, T>>;

template <typename T>
enum_generator_t<T> enumerate(std::span<T> container, std::size_t start) {
    for (std::size_t idx = 0; idx < container.size(); ++idx) {
        co_yield std::make_tuple(idx + start, container[idx]);
    }
}

template <typename T>
enum_generator_t<T> enumerate(std::span<T> container) {
    return enumerate<T>(container, 0);
}

#endif  // ENUMERATE_H
