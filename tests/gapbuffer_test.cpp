#include "gapbuffer.h"

#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Default Constructor", "[Constructors]") {
    auto buf = Gapbuffer();

    REQUIRE(buf.size() == 0);
    REQUIRE(buf.gap_size() == 32);
    REQUIRE(buf.capacity() == 32);
    REQUIRE(buf.to_str() == "");
}

TEST_CASE("Capacity Constructor", "[Constructors]") {
    SECTION("Larger size") {
        auto buf = Gapbuffer(64);

        REQUIRE(buf.size() == 0);
        REQUIRE(buf.gap_size() == 64);
        REQUIRE(buf.capacity() == 64);
        REQUIRE(buf.to_str() == "");
    }

    SECTION("Erroneous size") {
        REQUIRE_THROWS_AS(Gapbuffer(1), std::runtime_error);
    }
}

TEST_CASE("String_view Constructor", "[Constructors]") {
    std::string s = "hello world";

    SECTION("String with contents") {
        auto buf = Gapbuffer(s);
        REQUIRE(buf.size() == 11);
        REQUIRE(buf.gap_size() == 8);
        REQUIRE(buf.capacity() == 11 + 8);
        REQUIRE(buf.to_str() == "hello world");
    }

    SECTION("Empty string") {
        auto buf = Gapbuffer("");
        REQUIRE(buf.size() == 0);
        REQUIRE(buf.gap_size() == 8);
        REQUIRE(buf.capacity() == 8);
        REQUIRE(buf.to_str() == "");
    }
}

TEST_CASE("Range Constructor", "[Constructors]") {
    SECTION("Vec of chars") {
        std::vector<char> v1 = {'h', 'e', 'l', 'l', 'o'};
        auto buf = Gapbuffer(v1.begin(), v1.end());

        REQUIRE(buf.size() == 5);
        REQUIRE(buf.gap_size() == 8);
        REQUIRE(buf.capacity() == 5 + 8);
        REQUIRE(buf.to_str() == "hello");
    }
}

TEST_CASE("Initializer List Constructor", "[Constructors]") {
    Gapbuffer buf = {'h', 'e', 'l', 'l', 'o'};

    REQUIRE(buf.size() == 5);
    REQUIRE(buf.gap_size() == 8);
    REQUIRE(buf.capacity() == 5 + 8);
    REQUIRE(buf.to_str() == "hello");
}

TEST_CASE("Copy Constructor", "[Constructors]") {
    std::string s = "hello world";
    auto buf = Gapbuffer(s);
    auto buf_copy(buf);

    REQUIRE(buf_copy.size() == 11);
    REQUIRE(buf_copy.size() == buf.size());

    REQUIRE(buf_copy.gap_size() == 8);
    REQUIRE(buf_copy.gap_size() == buf.gap_size());

    REQUIRE(buf_copy.capacity() == 11 + 8);
    REQUIRE(buf_copy.capacity() == buf.capacity());

    REQUIRE(buf_copy.to_str() == "hello world");
    REQUIRE(buf_copy.to_str() == buf.to_str());
}

TEST_CASE("Copy Assignment Constructor", "[Constructors]") {
    std::string s = "hello world";
    auto buf = Gapbuffer(s);
    auto buf_copy = buf;

    REQUIRE(buf_copy.size() == 11);
    REQUIRE(buf_copy.size() == buf.size());

    REQUIRE(buf_copy.gap_size() == 8);
    REQUIRE(buf_copy.gap_size() == buf.gap_size());

    REQUIRE(buf_copy.capacity() == 11 + 8);
    REQUIRE(buf_copy.capacity() == buf.capacity());

    REQUIRE(buf_copy.to_str() == "hello world");
    REQUIRE(buf_copy.to_str() == buf.to_str());
}

TEST_CASE("Move Constructor", "[Constructors]") {
    std::string s = "hello world";
    auto buf = Gapbuffer(s);
    auto buf_move = std::move(buf);

    REQUIRE(buf_move.size() == 11);
    REQUIRE_FALSE(buf_move.size() == buf.size());

    REQUIRE(buf_move.gap_size() == 8);
    REQUIRE_FALSE(buf_move.gap_size() == buf.gap_size());

    REQUIRE(buf_move.capacity() == 11 + 8);
    REQUIRE_FALSE(buf_move.capacity() == buf.capacity());

    REQUIRE(buf_move.to_str() == "hello world");
    REQUIRE_FALSE(buf_move.to_str() == buf.to_str());
}

