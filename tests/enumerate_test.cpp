#include "enumerate.h"

#include <vector>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("enumerate", "[enumerate]") {
    const std::vector<float> v = {1.1f, 3.4f, 17.2f, 8.9f, 0.0f, 5.1f, 1.1f};

    std::size_t expected_idx = 0;
    for (auto&& [idx, elem] : enumerate<const float>(v)) {
        // element matches the one at the same index
        REQUIRE(v.at(idx) == elem);

        // indices are produced in strict ascending order
        REQUIRE(idx == expected_idx);
        ++expected_idx;
    }
}

TEST_CASE("enumerate with start point", "[enumerate]") {
    std::vector<int> v = {1, 2, 3, 4, 5};
    std::size_t prev = 4;

    for (auto&& [idx, elem] : enumerate<int>(v, 5)) {
        REQUIRE(v.at(idx - 5) == elem);
        REQUIRE(++prev == idx);
    }
}
