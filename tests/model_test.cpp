#include "model.h"

#include <catch2/catch_test_macros.hpp>

#include "action.h"
#include "change.h"
#include "text_io.h"
#include "view.h"

TEST_CASE("Constructor", "[model]") {
    auto m = Model(32, "");

    REQUIRE(m.filename == "");
    REQUIRE(m.buf.capacity() == 32);
}

TEST_CASE("Constructor with params", "[model]") {
    lines_t expected_buf = {"foo", "bar", "baz"};
    auto m = Model(expected_buf, "");

    REQUIRE(m.filename == "");
    REQUIRE(m.buf.at(0) == "foo");
}

TEST_CASE("backspace", "[model]") {
    SECTION("backspace a newline") {
        lines_t v = {"foo", "bar", "baz"};
        auto m = Model(v, "");
        m.current_line = 1;
        std::ignore = m.backspace();

        REQUIRE(m.buf.size() == 2);
        REQUIRE(m.buf.at(0) == "foobar");
    }

    SECTION("backspace a char") {
        lines_t v = {"foo", "bar", "baz"};
        auto m = Model(v, "");
        m.current_line = 1;
        m.current_char = 2;
        std::ignore = m.backspace();

        REQUIRE(m.buf.size() == 3);
        REQUIRE(m.buf.at(1) == "br");
        REQUIRE(m.buf.at(1).size() == 2);
    }

    SECTION("backspace the last char") {
        lines_t v = {"foo", "bar", "baz"};
        auto m = Model(v, "");
        m.current_line = 1;
        m.current_char = 3;
        std::ignore = m.backspace();

        REQUIRE(m.buf.size() == 3);
        REQUIRE(m.buf.at(1) == "ba");
        REQUIRE(m.buf.at(1).size() == 2);
    }
}

TEST_CASE("newline", "[model]") {
    SECTION("At end of line") {
        lines_t v = {"foo", "bar", "baz"};
        auto m = Model(v, "");

        m.current_char = 3;
        const std::size_t prev_line_len = m.newline();

        REQUIRE(prev_line_len == v.at(0).size());
        REQUIRE(m.buf.size() == 4);
        REQUIRE(m.buf.at(0).size() == 3);
        REQUIRE(m.buf.at(1).size() == 0);
    }

    SECTION("At mid of line") {
        lines_t v = {"foo", "bar", "baz"};
        auto m = Model(v, "");

        m.current_char = 1;
        const std::size_t prev_line_len = m.newline();

        REQUIRE(prev_line_len == 1);
        REQUIRE(m.buf.size() == 4);
        REQUIRE(m.buf.at(0).size() == 1);
        REQUIRE(m.buf.at(0) == "f");
        REQUIRE(m.buf.at(1).size() == 2);
        REQUIRE(m.buf.at(1) == "oo");
    }

    SECTION("At start of line") {
        lines_t v = {"foo", "bar", "baz"};
        auto m = Model(v, "");

        const std::size_t prev_line_len = m.newline();

        REQUIRE(prev_line_len == 0);
        REQUIRE(m.buf.size() == 4);
        REQUIRE(m.buf.at(0).size() == 0);
        REQUIRE(m.buf.at(0) == "");
        REQUIRE(m.buf.at(1).size() == v.at(0).size());
        REQUIRE(m.buf.at(1) == "foo");
    }

    SECTION("Remove whitespace from second line") {
        lines_t v = {"Some long text", "and another"};
        auto m = Model(v, "");
        m.current_char = 4;

        const std::size_t prev_line_len = m.newline();
        REQUIRE(prev_line_len == 4);
        REQUIRE(m.buf.at(0) == "Some");
        REQUIRE(m.buf.at(1) == "long text");
        REQUIRE(m.buf.at(2) == "and another");
    }
}

TEST_CASE("insert", "[model]") {
    SECTION("Insert at start of line") {
        lines_t v = {"foo", "bar", "baz"};
        auto m = Model(v, "");

        m.insert('x');
        REQUIRE(m.buf.at(0) == "xfoo");
        REQUIRE(m.buf.at(0).size() == 4);
        REQUIRE(m.buf.size() == 3);

        m.insert('y');
        REQUIRE(m.buf.at(0) == "xyfoo");
        REQUIRE(m.buf.at(0).size() == 5);
        REQUIRE(m.buf.size() == 3);
    }

    SECTION("Insert in mid of line") {
        lines_t v = {"foo", "bar", "baz"};
        auto m = Model(v, "");

        m.current_char++;
        m.insert('x');

        REQUIRE(m.buf.at(0) == "fxoo");
        REQUIRE(m.buf.at(0).size() == 4);
        REQUIRE(m.buf.size() == 3);
    }

    SECTION("Insert at end of line") {
        lines_t v = {"foo", "bar", "baz"};
        auto m = Model(v, "");

        m.current_char = static_cast<unsigned int>(v.at(0).size());
        m.insert('x');

        REQUIRE(m.buf.at(0) == "foox");
        REQUIRE(m.buf.at(0).size() == 4);
        REQUIRE(m.buf.size() == 3);
    }
}

