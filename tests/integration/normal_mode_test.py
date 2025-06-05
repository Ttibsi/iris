import time

from setup import setup
from setup import TmuxRunner


@setup("tests/fixture/test_file_1.txt")
def test_x_key(r: TmuxRunner):
    r.press("x")

    with open("tests/fixture/test_file_1.txt") as f:
        pre_text = f.readlines()[0].strip()

    assert r.lines()[0] == f" 1\u2502{pre_text[1:]}"


@setup("tests/fixture/test_file_1.txt")
def test_a_key(r: TmuxRunner):
    r.press("a")

    statusbar: list[str] = r.await_statusbar_parts()
    assert statusbar[0] == "WRITE"
    assert statusbar[-1] == "1:2"


@setup("tests/fixture/test_file_1.txt")
def test_upper_a_key(r: TmuxRunner):
    r.press("A")

    with open("tests/fixture/test_file_1.txt") as f:
        line_1_len: int = len(f.readlines()[0])

    statusbar: list[str] = r.await_statusbar_parts()
    assert statusbar[-1] == f"1:{line_1_len}"


@setup("tests/fixture/test_file_1.txt")
def test_underscore_key(r: TmuxRunner):
    r.type_str("llllllllj")
    statusbar: list[str] = r.await_statusbar_parts()
    assert statusbar[-1] == "2:9"

    r.press("_")
    statusbar = r.await_statusbar_parts()
    assert statusbar[-1] == "2:5"

    r.press("k")
    r.press("_")

    statusbar = r.await_statusbar_parts()
    assert statusbar[-1] == "1:1"


@setup("tests/fixture/test_file_1.txt")
def test_dollar_key(r: TmuxRunner):
    r.press("$")

    with open("tests/fixture/test_file_1.txt") as f:
        line_1_len: int = len(f.readlines()[0])

    statusbar: list[str] = r.await_statusbar_parts()
    assert statusbar[-1] == f"1:{line_1_len}"


@setup("tests/fixture/test_file_1.txt")
def test_w_key(r: TmuxRunner):
    r.press("w")
    statusbar: list[str] = r.await_statusbar_parts()
    assert statusbar[-1] == "1:6"

    r.press("w")
    statusbar = r.await_statusbar_parts()
    assert statusbar[-1] == "1:9"

    r.press("w")
    statusbar = r.await_statusbar_parts()
    assert statusbar[-1] == "1:14"

    # press again should not move
    r.press("w")
    statusbar = r.await_statusbar_parts()
    assert statusbar[-1] == "1:14"


@setup("tests/fixture/test_file_1.txt")
def test_b_key(r: TmuxRunner):
    # start at end of line
    r.press("$")

    with open("tests/fixture/test_file_1.txt") as f:
        line_1_len: int = len(f.readlines()[0])

    statusbar: list[str] = r.await_statusbar_parts()
    assert statusbar[-1] == f"1:{line_1_len}"

    r.press("b")
    statusbar = r.await_statusbar_parts()
    assert statusbar[-1] == "1:12"

    r.press("b")
    statusbar = r.await_statusbar_parts()
    assert statusbar[-1] == "1:7"

    r.press("b")
    statusbar = r.await_statusbar_parts()
    assert statusbar[-1] == "1:4"

    r.press("b")
    statusbar = r.await_statusbar_parts()
    assert statusbar[-1] == "1:1"

    # After reaching the start of the line, we don't crash
    r.press("b")
    statusbar = r.await_statusbar_parts()
    assert statusbar[-1] == "1:1"


@setup("tests/fixture/test_file_1.txt")
def test_upper_o_key(r: TmuxRunner):
    r.press("O")

    lines: list[str] = r.lines()
    assert lines[0] == " 1\u2502"
    assert lines[1] == " 2\u2502This is some text"

    statusbar: list[str] = r.await_statusbar_parts()
    assert statusbar[0] == "WRITE"
    assert statusbar[-1] == "1:1"


