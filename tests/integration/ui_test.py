from setup import setup
from setup import temp_named_file
from setup import TmuxRunner


@setup()
def test_clean_startup(r: TmuxRunner):
    lines = r.lines()

    assert len(lines) == 24
    assert str(1) in lines[0]

    for i in range(1, 22):
        assert "~" in lines[i]

    assert "1:1" in lines[22]


@setup("tests/fixture/test_file_1.txt", width=160)
def test_open_with_file(r: TmuxRunner):
    lines = r.lines()

    with open("tests/fixture/test_file_1.txt") as file:
        content: list[str] = file.readlines()

    for idx, line in enumerate(content, start=1):
        expected = f" {idx}\u2502{line.rstrip().replace("\t", " " * 4)}"
        assert lines[idx - 1] == expected

    assert lines[len(content) + 3] == "~"

    status_bar = r.await_statusbar_parts()
    assert len(status_bar) == 4
    assert status_bar[2] == "tests/fixture/test_file_1.txt"


@setup("tests/fixture/very_long_line.txt")
def test_render_truncated_line(r: TmuxRunner):
    lines = r.lines()
    assert lines[0][-1] == "\u00BB"
    assert lines[0][-2] == "5"


def test_render_truncated_filename_in_statusbar():
    file_name = f"This_is_a_{'really_' * 8}long_file_name.txt"

    with temp_named_file(file_name):
        dims = {"width": 100, "height": 24}
        with TmuxRunner("build/src/iris", file_name, **dims) as r:
            r.await_text("READ")
            r.assert_filename_in_statusbar("long_file_name.txt")


@setup("tests/fixture/lorem_ipsum.txt")
def test_move_cursor_vertically(r: TmuxRunner):
    status_bar = r.await_statusbar_parts()
    assert status_bar[-1] == "1:1"

    for _ in range(5):
        r.press("j")

    status_bar = r.await_statusbar_parts()
    assert status_bar[-1] == "6:1"

    r.press("k")
    r.press("k")

    status_bar = r.await_statusbar_parts()
    assert status_bar[-1] == "4:1"


@setup("tests/fixture/lorem_ipsum.txt", multi_file=True)
def test_multi_file_move_cursor_vertically(r: TmuxRunner):
    r.await_cursor_pos(1, 4)
    r.assert_inverted_text(r.await_tab_bar_parts()[1], "lorem_ipsum.txt")

    r.await_statusbar_parts()[-1] == "1:1"
    r.press("j")
    r.await_statusbar_parts()[-1] == "2:1"
    r.press("]")
    r.await_statusbar_parts()[-1] == "9:1"


@setup("tests/fixture/lorem_ipsum.txt")
def test_active_line_number_highlighted(r: TmuxRunner):
    assert r.SELECTED_LINE_ANSI in r.get_lineno(0)

    r.press("j")

    assert r.SELECTED_LINE_ANSI in r.get_lineno(1)
    assert "128;128;128" in r.get_lineno(0)


@setup("tests/fixture/lorem_ipsum.txt")
def test_scroll_view_vertically(r: TmuxRunner):
    for _ in range(21):
        r.press("j")

    lines = r.lines()
    status_bar = r.await_statusbar_parts()
    assert status_bar[-1] == "22:1"
    assert lines[21][4:15] == "consectetur"

    r.press("j")

    lines = r.lines()
    status_bar = r.await_statusbar_parts()
    assert status_bar[-1] == "23:1"
    assert lines[21][4:9] == "Morbi"


@setup("tests/fixture/lorem_ipsum.txt", multi_file=True)
def test_multi_file_scroll_cursor_vertically(r: TmuxRunner):
    r.await_cursor_pos(1, 4)
    r.assert_inverted_text(r.await_tab_bar_parts()[1], "lorem_ipsum.txt")

    r.type_str("]" * 3)
    r.await_statusbar_parts()[-1] == "32:1"
    r.await_cursor_pos(19, 4)

    r.type_str("[" * 3)
    r.await_cursor_pos(1, 4)
    r.await_statusbar_parts()[-1] == "1:1"


@setup("tests/fixture/lorem_ipsum.txt")
def test_cursor_clamping_when_moved(r: TmuxRunner):
    for _ in range(1, 8):
        r.press("j")

    for _ in range(1, 5):
        r.press("l")

    assert r.cursor_pos() == (7, 8)
    assert r.await_statusbar_parts()[-1] == "8:5"

    r.press("j")
    assert r.cursor_pos() == (8, 4)
    assert r.await_statusbar_parts()[-1] == "9:1"

    r.press("j")
    assert r.cursor_pos() == (9, 4)
    assert r.await_statusbar_parts()[-1] == "10:1"


def test_open_at_specific_line():
    file_name: str = "tests/fixture/lorem_ipsum.txt"
    with TmuxRunner("build/src/iris", file_name, "-l22") as r:
        r.await_text("READ")
        status_bar: list[str] = r.await_statusbar_parts()
        assert status_bar[-1] == "22:1"

        assert r.cursor_pos() == (12, 4)

        line_start: str = " 10\u2502Fusce accumsan quis dolor sed tempus"
        assert line_start in r.lines()[0]


def test_cli_line_overflowing():
    file_name: str = "tests/fixture/lorem_ipsum.txt"
    with TmuxRunner("build/src/iris", file_name, "-l1000") as r:
        r.await_text("READ")
        status_bar: list[str] = r.await_statusbar_parts()
        assert status_bar[-1] == "88:1"

        assert r.cursor_pos() == (12, 4)

        line_start: str = " 76\u2502vulputate posuere sagittis"
        assert line_start in r.lines()[0]


@setup("tests/fixture/read_only.txt")
def test_open_readonly_file(r: TmuxRunner):
    assert "[RO]" in r.await_statusbar_parts()


@setup("tests/fixture/test_file_1.txt", multi_file=True)
def test_multi_file_cursor_on_active_line(r: TmuxRunner):
    r.await_statusbar_parts()[-1] == "1:1"
    lineno: str = r.get_lineno(1)
    assert lineno[-1] == "1"
    assert r.SELECTED_LINE_ANSI in lineno

    r.press("j")
    lineno = r.get_lineno(1)
    assert lineno[-1] == "1"
    assert r.SELECTED_LINE_ANSI not in lineno

    assert r.get_lineno(2)[-1] == "2"
    assert r.SELECTED_LINE_ANSI in r.get_lineno(2)


@setup("tests/fixture/test_file_1.txt", multi_file=True)
def test_modified_marker_in_tab_bar(r: TmuxRunner):
    r.press("x")
    tab_bar = r.await_tab_bar_parts()
    assert tab_bar[0] == "temp_file.txt"
    r.assert_inverted_text(tab_bar[1], "test_file_1.txt*")
