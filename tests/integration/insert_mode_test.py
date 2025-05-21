from setup import setup
from setup import TmuxRunner


@setup()
def test_inserting_char_at_start_of_empty_file(r: TmuxRunner):
    r.press("i")
    r.await_text("WRITE")
    r.press("h")
    r.press("e")
    r.press("l")
    r.press("l")
    r.press("o")

    r.await_text("hello")
    lines = r.lines()
    assert lines[0] == " 1\u2502hello"

    status_bar = r.statusbar_parts()
    assert status_bar[-1] == "1:6"


@setup("tests/fixture/test_file_1.txt")
def test_inserting_char_at_start_of_line(r: TmuxRunner):
    r.press("i")
    r.await_text("WRITE")
    r.press("h")
    r.press("e")
    r.press("l")
    r.press("l")
    r.press("o")

    lines = r.lines()
    expected_text = "helloThis is some text"
    assert lines[0] == f" 1\u2502{expected_text}"

    status_bar = r.statusbar_parts()
    assert status_bar[-1] == "1:6"


@setup("tests/fixture/test_file_1.txt")
def test_inserting_char_in_middle_of_line(r: TmuxRunner):
    for _ in range(5):
        r.press("l")

    r.press("i")
    r.press("v")

    lines = r.lines()
    expected_text = "This vis some text"
    assert lines[0] == f" 1\u2502{expected_text}"

    status_bar: list[str] = r.statusbar_parts()
    assert status_bar[5][2:] == "7"


# TODO: GDB to see if move-right works correctly.
# Not sure press("l") is going far enough here
@setup("tests/fixture/test_file_1.txt")
def test_inserting_char_end_of_line(r: TmuxRunner):
    with open("tests/fixture/test_file_1.txt") as f:
        contents = f.readlines()

    for _ in range(len(contents[0])):
        r.press("l")

    r.press("i")
    r.press("v")

    lines = r.lines()
    expected_text = "This is some textv"
    assert lines[0] == f" 1\u2502{expected_text}"

    status_bar: list[str] = r.statusbar_parts()
    assert status_bar[5][2:] == "19"


@setup("tests/fixture/test_file_1.txt")
def test_inserting_char_end_of_file(r: TmuxRunner):
    with open("tests/fixture/test_file_1.txt") as f:
        contents = f.readlines()

    for _ in range(len(contents)):
        r.press("j")

    for _ in range(len(contents[-1])):
        r.press("l")

    r.press("i")
    r.press("v")

    lines = r.lines()
    last_line: int = len(contents) - 1

    assert lines[last_line][-1] == "v"
    assert "and another newlinev" in lines[last_line]

    status_bar = r.statusbar_parts()
    assert status_bar[-1] == f"{len(contents)}:{len(contents[-1]) + 1}"


@setup("tests/fixture/test_file_1.txt")
def test_backspace_char(r: TmuxRunner):
    for _ in range(5):
        r.press("l")

    r.press("i")
    r.press("BSpace")

    lines = r.lines()

    expected_text = "Thisis some text"
    assert lines[0] == f" 1\u2502{expected_text}"

    status_bar = r.statusbar_parts()
    assert status_bar[5][2] == "5"


@setup("tests/fixture/test_file_1.txt")
def test_backspace_newline(r: TmuxRunner):
    r.press("j")
    r.press("j")
    r.press("i")
    r.press("BSpace")

    lines = r.color_screenshot()
    with open("tests/fixture/test_file_1.txt") as f:
        contents = f.readlines()

    line_1_clean: str = contents[1].rstrip().replace("\t", "    ")
    line_2_clean: str = contents[2].rstrip()
    highlight_line_num: str = "\x1b[38;2;255;221;51m 2\u2502\x1b[39m"
    full_line: str = f"{highlight_line_num}{line_1_clean}{line_2_clean}"

    assert lines[1] == full_line
    assert lines[len(contents) - 1] == "~"

    status_bar = r.statusbar_parts()
    assert status_bar[5][0] == "2"

    # +1 because cursor should be on first char of "second" line
    assert status_bar[5][2:] == f"{len(line_1_clean) + 1}"


@setup("tests/fixture/test_file_1.txt")
def test_insert_newline(r: TmuxRunner):
    r.press("j")
    r.press("j")

    with open("tests/fixture/test_file_1.txt") as f:
        contents = f.readlines()

    for _ in range(len(contents[-1])):
        r.press("l")

    r.press("i")
    r.await_text("WRITE")
    r.press("Enter")
    r.press("h")
    r.press("e")
    r.press("l")
    r.press("l")
    r.press("o")

    lines = r.lines()
    assert lines[3] == " 4\u2502hello"
    assert lines[4] == "~"

    status_bar = r.statusbar_parts()
    assert status_bar[5][0] == "4"
    assert status_bar[5][2:] == "6"


@setup()
def test_line_truncates_when_inserting_char(r: TmuxRunner):
    r.press("i")
    for idx in range(77):
        r.press(f"{idx % 10}")

    lines = r.lines()
    assert len(lines[0]) == 80
    assert lines[0][-1] == "6"

    status_bar = r.statusbar_parts()
    assert status_bar[4] == "1:78"

    r.press("_")

    lines = r.lines()
    assert len(lines[0]) == 80
    assert lines[0][-1] == "\u00BB"

    status_bar = r.statusbar_parts()
    assert status_bar[4] == "1:79"


@setup("tests/fixture/very_long_line.txt")
def test_inserting_char_into_truncated_line(r: TmuxRunner):
    r.press("i")
    r.press("_")

    lines = r.lines()

    # Skipping line number
    assert lines[0][3] == "_"
    assert lines[0][4] == "0"
    assert lines[0][-1] == "\u00BB"


@setup("tests/fixture/very_long_line.txt")
def test_inserting_newline_into_truncated_line(r: TmuxRunner):
    for _ in range(10):
        r.press("l")

    r.press("i")
    r.press("Enter")

    lines = r.lines()

    assert lines[0] == " 1\u25020123456789"
    assert lines[1][0:13] == " 2\u25020123456789"

    status_bar = r.statusbar_parts()
    assert status_bar[5] == "2:1"


@setup()
def test_newline_with_preceeding_whitespace(r: TmuxRunner):
    r.press("i")

    text: str = "hello world"
    for c in text:
        r.press(c)

    r.press("Escape")
    r.press("h")
    r.press("h")
    r.press("h")
    r.press("h")
    r.press("h")
    r.press("h")

    # Cursor should now be before the space
    assert r.statusbar_parts()[-1] == "1:6"

    r.press("i")
    r.press("Enter")

    assert r.lines()[0] == " 1\u2502hello"
    assert r.lines()[1] == " 2\u2502world"


@setup()
def test_press_tab_for_spaces(r: TmuxRunner):
    r.press("i")
    r.press("Tab")
    r.press("!")

    assert r.lines()[0] == " 1\u2502    !"
    assert r.statusbar_parts()[-1] == "1:6"
    assert r.statusbar_parts()[2] == "[X]"


@setup("tests/fixture/test_file_1.txt")
def test_backspace_does_nothing_at_first_char(r: TmuxRunner):
    r.press("i")
    initial: tuple[int, ...] = r.cursor_pos()

    r.press("BSpace")
    assert r.statusbar_parts()[-1] == "1:1"

    assert r.cursor_pos() == initial
