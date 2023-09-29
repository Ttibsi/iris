//////////////////////////////////////////////////////////////////////////////
// MIT License
//
// Copyright (c) 2023 Ttibsi
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//////////////////////////////////////////////////////////////////////////////
// Code source: https://github.com/Ttibsi/rawterm/blob/main/rawterm.h
// Version: v1.4.0
//////////////////////////////////////////////////////////////////////////////

#ifndef RAWTERM_H
#define RAWTERM_H

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

enum Mod {
    Alt_L,
    Arrow,
    Control,
    Delete,
    Escape,
    Function,
    Shift,
    Space,
};

struct Key {
    char code;
    std::vector<Mod> mod;
    std::string raw;
};

int enable_raw_mode();
void enter_alt_screen();
void exit_alt_screen();
Key process_keypress();

std::pair<int, int> get_term_size();
void clear_screen();

void move_cursor(int line, int col);
void save_cursor_position();
void load_cursor_position();

std::string bold(std::string s);
std::string italic(std::string s);
std::string underline(std::string s);
std::string blink(std::string s);
std::string inverse(std::string s);
std::string hidden(std::string s);
std::string strikethrough(std::string s);

#endif // RAWTERM_H

#ifdef RAWTERM_IMPLEMENTATION

// enable/disable raw mode
// https://viewsourcecode.org/snaptoken/kilo/02.enteringRawMode.html

struct termios orig;

void die(const char *s) {
    std::perror(s);
    std::exit(1);
}

// This doesn't need explicitly calling because of the atexit() call
void disable_raw_mode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig) == -1)
        die("tcsetattr");
}

int enable_raw_mode() {
    if (tcgetattr(STDIN_FILENO, &orig) == -1)
        die("tcgetattr");
    atexit(disable_raw_mode);

    struct termios raw = orig;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    // raw.c_lflag |= ~(CS8); // Disabled to allow term_size reading
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");

    return 0;
}

// Enter/leave alternate screen
// https://stackoverflow.com/a/12920036
// Will need to find another solution for windows (#ifdef WIN32)
void enter_alt_screen() { std::cout << "\0337\033[?47h\033[H"; }

void exit_alt_screen() { std::cout << "\033[2J\033[?47l\0338"; }

