#include "controller.h"

#include <unistd.h>

#include <array>
#include <catch2/catch_test_macros.hpp>

// Helper function to simulate keypress by writing to a pipe
void simulate_keypress(int write_fd, char ch) {
    write(write_fd, &ch, 1);
}

TEST_CASE("parse_input", "[CONTROLLER]") {
    std::array<int, 2> pipe_fds;
    REQUIRE(pipe(pipe_fds.data()) == 0);

    int old_stdin = dup(STDIN_FILENO);
    REQUIRE(dup2(pipe_fds.at(0), STDIN_FILENO) != -1);

    SECTION("Call Quit") {
        simulate_keypress(pipe_fds.at(1), 'q');
        auto ret = parse_input();
        REQUIRE(ret == ParseInputRet::Break);
    }

    // cleanup
    dup2(old_stdin, STDIN_FILENO);
    close(old_stdin);
    close(pipe_fds.at(0));
}