TEST_CASE("lineno_in_scope", "[model]") {
    lines_t v = {"foo", "bar", "baz"};
    auto m = Model(v, "");
    REQUIRE(m.lineno_in_scope(2));
    REQUIRE_FALSE(m.lineno_in_scope(6));
}

TEST_CASE("next_word_pos", "[model]") {
    auto m = Model({"This is the first line", "std::foo();"}, "");

    REQUIRE(m.next_word_pos().has_value());
    REQUIRE(m.next_word_pos().value() == 5);
    m.current_char = 19;
    REQUIRE_FALSE(m.next_word_pos().has_value());

    m.current_line++;
    m.current_char = 1;

    REQUIRE(m.next_word_pos().has_value());
    REQUIRE(m.next_word_pos().value() == 4);

    // if at end of line, don't crash
    m.current_line = 0;
    m.current_char = static_cast<uint>(m.buf.at(0).size()) - 1;
    REQUIRE_FALSE(m.next_word_pos().has_value());
}

TEST_CASE("prev_word_pos", "[model]") {
    auto m = Model({"This is the first line", "std::foo();"}, "");

    m.current_char = 21;
    REQUIRE(m.prev_word_pos().has_value());
    REQUIRE(m.prev_word_pos().value() == 5);

    m.current_char -= 5;
    REQUIRE(m.prev_word_pos().has_value());
    REQUIRE(m.prev_word_pos().value() == 6);

    m.current_char -= 6;
    REQUIRE(m.prev_word_pos().has_value());
    REQUIRE(m.prev_word_pos().value() == 4);

    // If at start of line, don't crash
    m.current_char = 0;
    REQUIRE_FALSE(m.prev_word_pos().has_value());
}

TEST_CASE("next_para_pos", "[model]") {
    auto m = Model({"line one", "line two", "line three", "", "line four", "line five"}, "");

    auto opt = m.next_para_pos();
    REQUIRE(opt.has_value());
    REQUIRE(opt.value() == 3);

    m.current_line = 4;

    opt = m.next_para_pos();
    REQUIRE(opt.has_value());
    REQUIRE(opt.value() == 1);

    m.current_line = 5;
    opt = m.next_para_pos();
    REQUIRE_FALSE(opt.has_value());
}

TEST_CASE("prev_para_pos", "[model]") {
    auto m = Model({"line one", "line two", "line three", "", "line four", "line five"}, "");

    auto opt = m.prev_para_pos();
    REQUIRE_FALSE(opt.has_value());

    m.current_line = static_cast<unsigned int>(m.buf.size() - 1);

    opt = m.prev_para_pos();
    REQUIRE(opt.has_value());
    REQUIRE(opt.value() == 2);

    m.current_line = 2;
    opt = m.prev_para_pos();
    REQUIRE(opt.has_value());
    REQUIRE(opt.value() == 2);
}

TEST_CASE("toggle_case", "[model]") {
    auto m = Model({"line one", "line two", "line three", "", "line four", "line five"}, "");

    m.toggle_case();
    REQUIRE(m.buf.at(0).at(0) == 'L');

    m.current_char = 4;
    REQUIRE(m.buf.at(0).at(4) == ' ');
}

TEST_CASE("find_next", "[model]") {
    auto m = Model({"line one", "line two", "line three", "", "line four", "line five"}, "");

    auto ret = m.find_next('o');
    REQUIRE(ret.has_value());
    REQUIRE(ret.value().vertical == 0);
    REQUIRE(ret.value().horizontal == 5);

    m.current_line += static_cast<unsigned int>(ret.value().vertical);
    m.current_char += static_cast<unsigned int>(ret.value().horizontal);

    ret = m.find_next('o');
    REQUIRE(ret.has_value());
    REQUIRE(ret.value().vertical == 1);
    REQUIRE(ret.value().horizontal == 7);

    m.current_line += static_cast<unsigned int>(ret.value().vertical);
    m.current_char += static_cast<unsigned int>(ret.value().horizontal);

    ret = m.find_next('_');
    REQUIRE_FALSE(ret.has_value());
}