// Read user input
Key handle_escape(std::vector<std::string> substrings, std::string raw) {
    // substrings[0] is escape char
    // if substrings[1] is equal to a letter (upper or lowercase), it's left
    // alt+letter if substrings[1] is [, it's an arrow key (abcd after for udrl)
    // if substrings[2] is \x31 (1), it can be f5+
    // f1 \x1b\x4f\x50
    // f2 \x1b\x4f\x51
    // f3 \x1b\x4f\x52
    // f4 \x1b\x4f\x53
    // f5 \x1b\x5b\x31\x35\x7e
    // f6 \x1b\x5b\x31\x37\x7e
    // f7 \x1b\x5b\x31\x38\x7e
    // f8 \x1b\x5b\x31\x39\x7e
    // f9 \x1b\x5b\x32\x30\x7e
    // f10 \x1b\x5b\x32\x31\x7e
    // f11 \x1b\x5b\x32\x33\x7e
    // f12 \x1b\x5b\x32\x34\x7e
    // delete: \x1b\x5b\x33\x7e

    std::vector<std::string> asciiLetters;
    for (int i = 0x41; i <= 0x5A; ++i) {
        std::stringstream ss;
        ss << "\\x" << std::uppercase << std::hex << std::setw(2)
           << std::setfill('0') << i;
        asciiLetters.push_back(ss.str());
    }
    for (int i = 0x61; i <= 0x7A; ++i) {
        std::stringstream ss;
        ss << "\\x" << std::uppercase << std::hex << std::setw(2)
           << std::setfill('0') << i;
        asciiLetters.push_back(ss.str());
    }
    for (int i = 0x31; i <= 0x39; ++i) {
        std::stringstream ss;
        ss << "\\x" << std::uppercase << std::hex << std::setw(2)
           << std::setfill('0') << i;
        asciiLetters.push_back(ss.str());
    }

    if (std::find(asciiLetters.begin(), asciiLetters.end(), substrings[1]) !=
        asciiLetters.end()) {
        int placeholder;
        std::istringstream(substrings[1].substr(2)) >> std::hex >> placeholder;
        return { char(placeholder), { Mod::Alt_L }, raw };
    } else if (substrings[1] == "\\x5b") {
        // ARROWS
        if (substrings[2] == "\\x41")
            return { 'A', { Mod::Arrow }, raw }; // up
        if (substrings[2] == "\\x42")
            return { 'B', { Mod::Arrow }, raw }; // down
        if (substrings[2] == "\\x43")
            return { 'C', { Mod::Arrow }, raw }; // right
        if (substrings[2] == "\\x44")
            return { 'D', { Mod::Arrow }, raw }; // left
        if (substrings[2] == "\\x33")
            return { ' ', { Mod::Delete }, raw }; // delete

        // FUNCTIONS pt 2
        if (substrings[2] == "\\x31") {
            if (substrings[3] == "\\x35")
                return { '5', { Mod::Function }, raw }; // f5
            if (substrings[3] == "\\x37")
                return { '6', { Mod::Function }, raw }; // f6
            if (substrings[3] == "\\x38")
                return { '7', { Mod::Function }, raw }; // f7
            if (substrings[3] == "\\x39")
                return { '8', { Mod::Function }, raw }; // f8
        } else if (substrings[2] == "\\x32") {
            if (substrings[3] == "\\x30")
                return { '9', { Mod::Function }, raw }; // f9
            if (substrings[3] == "\\x31")
                return { '0', { Mod::Function }, raw }; // f10
            if (substrings[3] == "\\x33")
                return { '1', { Mod::Function }, raw }; // f11
            if (substrings[3] == "\\x34")
                return { '2', { Mod::Function }, raw }; // f12
        }
    } else if (substrings[1] == "\\x4f") {
        // FUNCTIONS pt 1
        if (substrings[2] == "\\x50")
            return { '1', { Mod::Function }, raw }; // f1
        if (substrings[2] == "\\x51")
            return { '2', { Mod::Function }, raw }; // f2
        if (substrings[2] == "\\x52")
            return { '3', { Mod::Function }, raw }; // f3
        if (substrings[2] == "\\x53")
            return { '4', { Mod::Function }, raw }; // f4
    }

    return { ' ', { Mod::Escape }, raw }; // esc
}
Key process_keypress() {
    Key k;
    char seq[32];

    int ret = read(STDIN_FILENO, seq, sizeof(seq));
    if (ret < 0) {
        std::cerr << "ERROR: something went wrong during reading user input: "
                  << std::strerror(errno) << std::endl;
        return k;
    }

    std::string code;
    for (int i = 0; i < ret; ++i) {
        char buffer[5];
        std::snprintf(buffer, sizeof(buffer), "\\x%02x",
                      static_cast<unsigned char>(seq[i]));
        code += buffer;
    }

    k.raw = code;

    std::vector<std::string> substrings;
    for (size_t i = 0; i < code.length(); i += 4) {
        std::string sub = code.substr(i, 4);
        substrings.push_back(sub);
    }

    // TODO: alt-gr, multiple modifier keys?
    // NOTE: enter/^m are the same entry
    // https://www.rapidtables.com/code/text/ascii-table.html
    if (substrings[0] == "\\x01") {
        k.code = 'a';
        k.mod.push_back(Mod::Control);
    } else if (substrings[0] == "\\x02") {
        k.code = 'b';
        k.mod.push_back(Mod::Control);
    } else if (substrings[0] == "\\x03") {
        k.code = 'c';
        k.mod.push_back(Mod::Control);
    } else if (substrings[0] == "\\x04") {
        k.code = 'd';
        k.mod.push_back(Mod::Control);
    } else if (substrings[0] == "\\x05") {
        k.code = 'e';
        k.mod.push_back(Mod::Control);
    } else if (substrings[0] == "\\x06") {
        k.code = 'f';
        k.mod.push_back(Mod::Control);
    } else if (substrings[0] == "\\x07") {
        k.code = 'g';
        k.mod.push_back(Mod::Control);
    } else if (substrings[0] == "\\x08") {
        k.code = 'h';
        k.mod.push_back(Mod::Control);
    } else if (substrings[0] == "\\x09") {
        k.code = 'i';
        k.mod.push_back(Mod::Control);
    } else if (substrings[0] == "\\x0a") {
        k.code = 'j';
        k.mod.push_back(Mod::Control);
    } else if (substrings[0] == "\\x0b") {
        k.code = 'k';
        k.mod.push_back(Mod::Control);
    } else if (substrings[0] == "\\x0c") {
        k.code = 'l';
        k.mod.push_back(Mod::Control);
    } else if (substrings[0] == "\\x0d") {
        k.code = 'm';
        k.mod.push_back(Mod::Control);
    } else if (substrings[0] == "\\x0e") {
        k.code = 'n';
        k.mod.push_back(Mod::Control);
    } else if (substrings[0] == "\\x0f") {
        k.code = 'o';
        k.mod.push_back(Mod::Control);
    } else if (substrings[0] == "\\x10") {
        k.code = 'p';
        k.mod.push_back(Mod::Control);
    } else if (substrings[0] == "\\x11") {
        k.code = 'q';
        k.mod.push_back(Mod::Control);
    } else if (substrings[0] == "\\x12") {
        k.code = 'r';
        k.mod.push_back(Mod::Control);
    } else if (substrings[0] == "\\x13") {
        k.code = 's';
        k.mod.push_back(Mod::Control);
    } else if (substrings[0] == "\\x14") {
        k.code = 't';
        k.mod.push_back(Mod::Control);
    } else if (substrings[0] == "\\x15") {
        k.code = 'u';
        k.mod.push_back(Mod::Control);
    } else if (substrings[0] == "\\x16") {
        k.code = 'v';
        k.mod.push_back(Mod::Control);
    } else if (substrings[0] == "\\x17") {
        k.code = 'w';
        k.mod.push_back(Mod::Control);
    } else if (substrings[0] == "\\x18") {
        k.code = 'x';
        k.mod.push_back(Mod::Control);
    } else if (substrings[0] == "\\x19") {
        k.code = 'y';
        k.mod.push_back(Mod::Control);
    } else if (substrings[0] == "\\x1a") {
        k.code = 'z';
        k.mod.push_back(Mod::Control);
    } else if (substrings[0] == "\\x1b") {
        // ESCAPE
        return handle_escape(substrings, code);
    } else if (substrings[0] == "\\x1c") {
        k.code = ' ';
    } else if (substrings[0] == "\\x1d") {
        k.code = ' ';
    } else if (substrings[0] == "\\x1e") {
        k.code = ' ';
    } else if (substrings[0] == "\\x1f") {
        k.code = ' ';
    } else if (substrings[0] == "\\x20") {
        k.code = ' ';
        k.mod.push_back(Mod::Space);
    } else if (substrings[0] == "\\x21") {
        k.code = '!';
    } else if (substrings[0] == "\\x22") {
        k.code = '"';
    } else if (substrings[0] == "\\x23") {
        k.code = '#';
    } else if (substrings[0] == "\\x24") {
        k.code = '$';
    } else if (substrings[0] == "\\x25") {
        k.code = '%';
    } else if (substrings[0] == "\\x26") {
        k.code = '&';
    } else if (substrings[0] == "\\x27") {
        k.code = '\'';
    } else if (substrings[0] == "\\x28") {
        k.code = '(';
    } else if (substrings[0] == "\\x29") {
        k.code = ')';
    } else if (substrings[0] == "\\x2a") {
        k.code = '*';
    } else if (substrings[0] == "\\x2b") {
        k.code = '+';
    } else if (substrings[0] == "\\x2c") {
        k.code = ',';
    } else if (substrings[0] == "\\x2d") {
        k.code = '-';
    } else if (substrings[0] == "\\x2e") {
        k.code = '.';
    } else if (substrings[0] == "\\x2f") {
        k.code = '/';
    } else if (substrings[0] == "\\x30") {
        // NUMBERS
        k.code = '0';
    } else if (substrings[0] == "\\x31") {
        k.code = '1';
    } else if (substrings[0] == "\\x32") {
        k.code = '2';
    } else if (substrings[0] == "\\x33") {
        k.code = '3';
    } else if (substrings[0] == "\\x34") {
        k.code = '4';
    } else if (substrings[0] == "\\x35") {
        k.code = '5';
    } else if (substrings[0] == "\\x36") {
        k.code = '6';
    } else if (substrings[0] == "\\x37") {
        k.code = '7';
    } else if (substrings[0] == "\\x38") {
        k.code = '8';
    } else if (substrings[0] == "\\x39") {
        k.code = '9';
    } else if (substrings[0] == "\\x3a") {
        k.code = ':';
    } else if (substrings[0] == "\\x3b") {
        k.code = ';';
    } else if (substrings[0] == "\\x3c") {
        k.code = '<';
    } else if (substrings[0] == "\\x3d") {
        k.code = '=';
    } else if (substrings[0] == "\\x3e") {
        k.code = '>';
    } else if (substrings[0] == "\\x3f") {
        k.code = '?';
    } else if (substrings[0] == "\\x40") {
        k.code = '@';
    } else if (substrings[0] == "\\x41") {
        // UPPERCASE LETTERS
        k.code = 'A';
        k.mod.push_back(Mod::Shift);
    } else if (substrings[0] == "\\x42") {
        k.code = 'B';
        k.mod.push_back(Mod::Shift);
    } else if (substrings[0] == "\\x43") {
        k.code = 'C';
        k.mod.push_back(Mod::Shift);
    } else if (substrings[0] == "\\x44") {
        k.code = 'D';
        k.mod.push_back(Mod::Shift);
    } else if (substrings[0] == "\\x45") {
        k.code = 'E';
        k.mod.push_back(Mod::Shift);
    } else if (substrings[0] == "\\x46") {
        k.code = 'F';
        k.mod.push_back(Mod::Shift);
    } else if (substrings[0] == "\\x47") {
        k.code = 'G';
        k.mod.push_back(Mod::Shift);
    } else if (substrings[0] == "\\x48") {
        k.code = 'H';
        k.mod.push_back(Mod::Shift);
    } else if (substrings[0] == "\\x49") {
        k.code = 'I';
        k.mod.push_back(Mod::Shift);
    } else if (substrings[0] == "\\x4a") {
        k.code = 'J';
        k.mod.push_back(Mod::Shift);
    } else if (substrings[0] == "\\x4b") {
        k.code = 'K';
        k.mod.push_back(Mod::Shift);
    } else if (substrings[0] == "\\x4c") {
        k.code = 'L';
        k.mod.push_back(Mod::Shift);
    } else if (substrings[0] == "\\x4d") {
        k.code = 'M';
        k.mod.push_back(Mod::Shift);
    } else if (substrings[0] == "\\x4e") {
        k.code = 'N';
        k.mod.push_back(Mod::Shift);
    } else if (substrings[0] == "\\x4f") {
        k.code = 'O';
        k.mod.push_back(Mod::Shift);
    } else if (substrings[0] == "\\x50") {
        k.code = 'P';
        k.mod.push_back(Mod::Shift);
    } else if (substrings[0] == "\\x51") {
        k.code = 'Q';
        k.mod.push_back(Mod::Shift);
    } else if (substrings[0] == "\\x52") {
        k.code = 'R';
        k.mod.push_back(Mod::Shift);
    } else if (substrings[0] == "\\x53") {
        k.code = 'S';
        k.mod.push_back(Mod::Shift);
    } else if (substrings[0] == "\\x54") {
        k.code = 'T';
        k.mod.push_back(Mod::Shift);
    } else if (substrings[0] == "\\x55") {
        k.code = 'U';
        k.mod.push_back(Mod::Shift);
    } else if (substrings[0] == "\\x56") {
        k.code = 'V';
        k.mod.push_back(Mod::Shift);
    } else if (substrings[0] == "\\x57") {
        k.code = 'W';
        k.mod.push_back(Mod::Shift);
    } else if (substrings[0] == "\\x58") {
        k.code = 'X';
        k.mod.push_back(Mod::Shift);
    } else if (substrings[0] == "\\x59") {
        k.code = 'Y';
        k.mod.push_back(Mod::Shift);
    } else if (substrings[0] == "\\x5a") {
        k.code = 'Z';
        k.mod.push_back(Mod::Shift);
    } else if (substrings[0] == "\\x5b") {
        k.code = '[';
    } else if (substrings[0] == "\\x5c") {
        k.code = '\\';
    } else if (substrings[0] == "\\x5d") {
        k.code = ']';
    } else if (substrings[0] == "\\x5e") {
        k.code = '^';
    } else if (substrings[0] == "\\x5f") {
        k.code = '_';
    } else if (substrings[0] == "\\x60") {
        k.code = '`';
    } else if (substrings[0] == "\\x61") {
        // LOWERCASE LETTERS
        k.code = 'a';
    } else if (substrings[0] == "\\x62") {
        k.code = 'b';
    } else if (substrings[0] == "\\x63") {
        k.code = 'c';
    } else if (substrings[0] == "\\x64") {
        k.code = 'd';
    } else if (substrings[0] == "\\x65") {
        k.code = 'e';
    } else if (substrings[0] == "\\x66") {
        k.code = 'f';
    } else if (substrings[0] == "\\x67") {
        k.code = 'g';
    } else if (substrings[0] == "\\x68") {
        k.code = 'h';
    } else if (substrings[0] == "\\x69") {
        k.code = 'i';
    } else if (substrings[0] == "\\x6a") {
        k.code = 'j';
    } else if (substrings[0] == "\\x6b") {
        k.code = 'k';
    } else if (substrings[0] == "\\x6c") {
        k.code = 'l';
    } else if (substrings[0] == "\\x6d") {
        k.code = 'm';
    } else if (substrings[0] == "\\x6e") {
        k.code = 'n';
    } else if (substrings[0] == "\\x6f") {
        k.code = 'o';
    } else if (substrings[0] == "\\x70") {
        k.code = 'p';
    } else if (substrings[0] == "\\x71") {
        k.code = 'q';
    } else if (substrings[0] == "\\x72") {
        k.code = 'r';
    } else if (substrings[0] == "\\x73") {
        k.code = 's';
    } else if (substrings[0] == "\\x74") {
        k.code = 't';
    } else if (substrings[0] == "\\x75") {
        k.code = 'u';
    } else if (substrings[0] == "\\x76") {
        k.code = 'v';
    } else if (substrings[0] == "\\x77") {
        k.code = 'w';
    } else if (substrings[0] == "\\x78") {
        k.code = 'x';
    } else if (substrings[0] == "\\x79") {
        k.code = 'y';
    } else if (substrings[0] == "\\x7a") {
        k.code = 'z';
    } else if (substrings[0] == "\\x7b") {
        k.code = '{';
    } else if (substrings[0] == "\\x7c") {
        k.code = '|';
    } else if (substrings[0] == "\\x7d") {
        k.code = '}';
    } else if (substrings[0] == "\\x7e") {
        k.code = '~';
    } else if (substrings[0] == "\\x7f") {
        // DELETE
        k.code = ' ';
    }

    return k;
}

std::pair<int, int> get_term_size() {
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    return std::make_pair(w.ws_row, w.ws_col);
}
void clear_screen() { std::cout << "\033[2J"; }

void move_cursor(int line, int col) {
    std::cout << "\033[" << std::to_string(line) << ";" << std::to_string(col)
              << "H" << std::flush;
}
void save_cursor_position() { std::cout << "\033[s" << std::flush; }
void load_cursor_position() { std::cout << "\033[u" << std::flush; }

// Text formatting
std::string bold(std::string s) { return "\033[1m" + s + "\033[22m"; }
std::string italic(std::string s) { return "\033[3m" + s + "\033[23m"; }
std::string underline(std::string s) { return "\033[4m" + s + "\033[24m"; }
std::string blink(std::string s) { return "\033[5m" + s + "\033[25m"; }
std::string inverse(std::string s) { return "\033[7m" + s + "\033[27m"; }
std::string hidden(std::string s) { return "\033[8m" + s + "\033[28m"; }
std::string strikethrough(std::string s) { return "\033[9m" + s + "\033[29m"; }

// Cursor positioning

#endif // RAWTERM_IMPLEMENTATION