@setup("tests/fixture/test_file_1.txt")
def test_o_key(r: TmuxRunner):
    r.press("o")

    lines: list[str] = r.lines()
    assert lines[0] == " 1\u2502This is some text"
    assert lines[1] == " 2\u2502"

    statusbar: list[str] = r.await_statusbar_parts()
    assert statusbar[0] == "WRITE"
    assert statusbar[-1] == "2:1"


@setup("tests/fixture/lorem_ipsum.txt")
def test_open_squacket_key(r: TmuxRunner):
    r.type_str("j" * 14)
    r.press("[")

    statusbar: list[str] = r.await_statusbar_parts()
    assert statusbar[-1] == "9:1"

    r.press("[")
    statusbar = r.await_statusbar_parts()
    assert statusbar[-1] == "1:1"

    # Don't move when we're at the top line
    r.press("[")
    statusbar = r.await_statusbar_parts()
    assert statusbar[-1] == "1:1"


@setup("tests/fixture/lorem_ipsum.txt")
def test_close_squacket_key(r: TmuxRunner):
    r.type_str("j" * 14)
    r.press("]")

    statusbar: list[str] = r.await_statusbar_parts()
    assert statusbar[-1] == "19:1"

    for _ in range(7):
        r.press("]")
    statusbar = r.await_statusbar_parts()
    assert statusbar[-1] == "80:1"

    # Go to end of file
    r.press("G")
    statusbar = r.await_statusbar_parts()
    assert statusbar[-1] == "88:1"

    r.press("]")
    statusbar = r.await_statusbar_parts()
    assert statusbar[-1] == "88:1"


@setup("tests/fixture/test_file_1.txt")
def test_r_key(r: TmuxRunner):
    r.press("r")
    r.press("#")

    assert r.lines()[0] == " 1\u2502#his is some text"

    # Empty line
    r.press("o")
    r.press("Escape")
    r.press("r")
    r.press("!")
    assert r.lines()[1] == " 2\u2502!"


@setup("tests/fixture/lorem_ipsum.txt")
def test_z_key(r: TmuxRunner):
    r.type_str("]" * 5)
    r.press("z")
    time.sleep(0.05)
    statusbar = r.await_statusbar_parts()
    assert statusbar[-1] == "48:1"
    assert r.cursor_pos() == (12, 4)


@setup("tests/fixture/lorem_ipsum.txt", multi_file=True)
def test_multi_file_z_key(r: TmuxRunner):
    r.type_str("]" * 5)
    r.press("z")
    time.sleep(0.05)
    statusbar = r.await_statusbar_parts()
    assert statusbar[-1] == "48:1"
    assert r.cursor_pos() == (13, 4)
    assert r.SELECTED_LINE_ANSI in r.color_screenshot()[13]


@setup("tests/fixture/lorem_ipsum.txt")
def test_upper_g_key(r: TmuxRunner):
    r.press("G")

    statusbar: list[str] = r.await_statusbar_parts()
    assert statusbar[-1] == "88:1"

    assert r.cursor_pos() == (21, 4)


@setup("tests/fixture/lorem_ipsum.txt")
def test_g_key(r: TmuxRunner):
    r.type_str("]]]]]")

    statusbar: list[str] = r.await_statusbar_parts()
    time.sleep(0.05)
    assert statusbar[-1] == "48:1"
    assert r.cursor_pos() == (18, 4)

    r.press("g")

    statusbar = r.await_statusbar_parts()
    assert statusbar[-1] == "1:1"
    assert r.cursor_pos() == (0, 4)


@setup("tests/fixture/test_file_1.txt")
def test_tilde_key(r: TmuxRunner):
    r.press("~")
    assert r.lines()[0][3] == "t"

    # Do nothing on a non-char character
    r.type_str("l" * 4)
    r.press("~")
    assert r.lines()[0][7] == " "


@setup("tests/fixture/test_file_1.txt")
def test_f_key(r: TmuxRunner):
    r.type_str("f ")
    statusbar: list[str] = r.await_statusbar_parts()
    assert statusbar[-1] == "1:5"

    # Search on next line
    r.type_str("fn")
    statusbar = r.await_statusbar_parts()
    assert statusbar[-1] == "2:15"


