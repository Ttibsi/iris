#include "controller.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Constructor", "[CONTROLLER]") {
    Controller c;

    REQUIRE(c.models.capacity() == 8);
}

TEST_CASE("set_mode", "[CONTROLLER]") {
    Controller c;

    c.set_mode(Mode::Write);
    REQUIRE(c.mode == Mode::Write);

    c.set_mode(Mode::Read);
    REQUIRE(c.mode == Mode::Read);
}

TEST_CASE("get_mode", "[CONTROLLER]") {
    Controller c;

    REQUIRE(c.get_mode() == "READ");
    c.set_mode(Mode::Write);
    REQUIRE(c.get_mode() == "WRITE");
}

TEST_CASE("create_view", "[CONTROLLER]") {
    SECTION("View with file") {
        Controller c;
        c.create_view("tests/fixture/test_file_1.txt");

        REQUIRE(c.models.size() == 1);
        REQUIRE(*c.models.at(0).buf.begin() == 'T');
    }

    SECTION("Empty view") {
        Controller c;
        c.create_view("");

        REQUIRE(c.models.size() == 1);
        REQUIRE(c.models.at(0).buf.size() == 0);
    }
}

TEST_CASE("start_action_engine", "[CONTROLLER]") {
    SKIP("Still need to write this test");
}

// #include <array>
// #include <unistd.h>
//
// // Helper function to simulate keypress by writing to a pipe
// void simulate_keypress(int write_fd, char ch) {
//     write(write_fd, &ch, 1);
// }
//
// TEST_CASE("parse_input", "[CONTROLLER]") {
//     std::array<int, 2> pipe_fds;
//     REQUIRE(pipe(pipe_fds.data()) == 0);
//
//     int old_stdin = dup(STDIN_FILENO);
//     REQUIRE(dup2(pipe_fds.at(0), STDIN_FILENO) != -1);
//
//     SECTION("Call Quit") {
//         simulate_keypress(pipe_fds.at(1), 'q');
//         auto ret = parse_input();
//         REQUIRE(ret == ParseInputRet::Break);
//     }
//
//     // cleanup
//     dup2(old_stdin, STDIN_FILENO);
//     close(old_stdin);
//     close(pipe_fds.at(0));
// }
