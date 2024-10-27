#include "gapvector.h"

#include <sstream>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Default Constructor", "[Constructors]") {
    auto gv = Gapvector();

    REQUIRE(gv.size() == 0);
    REQUIRE(gv.capacity() == 32);
    REQUIRE(gv.to_str() == "");
}

TEST_CASE("Length Constructor", "[Constructors]") {
    auto gv = Gapvector(64);

    REQUIRE(gv.size() == 0);
    REQUIRE(gv.capacity() == 64);
    REQUIRE(gv.to_str() == "");
}

TEST_CASE("String_view Constructor", "[Constructors]") {
    std::string s = "hello world";
    auto gv = Gapvector(s);

    REQUIRE(gv.size() == 11);
    REQUIRE(gv.capacity() == 11 + 8);
    REQUIRE(gv.to_str() == "hello world");
}

TEST_CASE("Range Constructor", "[Constructors]") {
    SECTION("Vec of chars") {
        std::vector<char> v1 = {'h', 'e', 'l', 'l', 'o'};
        auto gv1 = Gapvector(v1.begin(), v1.end());

        REQUIRE(gv1.size() == 5);
        REQUIRE(gv1.capacity() == 5 + 8);
        REQUIRE(gv1.to_str() == "hello");
    }
}

TEST_CASE("Copy Constructor", "[Constructors]") {
    std::string s = "hello world";
    auto gv = Gapvector(s);
    auto gv_copy {gv};

    REQUIRE(gv_copy.size() == 11);
    REQUIRE(gv_copy.capacity() == 11 + 8);
    REQUIRE(gv_copy.to_str() == "hello world");
}

TEST_CASE("Copy Assignment Constructor", "[Constructors]") {
    std::string s = "hello world";
    auto gv = Gapvector(s);
    auto gv_copy = gv;

    REQUIRE(gv_copy.size() == 11);
    REQUIRE(gv_copy.capacity() == 11 + 8);
    REQUIRE(gv_copy.to_str() == "hello world");
}

TEST_CASE("Move Constructor", "[Constructors]") {
    std::string s = "hello world";
    auto gv = Gapvector(s);
    auto gv_move = std::move(gv);

    REQUIRE(gv_move.size() == 11);
    REQUIRE(gv_move.capacity() == 11 + 8);
    REQUIRE(gv_move.to_str() == "hello world");
}

TEST_CASE("Move Assignment Constructor", "[Constructors]") {
    std::string s = "hello world";
    auto gv = Gapvector(s);
    auto gv_move = Gapvector();
    gv_move = std::move(gv);

    REQUIRE(gv_move.size() == 11);
    REQUIRE(gv_move.capacity() == 11 + 8);
    REQUIRE(gv_move.to_str() == "hello world");
}

TEST_CASE("Operator Print", "[Operator Overloads]") {
    std::string s = "hello world";
    auto gv = Gapvector(s);
    std::ostringstream oss;

    SECTION("initial") {
        std::string t = "[hello world        ]";
        oss << gv;
        REQUIRE(oss.str() == t);
    }

    SECTION("After push back") {
        gv.push_back('_');
        std::string t = "[hello world_       ]";
        oss << gv;
        REQUIRE(oss.str() == t);
    }
}

TEST_CASE("Operator Squacket", "[Operator Overloads]") {
    std::string s = "hello world";
    auto gv = Gapvector(s);

    REQUIRE(gv[0] == 'h');
    REQUIRE(gv[5] == ' ');
}

TEST_CASE("Operator Squacket (Const)", "[Operator Overloads]") {
    std::string s = "hello world";
    const auto gv = Gapvector(s);

    REQUIRE(gv[0] == 'h');
    REQUIRE(gv[5] == ' ');
}

TEST_CASE("At", "[Element Access]") {
    SECTION("Standard at") {
        std::string s = "hello world";
        auto gv = Gapvector(s);

        REQUIRE(gv.at(0) == 'h');
        REQUIRE(gv.at(5) == ' ');
        REQUIRE_THROWS_AS(gv.at(21), std::out_of_range);
    }

    SECTION("Check at a resize boundary") {
        auto gv = Gapvector();
        gv.insert(gv.begin(), "0123456789012345678901234567890");

        REQUIRE(gv.size() == 31);
        REQUIRE(gv.capacity() == 32);
        REQUIRE(gv.at(30) == '0');
        REQUIRE_THROWS_AS(gv.at(31), std::out_of_range);

        gv.push_back('!');
        REQUIRE(gv.size() == 32);
        REQUIRE(gv.capacity() == 64);
        REQUIRE(gv.at(31) == '!');
    }
}

TEST_CASE("At (Const)", "[Element Access]") {
    std::string s = "hello world";
    const auto gv = Gapvector(s);
    REQUIRE(gv.at(0) == 'h');
    REQUIRE(gv.at(5) == ' ');
    REQUIRE_THROWS_AS(gv.at(21), std::out_of_range);
}