@setup("tests/fixture/test_file_1.txt")
def test_upper_f_key(r: TmuxRunner):
    r.type_str("j" * 2)
    r.press("$")
    statusbar: list[str] = r.await_statusbar_parts()
    assert statusbar[-1] == "3:20"

    r.type_str("Fw")
    statusbar = r.await_statusbar_parts()
    assert statusbar[-1] == "3:15"

    # Search backwards on prev line
    r.type_str("G_")
    r.type_str("Ft")
    statusbar = r.await_statusbar_parts()
    assert statusbar[-1] == "2:29"


@setup("tests/fixture/test_file_1.txt")
def test_undo_redo_backspace(r: TmuxRunner):
    r.type_str("l" * 6)
    r.press("i")
    r.press("BSpace")
    r.press("Escape")

    assert " is " not in r.lines()[0]

    r.press("u")
    assert r.lines()[0] == " 1\u2502This is some text"

    r.press("R")
    assert r.lines()[0] == " 1\u2502This s some text"


@setup("tests/fixture/test_file_1.txt")
def test_undo_redo_del_current_char(r: TmuxRunner):
    r.type_str("l" * 6)
    r.press("x")

    assert " is " not in r.lines()[0]

    r.press("u")
    assert r.lines()[0] == " 1\u2502This is some text"

    r.press("R")
    assert r.lines()[0] == " 1\u2502This i some text"


@setup("tests/fixture/test_file_1.txt")
def test_undo_redo_newline(r: TmuxRunner):
    r.type_str("l" * 7)
    r.press("i")
    r.press("Enter")
    r.press("Escape")

    assert r.await_statusbar_parts()[-1] == "2:1"
    lines: list[str] = r.lines()
    assert lines[0] == " 1\u2502This is"
    assert lines[1] == " 2\u2502some text"

    r.press("u")
    assert r.await_statusbar_parts()[-1] == "2:1"
    lines = r.lines()
    assert lines[0] == " 1\u2502This issome text"
    assert lines[1] == " 2\u2502    here is a newline and a tab"

    r.press("R")
    assert r.await_statusbar_parts()[-1] == "2:1"
    lines = r.lines()
    assert lines[0] == " 1\u2502This is"
    assert lines[1] == " 2\u2502some text"


@setup("tests/fixture/test_file_1.txt")
def test_undo_redo_toggle_case(r: TmuxRunner):
    r.press("l")
    r.press("~")

    line: str = r.lines()[0]
    assert "H" in line

    r.press("u")
    line = r.lines()[0]
    assert "H" not in line

    r.press("R")
    line = r.lines()[0]
    assert "H" in line


@setup("tests/fixture/test_file_1.txt")
def test_undo_redo_insert_char(r: TmuxRunner):
    r.type_str("ihello")
    r.press("Escape")

    line: str = r.lines()[0]
    assert line.startswith(" 1\u2502hello")

    r.press("u")
    line = r.lines()[0]
    assert line.startswith(" 1\u2502hellT")

    r.press("R")
    line = r.lines()[0]
    assert line.startswith(" 1\u2502hello")


@setup("tests/fixture/test_file_1.txt")
def test_undo_redo_replace_char(r: TmuxRunner):
    r.type_str("l" * 6)
    r.type_str("r!")

    line: str = r.lines()[0]
    assert " i! " in line

    r.press("u")
    line = r.lines()[0]
    assert " is " in line

    r.press("R")
    line = r.lines()[0]
    assert " i! " in line


@setup("tests/fixture/test_file_1.txt")
def test_upper_j_key(r: TmuxRunner):
    r.press("J")
    assert "here is a newline and a tab" in r.lines()[0]
    assert "This is some text" in r.lines()[1]


@setup("tests/fixture/test_file_1.txt")
def test_upper_k_key(r: TmuxRunner):
    r.press("G")
    r.press("K")
    assert "and another newline" in r.lines()[1]
    assert "here is a newline and a tab" in r.lines()[2]