TEST_CASE("Move Assignment Constructor", "[Constructors]") {
    std::string s = "hello world";
    auto buf = Gapbuffer(s);
    auto buf_move = Gapbuffer();
    buf_move = std::move(buf);

    REQUIRE(buf_move.size() == 11);
    REQUIRE_FALSE(buf_move.size() == buf.size());

    REQUIRE(buf_move.gap_size() == 8);
    REQUIRE_FALSE(buf_move.gap_size() == buf.gap_size());

    REQUIRE(buf_move.capacity() == 11 + 8);
    REQUIRE_FALSE(buf_move.capacity() == buf.capacity());

    REQUIRE(buf_move.to_str() == "hello world");
    REQUIRE_FALSE(buf_move.to_str() == buf.to_str());
}

TEST_CASE("Destructor", "[Constructors]") {
    char* e;

    {
        auto buf = Gapbuffer("Hello");
        e = &buf.at(1);
    }  // Out of scope, gets destructed

    REQUIRE_FALSE(*e == 'e');
}

TEST_CASE("Operator Print", "[Operator Overloads]") {
    std::string s = "hello world";
    auto buf = Gapbuffer(s);
    std::ostringstream oss;

    SECTION("initial") {
        std::string t = "[hello world        ]";
        oss << buf;
        REQUIRE(oss.str() == t);
    }

    SECTION("After push back") {
        buf.push_back('_');
        std::string t = "[hello world_       ]";
        oss << buf;
        REQUIRE(oss.str() == t);
    }

    SECTION("After retreatiing") {
        buf.retreat();
        std::string t = "[hello worl        d]";
        oss << buf;
        REQUIRE(oss.str() == t);
    }
}

TEST_CASE("Operator Squacket", "[Operator Overloads]") {
    std::string s = "hello world";
    auto buf = Gapbuffer(s);

    REQUIRE(buf[0] == 'h');
    REQUIRE(buf[5] == ' ');
    REQUIRE_THROWS_AS(buf[12], std::out_of_range);
}

TEST_CASE("Operator Squacket (Const)", "[Operator Overloads]") {
    std::string s = "hello world";
    const auto buf = Gapbuffer(s);

    REQUIRE(buf[0] == 'h');
    REQUIRE(buf[5] == ' ');
    REQUIRE_THROWS_AS(buf[12], std::out_of_range);
}

TEST_CASE("Operator eq", "[Operator Overloads]") {
    auto buf1 = Gapbuffer("Hello world");
    auto buf2 = Gapbuffer("Hello world");
    auto buf3 = Gapbuffer();

    buf2.retreat();
    buf2.retreat();

    REQUIRE(buf1 == buf2);
    REQUIRE_FALSE(buf1 == buf3);

    buf2.pop_back();
    REQUIRE_FALSE(buf1 == buf2);
    buf2.push_back('!');
    REQUIRE_FALSE(buf1 == buf2);
}

TEST_CASE("Operator ne", "[Operator Overloads]") {
    auto buf1 = Gapbuffer("Hello world");
    auto buf2 = Gapbuffer("Hello world");
    auto buf3 = Gapbuffer();

    buf2.retreat();
    buf2.retreat();

    REQUIRE_FALSE(buf1 != buf2);
    REQUIRE(buf1 != buf3);

    buf2.pop_back();
    REQUIRE(buf1 != buf2);
    buf2.push_back('!');
    REQUIRE(buf1 != buf2);
}

TEST_CASE("At", "[Element Access]") {
    SECTION("Standard at") {
        std::string s = "hello world";
        auto buf = Gapbuffer(s);

        REQUIRE(buf.at(0) == 'h');
        REQUIRE(buf.at(5) == ' ');
        REQUIRE_THROWS_AS(buf.at(21), std::out_of_range);
    }

    SECTION("Check at a resize boundary") {
        auto buf = Gapbuffer();
        buf.insert("0123456789012345678901234567890");

        REQUIRE(buf.size() == 31);
        REQUIRE(buf.capacity() == 32);
        REQUIRE(buf.at(30) == '0');
        REQUIRE_THROWS_AS(buf.at(31), std::out_of_range);

        buf.push_back('!');
        REQUIRE(buf.size() == 32);
        REQUIRE(buf.capacity() == 64);
        REQUIRE(buf.at(31) == '!');
    }
}

