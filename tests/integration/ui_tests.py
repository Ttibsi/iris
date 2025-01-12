import hecate
from setup import get_statusbar_parts
from setup import setup


@setup()
def test_clean_startup(r: hecate.Runner):
    lines = r.screenshot().split("\n")

    assert len(lines) == 25  # TODO: Investigate why theres an extra line here
    assert str(1) in lines[0]

    for i in range(1, 22):
        assert "~" in lines[i]

    assert "1:1" in lines[22]


@setup("tests/fixture/test_file_1.txt")
def test_open_with_file(r: hecate.Runner):
    lines = r.screenshot().split("\n")

    with open("tests/fixture/test_file_1.txt") as file:
        content: list[str] = file.readlines()

    for idx, line in enumerate(content, start=1):
        expected = f" {idx}\u2502{line.rstrip().replace("\t", " " * 4)}"
        assert lines[idx - 1] == expected

    assert lines[len(content) + 3] == "~"

    status_bar = lines[22].split(" ")
    status_bar = [x for x in status_bar if x != ""]
    assert len(status_bar) == 4
    assert status_bar[1] == "tests/fixture/test_file_1.txt"


@setup("tests/fixture/very_long_line.txt")
def test_render_truncated_line(r: hecate.Runner):
    lines = r.screenshot().split("\n")
    assert lines[0][-1] == "\u00BB"
    assert lines[0][-2] == "5"


@setup("tests/fixture/lorem_ipsum.txt")
def test_move_cursor_vertically(r: hecate.Runner):
    status_bar = get_statusbar_parts(r.screenshot().split("\n"))
    assert status_bar[3] == "1:1"

    for _ in range(5):
        r.press("j")

    status_bar = get_statusbar_parts(r.screenshot().split("\n"))
    assert status_bar[3] == "6:1"

    r.press("k")
    r.press("k")

    status_bar = get_statusbar_parts(r.screenshot().split("\n"))
    assert status_bar[3] == "4:1"


@setup("tests/fixture/lorem_ipsum.txt")
def test_scroll_view_vertically(r: hecate.Runner):
    for _ in range(21):
        r.press("j")

    lines = r.screenshot().split("\n")
    status_bar = get_statusbar_parts(lines)
    assert status_bar[3] == "22:1"
    assert lines[21][4:15] == "consectetur"

    r.press("j")

    lines = r.screenshot().split("\n")
    status_bar = get_statusbar_parts(lines)
    # breakpoint()
    assert status_bar[3] == "23:1"
    assert lines[21][4:9] == "Morbi"
