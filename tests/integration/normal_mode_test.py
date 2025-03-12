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

    statusbar: list[str] = r.statusbar_parts()
    assert statusbar[0] == "WRITE"
    assert statusbar[-1] == "1:2"


@setup("tests/fixture/test_file_1.txt")
def test_upper_a_key(r: TmuxRunner):
    r.press("A")

    with open("tests/fixture/test_file_1.txt") as f:
        line_1_len: int = len(f.readlines()[0])

    statusbar: list[str] = r.statusbar_parts()
    assert statusbar[-1] == f"1:{line_1_len}"


@setup("tests/fixture/test_file_1.txt")
def test_underscore_key(r: TmuxRunner):
    r.type_str("llllllllj")
    statusbar: list[str] = r.statusbar_parts()
    assert statusbar[-1] == "2:9"

    r.press("_")
    statusbar = r.statusbar_parts()
    assert statusbar[-1] == "2:5"

    r.press("k")
    r.press("_")

    statusbar = r.statusbar_parts()
    assert statusbar[-1] == "1:1"


@setup("tests/fixture/test_file_1.txt")
def test_dollar_key(r: TmuxRunner):
    r.press("$")

    with open("tests/fixture/test_file_1.txt") as f:
        line_1_len: int = len(f.readlines()[0])

    statusbar: list[str] = r.statusbar_parts()
    assert statusbar[-1] == f"1:{line_1_len}"


@setup("tests/fixture/test_file_1.txt")
def test_w_key(r: TmuxRunner):
    r.press("w")
    statusbar: list[str] = r.statusbar_parts()
    assert statusbar[-1] == "1:6"

    r.press("w")
    statusbar = r.statusbar_parts()
    assert statusbar[-1] == "1:9"

    r.press("w")
    statusbar = r.statusbar_parts()
    assert statusbar[-1] == "1:14"

    # press again should not move
    r.press("w")
    statusbar = r.statusbar_parts()
    assert statusbar[-1] == "1:14"


@setup("tests/fixture/test_file_1.txt")
def test_b_key(r: TmuxRunner):
    # start at end of line
    r.press("$")

    with open("tests/fixture/test_file_1.txt") as f:
        line_1_len: int = len(f.readlines()[0])

    statusbar: list[str] = r.statusbar_parts()
    assert statusbar[-1] == f"1:{line_1_len}"

    r.press("b")
    statusbar = r.statusbar_parts()
    assert statusbar[-1] == "1:12"

    r.press("b")
    statusbar = r.statusbar_parts()
    assert statusbar[-1] == "1:7"

    r.press("b")
    statusbar = r.statusbar_parts()
    assert statusbar[-1] == "1:4"

    r.press("b")
    statusbar = r.statusbar_parts()
    assert statusbar[-1] == "1:1"

    # After reaching the start of the line, we don't crash
    r.press("b")
    statusbar = r.statusbar_parts()
    assert statusbar[-1] == "1:1"


@setup("tests/fixture/test_file_1.txt")
def test_upper_o_key(r: TmuxRunner):
    r.press("O")

    lines: list[str] = r.lines()
    assert lines[0] == " 1\u2502"
    assert lines[1] == " 2\u2502This is some text"

    statusbar: list[str] = r.statusbar_parts()
    assert statusbar[0] == "WRITE"
    assert statusbar[-1] == "1:1"


@setup("tests/fixture/test_file_1.txt")
def test_o_key(r: TmuxRunner):
    r.press("o")

    lines: list[str] = r.lines()
    assert lines[0] == " 1\u2502This is some text"
    assert lines[1] == " 2\u2502"

    statusbar: list[str] = r.statusbar_parts()
    assert statusbar[0] == "WRITE"
    assert statusbar[-1] == "2:1"