TEST_CASE("Front", "[Element Access]") {
    auto gv = Gapvector();
    REQUIRE_THROWS_AS(gv.front(), std::out_of_range);

    std::string s = "hello world";
    gv.insert(gv.begin(), s);

    REQUIRE(gv.front() == 'h');
}

TEST_CASE("Front (Const)", "[Element Access]") {
    const auto gv = Gapvector();
    REQUIRE_THROWS_AS(gv.front(), std::out_of_range);

    std::string s = "hello world";
    const auto gv2 = Gapvector(s);

    REQUIRE(gv2.front() == 'h');
}

TEST_CASE("Back", "[Element Access]") {
    auto gv = Gapvector();
    REQUIRE_THROWS_AS(gv.front(), std::out_of_range);

    std::string s = "hello world";
    gv.insert(gv.begin(), s);

    REQUIRE(gv.back() == 'd');
}

TEST_CASE("Back (Const)", "[Element Access]") {
    std::string s = "hello world";
    const auto gv = Gapvector(s);

    REQUIRE(gv.back() == 'd');
}

TEST_CASE("ToStr", "[Element Access]") {
    std::string s = "hello world";
    auto gv = Gapvector(s);
    REQUIRE(gv.to_str() == s);

    gv.push_back('_');
    gv.push_back('_');
    gv.push_back('_');
    REQUIRE(gv.to_str() == "hello world___");
}

TEST_CASE("line", "[Element Access]") {
    SECTION("Newline at start and end") {
        std::string s = "lorem ipsum\r\ndolor sit amet\r\nfoo bar baz";
        auto gv = Gapvector(s);
        REQUIRE(gv.line(21) == "dolor sit amet");
    }

    SECTION("No newlines present") {
        std::string s = "dolor sit amet";
        auto gv = Gapvector(s);
        REQUIRE(gv.line(5) == "dolor sit amet");
    }

    SECTION("Newline just at start") {
        std::string s = "lorem ipsum\r\ndolor sit amet";
        auto gv = Gapvector(s);
        REQUIRE(gv.line(21) == "dolor sit amet");
    }

    SECTION("Newline just at end") {
        std::string s = "dolor sit amet\r\nlorem ipsum";
        auto gv = Gapvector(s);
        REQUIRE(gv.line(5) == "dolor sit amet");
    }

    SECTION("Get the line at a resize border") {
        auto gv = Gapvector();
        gv.insert(gv.begin(), "#include <iostream>\r\n\r\nint main");
        REQUIRE(gv.capacity() == 32);
        gv.push_back('(');
        REQUIRE(gv.capacity() == 64);  // Check it's been resized
        REQUIRE(gv.size() == 32);

        REQUIRE(gv.line(32) == "int main(");
    }
}

TEST_CASE("find_ith_char", "[Element Access]") {
    std::string s = "lorem ipsum\ndolor sit amet\nfoo bar baz";
    auto gv = Gapvector(s);

    REQUIRE(gv.find_ith_char('\n', 2) == 26);
    REQUIRE(gv.find_ith_char(' ', 1) == 5);
    REQUIRE(gv.find_ith_char('q', 1) == -1);
    REQUIRE(gv.find_ith_char(' ', 0) == 0);
}

TEST_CASE("Begin", "[Iterators]") {
    std::string s = "hello world";
    auto gv = Gapvector(s);

    REQUIRE(*gv.begin() == 'h');
    REQUIRE(*(gv.begin() + 3) == 'l');
}

// TODO: Check over this -- no cursor
TEST_CASE("End", "[Iterators]") {
    std::string s = "hello world";
    auto gv = Gapvector(s);

    REQUIRE(*(gv.end() - 9) == 'd');
}

TEST_CASE("Begin (Const)", "[Iterators]") {
    std::string s = "hello world";
    const auto gv = Gapvector(s);

    REQUIRE(*gv.begin() == 'h');
    REQUIRE(*(gv.begin() + 3) == 'l');
}

TEST_CASE("End (Const)", "[Iterators]") {
    std::string s = "hello world";
    const auto gv = Gapvector(s);

    REQUIRE(*(gv.end() - 9) == 'd');
}

TEST_CASE("Cbegin", "[Iterators]") {
    std::string s = "hello world";
    const auto gv = Gapvector(s);

    REQUIRE(*gv.cbegin() == 'h');
    REQUIRE(*(gv.cbegin() + 3) == 'l');
}

TEST_CASE("Cend", "[Iterators]") {
    std::string s = "hello world";
    const auto gv = Gapvector(s);

    REQUIRE(*(gv.cend() - 9) == 'd');
}

TEST_CASE("Rbegin", "[Iterators]") {
    std::string s = "hello world";
    auto gv = Gapvector(s);

    REQUIRE(*gv.rbegin() == 'd');
    REQUIRE(*(gv.rbegin() + 3) == 'o');
}

TEST_CASE("Rbegin (const)", "[Iterators]") {
    std::string s = "hello world";
    auto gv = Gapvector(s);

    REQUIRE(*gv.rbegin() == 'd');
    REQUIRE(*(gv.rbegin() + 3) == 'o');
}

