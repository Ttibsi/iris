#include "controller.h"

#include <catch2/catch_test_case_info.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Construction", "[controller]") {
    Controller c;
    REQUIRE(c.models.capacity() == 8);
}

TEST_CASE("set_mode", "[controller]") {
    Controller c;

    c.set_mode(Mode::Write);
    REQUIRE(c.mode == Mode::Write);

    c.set_mode(Mode::Read);
    REQUIRE(c.mode == Mode::Read);
}

TEST_CASE("get_mode", "[controller]") {
    Controller c;

    REQUIRE(c.get_mode() == "READ");
    c.set_mode(Mode::Write);
    REQUIRE(c.get_mode() == "WRITE");
}

TEST_CASE("create_view", "[controller]") {
    SECTION("View with file") {
        Controller c;
        c.create_view("tests/fixture/test_file_1.txt", 0);

        REQUIRE(c.models.size() == 1);
        REQUIRE(*c.models.at(0).buf.at(0).begin() == 'T');
    }

    SECTION("Empty view") {
        Controller c;
        c.create_view("", 0);

        REQUIRE(c.models.size() == 1);
        REQUIRE(c.models.at(0).buf.size() == 1);
    }

    SECTION("View with scroll offset") {
        Controller c;
        c.create_view("tests/fixture/lorem_ipsum.txt", 20);

        REQUIRE(c.models.size() == 1);
        REQUIRE(*c.models.at(0).buf.at(0).begin() == 'L');
    }
}

TEST_CASE("start_action_engine", "[controller]") {
    SKIP("TODO");
}

TEST_CASE("is_readonly_model", "[controller]") {
    Controller c;
    c.create_view("tests/fixture/test_file_1.txt", 0);
    REQUIRE(!c.is_readonly_model());
    c.view.get_active_model()->readonly = true;
    REQUIRE(c.is_readonly_model());
}

TEST_CASE("quit_app", "[controller]") {
    SECTION("Only one tab open") {
        Controller c;
        c.create_view("tests/fixture/test_file_1.txt", 0);

        REQUIRE(!c.quit_app(false));
        REQUIRE(c.quit_flag);
    }

    SECTION("Only one tab and modified buffer") {
        Controller c;
        c.create_view("tests/fixture/test_file_1.txt", 0);
        c.models.at(0).insert('!');

        REQUIRE(!c.quit_app(false));
        REQUIRE(!c.quit_flag);
    }

    SECTION("Multiple tabs open") {
        Controller c;
        c.create_view("tests/fixture/test_file_1.txt", 0);
        c.view.tab_new();

        REQUIRE(c.quit_app(false));
        REQUIRE(!c.quit_flag);
    }

    SECTION("Two tab and this buffer is modified") {
        Controller c;
        c.create_view("tests/fixture/test_file_1.txt", 0);
        c.view.tab_new();
        c.models.at(1).insert('!');

        REQUIRE(!c.quit_app(false));
        REQUIRE(!c.quit_flag);
    }

    SECTION("Two tab and other buffer is modified") {
        Controller c;
        c.create_view("tests/fixture/test_file_1.txt", 0);
        c.view.tab_new();
        c.models.at(0).insert('!');

        REQUIRE(c.quit_app(false));
        REQUIRE(!c.quit_flag);
    }
}

TEST_CASE("check_for_saved_file", "[controller]") {
    SECTION("Unsaved") {
        Controller c;
        c.create_view("tests/fixture/test_file_1.txt", 0);
        c.models.at(0).insert('!');
        REQUIRE(!c.check_for_saved_file(false));
    }

    SECTION("Saved") {
        Controller c;
        c.create_view("tests/fixture/test_file_1.txt", 0);

        REQUIRE(c.check_for_saved_file(false));
    }

    SECTION("Skip check") {
        Controller c;
        REQUIRE(c.check_for_saved_file(true));
    }
}

TEST_CASE("write_all", "[controller]") {
    Controller c;
    c.create_view("tests/fixture/test_file_1.txt", 0);
    c.view.tab_new();

    c.models.at(0).insert('!');
    REQUIRE(c.models.at(0).unsaved);

    WriteAllData write_all = c.write_all();
    REQUIRE(write_all.files == 1);
    REQUIRE(write_all.valid);
    REQUIRE(!c.models.at(0).unsaved);

    // Undo changes to not break other tests
    c.view.cursor_right();
    std::ignore = c.models.at(0).backspace();
    std::ignore = c.write_all();
}

TEST_CASE("quit_all", "[controller]") {
    Controller c;

    SECTION("saved models only") {
        c.create_view("tests/fixture/test_file_1.txt", 0);
        c.view.tab_new();
        c.add_model("tests/fixture/lorem_ipsum.txt");

        REQUIRE(c.models.size() == 3);

        bool redraw = c.quit_all();
        REQUIRE(!redraw);
        REQUIRE(c.models.empty());
        REQUIRE(c.view.view_models.empty());
    }

    SECTION("unsaved model") {
        c.create_view("tests/fixture/test_file_1.txt", 0);
        c.view.tab_new();
        c.add_model("tests/fixture/lorem_ipsum.txt");

        // Make second model unsaved
        c.models.at(2).insert('!');
        REQUIRE(c.models.at(2).unsaved);

        bool redraw = c.quit_all();
        REQUIRE(redraw);
        REQUIRE(c.models.size() == 1);
        REQUIRE(c.view.active_model == 0);

        // Clean up - save the unsaved model and quit again
        c.view.cursor_right();
        std::ignore = c.models.at(0).backspace();
        std::ignore = c.write_all();
        std::ignore = c.quit_all();
    }
}