TEST_CASE("At (Const)", "[Element Access]") {
    std::string s = "hello world";
    const auto buf = Gapbuffer(s);

    REQUIRE(buf.at(0) == 'h');
    REQUIRE(buf.at(5) == ' ');
    REQUIRE_THROWS_AS(buf.at(21), std::out_of_range);
}

TEST_CASE("Front", "[Element Access]") {
    auto buf = Gapbuffer();
    REQUIRE_THROWS_AS(buf.front(), std::out_of_range);

    buf.insert("hello world");

    REQUIRE(buf.front() == 'h');
}

TEST_CASE("Front (Const)", "[Element Access]") {
    const auto buf = Gapbuffer();
    REQUIRE_THROWS_AS(buf.front(), std::out_of_range);

    const auto buf2 = Gapbuffer("hello world");

    REQUIRE(buf2.front() == 'h');
}

TEST_CASE("Back", "[Element Access]") {
    auto buf = Gapbuffer();
    REQUIRE_THROWS_AS(buf.front(), std::out_of_range);

    buf.insert("Hello world");
    REQUIRE(buf.back() == 'd');
}

TEST_CASE("Back (Const)", "[Element Access]") {
    std::string s = "hello world";
    const auto buf = Gapbuffer(s);

    REQUIRE(buf.back() == 'd');
}

TEST_CASE("to_str", "[Element Access]") {
    std::string s = "hello world";
    auto buf = Gapbuffer(s);
    REQUIRE(buf.to_str() == s);

    buf.push_back('_');
    buf.push_back('_');
    buf.push_back('_');
    REQUIRE(buf.to_str() == "hello world___");
}

TEST_CASE("line", "[Element Access]") {
    SECTION("Newline at start and end") {
        std::string s = "lorem ipsum\r\ndolor sit amet\r\nfoo bar baz";
        auto buf = Gapbuffer(s);
        REQUIRE(buf.line(21) == "dolor sit amet\r\n");
        REQUIRE(buf.line(0) == "lorem ipsum\r\n");  // test passing 0
    }

    SECTION("No newlines present") {
        std::string s = "dolor sit amet";
        auto buf = Gapbuffer(s);
        REQUIRE(buf.line(5) == "dolor sit amet");
        REQUIRE(buf.line(0) == "dolor sit amet");
        REQUIRE(buf.line(s.size()) == "dolor sit amet");
    }

    SECTION("Newline just at start") {
        std::string s = "lorem ipsum\r\ndolor sit amet";
        auto buf = Gapbuffer(s);
        REQUIRE(buf.line(21) == "dolor sit amet");
    }

    SECTION("Newline just at end") {
        std::string s = "dolor sit amet\r\nlorem ipsum";
        auto buf = Gapbuffer(s);
        REQUIRE(buf.line(5) == "dolor sit amet\r\n");
    }

    SECTION("Get the line at a resize border") {
        auto buf = Gapbuffer();
        buf.insert("#include <iostream>\r\n\r\nint main");
        REQUIRE(buf.capacity() == 32);
        buf.push_back('(');
        REQUIRE(buf.capacity() == 64);  // Check it's been resized
        REQUIRE(buf.size() == 32);

        REQUIRE(buf.line(32) == "int main(");
    }
}

TEST_CASE("find", "[Element Access]") {
    std::string s = "lorem ipsum\r\ndolor sit amet\r\nfoo bar baz";
    auto buf = Gapbuffer(s);

    REQUIRE(buf.find('\n', 2) == 28);
    REQUIRE(buf.find(' ', 1) == 5);
    REQUIRE(buf.find('q', 1) == -1);
    REQUIRE(buf.find(' ', 0) == 0);
}

TEST_CASE("Begin", "[Iterators]") {
    std::string s = "hello world";
    auto buf = Gapbuffer(s);

    REQUIRE(*buf.begin() == 'h');
    REQUIRE(*(buf.begin() + 3) == 'l');
}

TEST_CASE("End", "[Iterators]") {
    std::string s = "hello world";
    auto buf = Gapbuffer(s);

    SECTION("Initial buffer") {
        REQUIRE(*(buf.end() - 9) == 'd');
        REQUIRE(*(buf.end() - 10) == 'l');
    }

    SECTION("Move gap") {
        buf.retreat();
        REQUIRE(*(buf.end() - 9) == 'd');
        REQUIRE(*(buf.end() - 10) == 'l');
    }
}

TEST_CASE("Begin (Const)", "[Iterators]") {
    std::string s = "hello world";
    const auto buf = Gapbuffer(s);

    REQUIRE(*buf.begin() == 'h');
    REQUIRE(*(buf.begin() + 3) == 'l');
}

