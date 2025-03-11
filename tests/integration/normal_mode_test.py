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
