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
    SECTION("No unsaved models") {
        Controller c;
        c.create_view("tests/fixture/test_file_1.txt", 0);
        c.view.tab_new();
        c.add_model("tests/fixture/lorem_ipsum.txt");

        REQUIRE(c.models.size() == 3);

        auto ret = c.quit_all();
        REQUIRE(ret == QuitAll::Close);
    }

    SECTION("Unsaved models") {
        Controller c;
        c.create_view("tests/fixture/test_file_1.txt", 0);
        c.view.tab_new();
        c.add_model("tests/fixture/lorem_ipsum.txt");

        REQUIRE(c.models.size() == 3);

        c.models.at(2).insert('!');
        REQUIRE(c.models.at(2).unsaved);

        auto ret = c.quit_all();
        REQUIRE(ret == QuitAll::Redraw);
        REQUIRE(c.models.size() == 1);
    }
}

TEST_CASE("display_all_buffers", "[controller]") {
    Controller c;
    c.create_view("tests/fixture/test_file_1.txt", 0);
    c.view.tab_new();
    c.add_model("tests/fixture/lorem_ipsum.txt");

    REQUIRE(c.display_all_buffers());

    REQUIRE(c.meta_buffers.size() == 1);
    Model* buf_list = &c.meta_buffers.at(0);

    REQUIRE(buf_list->type == ModelType::META);
    REQUIRE(buf_list->filename == "[BUFFERS]");
    REQUIRE(buf_list->readonly);

    REQUIRE(buf_list->buf.size() == 7);

    REQUIRE(c.view.get_active_model() == buf_list);
}
