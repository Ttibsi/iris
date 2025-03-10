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