TEST_CASE("Rend", "[Iterators]") {
    std::string s = "hello world";
    auto gv = Gapvector(s);

    REQUIRE(*(gv.rend() - 1) == 'h');
}

TEST_CASE("Rend (const)", "[Iterators]") {
    std::string s = "hello world";
    const auto gv = Gapvector(s);

    REQUIRE(*(gv.rend() - 1) == 'h');
}

TEST_CASE("Crbegin", "[Iterators]") {
    std::string s = "hello world";
    const auto gv = Gapvector(s);

    REQUIRE(*gv.cbegin() == 'h');
    REQUIRE(*(gv.cbegin() + 3) == 'l');
}

TEST_CASE("Crend", "[Iterators]") {
    std::string s = "hello world";
    const auto gv = Gapvector(s);

    REQUIRE(*(gv.crend() - 1) == 'h');
}

TEST_CASE("Empty", "[Capacity]") {
    auto gv = Gapvector();
    REQUIRE(gv.empty() == true);
    gv.push_back('!');
    REQUIRE(gv.empty() == false);
}

TEST_CASE("Size", "[Capacity]") {
    auto gv = Gapvector();
    REQUIRE(gv.size() == 0);
    gv.push_back('!');
    REQUIRE(gv.size() == 1);
}

TEST_CASE("Capacity", "[Capacity]") {
    auto gv = Gapvector();
    REQUIRE(gv.capacity() == 32);

    auto gv2 = Gapvector(64);
    REQUIRE(gv2.capacity() == 64);
}

TEST_CASE("Reserve", "[Capacity]") {
    auto gv = Gapvector(16);
    REQUIRE(gv.capacity() == 16);

    SECTION("Doesn't resize smaller than existing size") {
        gv.reserve(8);
        REQUIRE(gv.capacity() == 16);
    }

    SECTION("Larger") {
        gv.reserve(32);
        REQUIRE(gv.capacity() == 32);
    }
}

TEST_CASE("Line Count", "[Capacity]") {
    SECTION("Empty") {
        auto gv = Gapvector();
        REQUIRE(gv.line_count() == 0);
    }

    SECTION("No Newlines present") {
        std::string s = "hello world";
        auto gv = Gapvector(s);
        REQUIRE(gv.line_count() == 1);
    }

    SECTION("Many Newlines present") {
        std::string s = "hello world\nthis is some text\nanother newline";
        auto gv = Gapvector(s);
        REQUIRE(gv.line_count() == 3);
    }
}

TEST_CASE("Clear", "[Modifiers]") {
    std::string s = "hello world";
    auto gv = Gapvector(s);

    REQUIRE(gv.size() == 11);
    REQUIRE(gv.capacity() == 11 + 8);

    gv.clear();

    REQUIRE(gv.size() == 0);
    REQUIRE(gv.capacity() == 11 + 8);
}

TEST_CASE("Insert Single", "[Modifiers]") {
    std::string s = "hello world";
    auto gv = Gapvector(s);
    gv.insert(gv.begin() + 5, ',');

    REQUIRE(gv.to_str() == "hello, world");
}

TEST_CASE("Insert String", "[Modifiers]") {
    std::string s = "hello world";
    auto gv = Gapvector(s);
    gv.insert(gv.begin() + 5, " there");

    REQUIRE(gv.to_str() == "hello there world");
}

TEST_CASE("Erase Pos", "[Modifiers]") {
    std::string s = "hello world";
    auto gv = Gapvector(s);
    gv.erase(gv.begin() + 5);

    REQUIRE(gv.to_str() == "helloworld");
}

TEST_CASE("Erase Count", "[Modifiers]") {
    std::string s = "hello world";
    auto gv = Gapvector(s);
    gv.erase(gv.begin() + 5, 6);

    REQUIRE(gv.to_str() == "hello");
}

TEST_CASE("Push Back", "[Modifiers]") {
    std::string s = "hello world";
    auto gv = Gapvector(s);

    gv.push_back('_');
    gv.push_back('_');
    gv.push_back('_');
    REQUIRE(gv.to_str() == "hello world___");
}

TEST_CASE("Pop Back", "[Modifiers]") {
    std::string s = "hello world";
    auto gv = Gapvector(s);

    gv.pop_back();
    REQUIRE(gv.to_str() == "hello worl");
}

TEST_CASE("Resize", "[Modifiers]") {
    auto gv = Gapvector(16);
    REQUIRE(gv.capacity() == 16);

    SECTION("Smaller") {
        gv.resize(8);
        REQUIRE(gv.capacity() == 8);
    }

    SECTION("Larger") {
        gv.resize(32);
        REQUIRE(gv.capacity() == 32);
    }

    SECTION("Ensure the contents remains the same after resizing") {
        auto gv = Gapvector();
        gv.insert(gv.begin(), "#include <iostream>\r\n\r\nint main");
        REQUIRE(gv.size() == 31);
        REQUIRE(gv.capacity() == 32);

        gv.push_back('(');
        REQUIRE(gv.size() == 32);
        REQUIRE(gv.capacity() == 64);

        REQUIRE(gv.to_str() == "#include <iostream>\r\n\r\nint main(");
    }
}