TEST_CASE("find_prev", "[model]") {
    auto m = Model({"line one", "line two", "line three", "", "line four", "line five"}, "");
    m.current_line = 5;
    m.current_char = 8;

    auto ret = m.find_prev('f');
    REQUIRE(ret.has_value());
    REQUIRE(ret.value().vertical == 0);
    REQUIRE(ret.value().horizontal == 5);

    m.current_line -= static_cast<unsigned int>(ret.value().vertical);
    m.current_char -= static_cast<unsigned int>(ret.value().horizontal);

    ret = m.find_prev('t');
    REQUIRE(ret.has_value());
    REQUIRE(ret.value().vertical == 3);
    REQUIRE(ret.value().horizontal == 5);

    m.current_line -= static_cast<unsigned int>(ret.value().vertical);
    m.current_char -= static_cast<unsigned int>(ret.value().horizontal);

    ret = m.find_prev('q');
    REQUIRE_FALSE(ret.has_value());
}

TEST_CASE("undo", "[model]") {
    auto m = Model({"line one", "line two", "line three", "", "line four", "line five"}, "");

    Controller c;
    auto v = View(&c, {24, 80});
    v.add_model(&m);

    SECTION("Backspace") {
        m.current_line = 1;
        m.current_char = 1;
        m.undo_stack.push_back(
            Change(ActionType::Backspace, m.get_current_char(), m.current_line, m.current_char));

        m.buf.at(m.current_line).erase(m.current_char, 1);
        REQUIRE(m.buf.at(m.current_line) == "lne two");

        REQUIRE(m.undo(&v));
        REQUIRE(m.buf.at(m.current_line) == "line two");
    };

    SECTION("DelCurrentChar") {
        m.current_line = 1;
        m.current_char = 2;

        char next_char = m.buf.at(m.current_line).at(m.current_char);
        m.undo_stack.push_back(
            Change(ActionType::DelCurrentChar, next_char, m.current_line, m.current_char));
        m.buf.at(m.current_line).erase(m.current_char, 1);
        REQUIRE(m.buf.at(m.current_line) == "lie two");

        REQUIRE(m.undo(&v));
        REQUIRE(m.buf.at(m.current_line) == "line two");
    };

    SECTION("Newline") {
        m.current_line = 1;
        m.current_char = 1;
        std::ignore = m.newline();
        REQUIRE(m.buf.at(m.current_line - 1) == "l");
        REQUIRE(m.buf.at(m.current_line) == "ine two");

        m.undo_stack.push_back(Change(ActionType::Newline, m.current_line, m.current_char));
        REQUIRE(m.undo(&v));

        m.current_line--;
        REQUIRE(m.buf.at(m.current_line) == "line two");
        REQUIRE(m.buf.size() == 6);
    };

    SECTION("ToggleCase") {
        m.current_line = 1;
        m.current_char = 1;
        m.toggle_case();
        REQUIRE(m.get_current_char() == 'I');

        m.undo_stack.push_back(Change(ActionType::ToggleCase, m.current_line, m.current_char));
        REQUIRE(m.undo(&v));
        REQUIRE(m.get_current_char() == 'i');
    };

    SECTION("InsertChar") {
        m.current_line = 1;
        m.current_char = 1;
        m.insert('?');
        REQUIRE(m.buf.at(m.current_line) == "l?ine two");

        m.undo_stack.push_back(Change(ActionType::InsertChar, m.current_line, m.current_char));
        REQUIRE(m.undo(&v));
        REQUIRE(m.buf.at(m.current_line) == "line two");
    };

    SECTION("ReplaceChar") {
        m.current_line = 1;
        m.current_char = 1;

        m.undo_stack.push_back(
            Change(ActionType::ReplaceChar, m.get_current_char(), m.current_line, m.current_char));

        m.replace_char('?');
        REQUIRE(m.buf.at(m.current_line) == "l?ne two");

        REQUIRE(m.undo(&v));
        REQUIRE(m.buf.at(m.current_line) == "line two");
    };
}

TEST_CASE("get_current_char", "[model]") {
    auto m = Model({"line one", "line two", "line three", "", "line four", "line five"}, "");
    REQUIRE(m.get_current_char() == 'l');

    m.current_line = 2;
    m.current_char = 4;
    REQUIRE(m.get_current_char() == ' ');
}
