import hecate
from setup import get_statusbar_parts
from setup import setup


@setup()
def test_inserting_char_at_start_of_empty_file(r: hecate.Runner):
    r.press("i")
    r.await_text("WRITE")
    r.press("h")
    r.press("e")
    r.press("l")
    r.press("l")
    r.press("o")

    r.await_text("hello")
    lines = r.screenshot().split("\n")
    assert lines[0] == " 1\u2502hello"

    status_bar = get_statusbar_parts(lines)
    assert status_bar[2] == "1:6"


@setup("tests/fixture/test_file_1.txt")
def test_inserting_char_at_start_of_line(r: hecate.Runner):
    r.press("i")
    r.await_text("WRITE")
    r.press("h")
    r.press("e")
    r.press("l")
    r.press("l")
    r.press("o")

    lines = r.screenshot().split("\n")
    expected_text = "helloThis is some text"
    assert lines[0] == f" 1\u2502{expected_text}"

    status_bar = get_statusbar_parts(lines)
    assert status_bar[-1] == "1:6"


@setup("tests/fixture/test_file_1.txt")
def test_inserting_char_in_middle_of_line(r: hecate.Runner):
    for _ in range(5):
        r.press("l")

    r.press("i")
    r.press("v")

    lines = r.screenshot().split("\n")
    expected_text = "This vis some text"
    assert lines[0] == f" 1\u2502{expected_text}"

    status_bar: list[str] = get_statusbar_parts(lines)
    assert status_bar[3][2:] == "7"


# TODO: GDB to see if move-right works correctly.
# Not sure press("l") is going far enough here
@setup("tests/fixture/test_file_1.txt")
def test_inserting_char_end_of_line(r: hecate.Runner):
    with open("tests/fixture/test_file_1.txt") as f:
        contents = f.readlines()

    for _ in range(len(contents[0])):
        r.press("l")

    r.press("i")
    r.press("v")

    lines = r.screenshot().split("\n")
    expected_text = "This is some textv"
    assert lines[0] == f" 1\u2502{expected_text}"

    status_bar: list[str] = get_statusbar_parts(lines)
    assert status_bar[3][2:] == "19"


@setup("tests/fixture/test_file_1.txt")
def test_inserting_char_end_of_file(r: hecate.Runner):
    with open("tests/fixture/test_file_1.txt") as f:
        contents = f.readlines()

    for _ in range(len(contents)):
        r.press("j")

    for _ in range(len(contents[-1])):
        r.press("l")

    r.press("i")
    r.press("v")

    lines = r.screenshot().split("\n")
    last_line: int = len(contents) - 1

    assert lines[last_line][-1] == "v"
    assert "and another newlinev" in lines[last_line]

    status_bar = get_statusbar_parts(lines)
    assert status_bar[3] == f"{len(contents)}:{len(contents[-1]) + 1}"


@setup("tests/fixture/test_file_1.txt")
def test_backspace_char(r: hecate.Runner):
    for _ in range(5):
        r.press("l")

    r.press("i")
    r.press("BSpace")

    lines = r.screenshot().split("\n")

    expected_text = "Thisis some text"
    assert lines[0] == f" 1\u2502{expected_text}"

    status_bar = get_statusbar_parts(lines)
    assert status_bar[3][2] == "5"


@setup("tests/fixture/test_file_1.txt")
def test_backspace_newline(r: hecate.Runner):
    r.press("j")
    r.press("j")
    r.press("i")
    r.press("BSpace")

    lines = r.screenshot().split("\n")
    with open("tests/fixture/test_file_1.txt") as f:
        contents = f.readlines()

    line_1_clean: str = contents[1].rstrip().replace("\t", "    ")
    line_2_clean: str = contents[2].rstrip()
    assert lines[1] == f" 2\u2502{line_1_clean}{line_2_clean}"
    assert lines[len(contents) - 1] == "~"

    status_bar = get_statusbar_parts(lines)
    assert status_bar[3][0] == "2"

    # +1 because cursor should be on first char of "second" line
    assert status_bar[3][2:] == f"{len(line_1_clean) + 1}"


@setup("tests/fixture/test_file_1.txt")
def test_insert_newline(r: hecate.Runner):
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

    lines = r.screenshot().split("\n")
    assert lines[3] == " 4\u2502hello"
    assert lines[4] == "~"

    status_bar = get_statusbar_parts(lines)
    assert status_bar[3][0] == "4"
    assert status_bar[3][2:] == "6"


@setup()
def test_line_truncates_when_inserting_char(r: hecate.Runner):
    r.press("i")
    for idx in range(77):
        r.press(f"{idx % 10}")

    lines = r.screenshot().split("\n")
    assert len(lines[0]) == 80
    assert lines[0][-1] == "6"

    status_bar = get_statusbar_parts(lines)
    assert status_bar[2] == "1:78"

    r.press("_")

    lines = r.screenshot().split("\n")
    assert len(lines[0]) == 80
    assert lines[0][-1] == "\u00BB"

    status_bar = get_statusbar_parts(lines)
    assert status_bar[2] == "1:79"


@setup("tests/fixture/very_long_line.txt")
def test_inserting_char_into_truncated_line(r: hecate.Runner):
    r.press("i")
    r.press("_")

    lines = r.screenshot().split("\n")

    # Skipping line number
    assert lines[0][3] == "_"
    assert lines[0][4] == "0"
    assert lines[0][-1] == "\u00BB"


@setup("tests/fixture/very_long_line.txt")
def test_inserting_newline_into_truncated_line(r: hecate.Runner):
    for _ in range(10):
        r.press("l")

    r.press("i")
    r.press("Enter")

    lines = r.screenshot().split("\n")

    assert lines[0] == " 1\u25020123456789"
    assert lines[1][0:13] == " 2\u25020123456789"

    status_bar = get_statusbar_parts(lines)
    assert status_bar[3] == "2:1"