TEST_CASE("End (const)", "[Iterators]") {
    std::string s = "hello world";
    const auto buf = Gapbuffer(s);

    SECTION("initial buffer") {
        REQUIRE(*(buf.end() - 9) == 'd');
        REQUIRE(*(buf.end() - 8) == 0);
    }
}

TEST_CASE("Cbegin", "[Iterators]") {
    std::string s = "hello world";
    const auto buf = Gapbuffer(s);

    REQUIRE(*buf.cbegin() == 'h');
    REQUIRE(*(buf.cbegin() + 3) == 'l');
}

TEST_CASE("Cend", "[Iterators]") {
    std::string s = "hello world";
    const auto buf = Gapbuffer(s);

    REQUIRE(*(buf.cend() - 9) == 'd');
}

TEST_CASE("Rbegin", "[Iterators]") {
    std::string s = "hello world";
    auto buf = Gapbuffer(s);

    REQUIRE(*buf.rbegin() == 'd');
    REQUIRE(*(buf.rbegin() + 3) == 'o');
}

TEST_CASE("Rend", "[Iterators]") {
    std::string s = "hello world";
    auto buf = Gapbuffer(s);

    REQUIRE(*(buf.rend() - 1) == 'h');
}

TEST_CASE("Rbegin (Const)", "[Iterators]") {
    std::string s = "hello world";
    const auto buf = Gapbuffer(s);

    REQUIRE(*buf.rbegin() == 'd');
    REQUIRE(*(buf.rbegin() + 3) == 'o');
}

TEST_CASE("Rend (Const)", "[Iterators]") {
    std::string s = "hello world";
    const auto buf = Gapbuffer(s);

    REQUIRE(*(buf.rend() - 1) == 'h');
}

TEST_CASE("Crbegin", "[Iterators]") {
    std::string s = "hello world";
    const auto buf = Gapbuffer(s);

    REQUIRE(*buf.cbegin() == 'h');
    REQUIRE(*(buf.cbegin() + 3) == 'l');
}

TEST_CASE("Crend", "[Iterators]") {
    std::string s = "hello world";
    const auto buf = Gapbuffer(s);

    REQUIRE(*(buf.crend() - 1) == 'h');
}

TEST_CASE("Empty", "[Capacity]") {
    auto buf = Gapbuffer();
    REQUIRE(buf.empty() == true);
    buf.push_back('!');
    REQUIRE(buf.empty() == false);
}

TEST_CASE("Size", "[Capacity]") {
    auto buf = Gapbuffer();
    REQUIRE(buf.size() == 0);
    buf.push_back('!');
    REQUIRE(buf.size() == 1);

    // Trigger resize
    buf.insert(std::string(buf.gap_size() + 1, '_'));
    REQUIRE(buf.size() == 33);
}

TEST_CASE("Gap Size", "[Capacity]") {
    auto buf = Gapbuffer();
    REQUIRE(buf.gap_size() == 32);
    buf.push_back('!');
    REQUIRE(buf.gap_size() == 31);

    // Trigger resize
    buf.insert(std::string(buf.gap_size() + 1, '_'));
    REQUIRE(buf.gap_size() == 31);
}

TEST_CASE("Capacity", "[Capacity]") {
    auto buf = Gapbuffer();
    REQUIRE(buf.capacity() == 32);

    // Trigger resize
    buf.insert(std::string(buf.gap_size() + 1, '_'));
    REQUIRE(buf.capacity() == 64);
}

TEST_CASE("Pos", "[Capacity]") {
    auto buf = Gapbuffer("Hello world!");
    REQUIRE(buf.pos() == 12);

    buf.retreat();
    buf.retreat();
    buf.retreat();
    buf.retreat();

    REQUIRE(buf.pos() == 8);
}

TEST_CASE("Reserve", "[Capacity]") {
    auto buf = Gapbuffer(16);
    REQUIRE(buf.capacity() == 16);

    SECTION("Doesn't resize smaller than existing size") {
        buf.reserve(8);
        REQUIRE(buf.capacity() == 16);
    }

    SECTION("Larger") {
        buf.reserve(32);
        REQUIRE(buf.capacity() == 32);
    }
}
TEST_CASE("Line Count", "[Capacity]") {
    SECTION("Empty") {
        auto buf = Gapbuffer();
        REQUIRE(buf.line_count() == 0);
    }

    SECTION("No Newlines present") {
        std::string s = "hello world";
        auto buf = Gapbuffer(s);
        REQUIRE(buf.line_count() == 1);
    }

    SECTION("Many Newlines present") {
        std::string s = "hello world\r\nthis is some text\r\nanother newline";
        auto buf = Gapbuffer(s);
        REQUIRE(buf.line_count() == 3);
    }
}

TEST_CASE("Clear", "[Modifiers]") {
    std::string s = "hello world";
    auto buf = Gapbuffer(s);

    REQUIRE(buf.size() == 11);
    REQUIRE(buf.gap_size() == 8);
    REQUIRE(buf.capacity() == 11 + 8);

    buf.clear();

    REQUIRE(buf.size() == 0);
    REQUIRE(buf.gap_size() == 19);
    REQUIRE(buf.capacity() == 11 + 8);
}

TEST_CASE("Insert", "[Modifiers]") {
    auto buf = Gapbuffer();
    buf.insert("#include <iostream>\r\n\r\n");

    REQUIRE(buf.size() == 23);
    REQUIRE(buf.gap_size() == 9);

    buf.insert("int main(");
    REQUIRE(buf.size() == 32);
    REQUIRE(buf.gap_size() == 32);
    REQUIRE(buf.capacity() == 64);
}

TEST_CASE("Erase", "[Modifiers]") {
    std::string s = "hello world";
    auto buf = Gapbuffer(s);
    buf.erase(6);

    REQUIRE(buf.to_str() == "hello");
    REQUIRE(buf.size() == 5);
    REQUIRE(buf.capacity() == 19);
}

TEST_CASE("Push Back", "[Modifiers]") {
    std::string s = "hello world";
    auto buf = Gapbuffer(s);

    buf.push_back('_');
    buf.push_back('_');
    buf.push_back('_');
    REQUIRE(buf.to_str() == "hello world___");
}

TEST_CASE("Pop Back", "[Modifiers]") {
    std::string s = "hello world";
    auto buf = Gapbuffer(s);

    buf.pop_back();
    REQUIRE(buf.to_str() == "hello worl");
}

TEST_CASE("Advance", "[Modifiers]") {
    SECTION("Space in gap") {
        std::string s = "hello world";
        auto buf = Gapbuffer(s);

        std::stringstream ss;

        ss << buf;
        REQUIRE(ss.str() == "[hello world        ]");

        buf.retreat();
        buf.retreat();
        buf.retreat();

        ss.str(std::string());
        ss << buf;
        REQUIRE(ss.str() == "[hello wo        rld]");

        buf.advance();
        buf.advance();

        ss.str(std::string());
        ss << buf;
        REQUIRE(ss.str() == "[hello worl        d]");
    }

    SECTION("Full buffer") {
        auto buf = Gapbuffer();
        buf.insert("#include <iostream>\r\n\r\nint main");
        REQUIRE(buf.capacity() == 32);

        std::stringstream ss;

        ss << buf;
        REQUIRE(ss.str() == "[#include <iostream>\r\n\r\nint main ]");

        buf.retreat();
        buf.retreat();
        buf.retreat();

        ss.str(std::string());
        ss << buf;
        REQUIRE(ss.str() == "[#include <iostream>\r\n\r\nint m ain]");

        buf.advance();
        buf.advance();

        ss.str(std::string());
        ss << buf;
        REQUIRE(ss.str() == "[#include <iostream>\r\n\r\nint mai n]");
    }
}

TEST_CASE("Retreat", "[Modifiers]") {
    SECTION("Space in gap") {
        std::string s = "hello world";
        auto buf = Gapbuffer(s);

        std::stringstream ss;

        ss << buf;
        REQUIRE(ss.str() == "[hello world        ]");

        buf.retreat();
        buf.retreat();
        buf.retreat();

        ss.str(std::string());
        ss << buf;
        REQUIRE(ss.str() == "[hello wo        rld]");
    }

    SECTION("Full buffer") {
        auto buf = Gapbuffer();
        buf.insert("#include <iostream>\r\n\r\nint main");
        REQUIRE(buf.capacity() == 32);

        std::stringstream ss;

        ss << buf;
        REQUIRE(ss.str() == "[#include <iostream>\r\n\r\nint main ]");

        buf.retreat();
        buf.retreat();
        buf.retreat();

        ss.str(std::string());
        ss << buf;
        REQUIRE(ss.str() == "[#include <iostream>\r\n\r\nint m ain]");
    }
}
