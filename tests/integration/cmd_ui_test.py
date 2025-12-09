from setup import setup
from setup import TmuxRunner


@setup()
def test_enter_command_mode(r: TmuxRunner):
    r.press(r.CMD_KEY)

    assert r.lines()[-1] == ";"
    assert r.await_statusbar_parts()[0] == "COMMAND"


@setup()
def test_add_char_to_cmd(r: TmuxRunner):
    r.press(r.CMD_KEY)
    r.type_str("ping")

    assert r.lines()[-1] == ";ping"
    assert r.await_statusbar_parts()[0] == "COMMAND"


@setup()
def test_remove_char_to_cmd(r: TmuxRunner):
    r.press(r.CMD_KEY)
    r.type_str("ping")
    r.press("BSpace")

    assert r.lines()[-1] == ";pin"
    assert r.await_statusbar_parts()[0] == "COMMAND"


@setup()
def test_submit_command(r: TmuxRunner):
    r.press(r.CMD_KEY)
    r.type_str("ping")
    r.press("Enter")

    assert r.lines()[-1] == "pong"
    assert r.await_statusbar_parts()[0] == "READ"


@setup()
def test_escape_cmd_mode(r: TmuxRunner):
    cur_pos = r.cursor_pos()
    r.press(r.CMD_KEY)
    r.type_str("ping")
    r.press("Escape")

    assert r.lines()[-1] == ""
    assert r.await_statusbar_parts()[0] == "READ"
    assert r.cursor_pos() == cur_pos


@setup()
def test_backspace_to_exit(r: TmuxRunner):
    r.press(r.CMD_KEY)
    r.type_str("ping")

    for _ in range(4):
        r.press("BSpace")

    assert r.await_statusbar_parts()[0] == "COMMAND"
    r.press("BSpace")
    assert r.await_statusbar_parts()[0] == "READ"
    r.await_cursor_pos(0, 3)


@setup()
def test_move_left_right(r: TmuxRunner):
    r.press(r.CMD_KEY)
    r.type_str("ping")
    r.await_cursor_pos(23, 5)

    r.press("Left")
    r.await_cursor_pos(23, 4)

    r.press("Left")
    r.await_cursor_pos(23, 3)

    r.press("Left")
    r.await_cursor_pos(23, 2)

    r.press("Left")
    r.await_cursor_pos(23, 1)

    # Don't go left of the semi
    r.press("Left")
    r.await_cursor_pos(23, 1)

    for _ in range(4):
        r.press("Right")
    r.await_cursor_pos(23, 5)

    # Cursor should not move after being at the end of the text
    r.press("Right")
    r.await_cursor_pos(23, 5)


@setup()
def test_move_left_and_insert(r: TmuxRunner):
    r.press(r.CMD_KEY)
    r.type_str("ping")
    r.await_cursor_pos(23, 5)

    for _ in range(4):
        r.press("Left")

    r.await_cursor_pos(23, 1)
    assert r.lines()[-1] == ";ping"

    r.press("!")
    r.await_cursor_pos(23, 2)
    assert r.lines()[-1] == ";!ping"


@setup("tests/fixture/test_file_1.txt")
def test_cursor_goes_back_after_cmd_enter(r: TmuxRunner):
    r.press("j")
    old_pos = r.cursor_pos()

    r.iris_cmd("ping")

    assert r.cursor_pos() == old_pos


@setup("tests/fixture/test_file_1.txt")
def test_overlay_window_clears_on_backspace(r: TmuxRunner):
    r.press(r.CMD_KEY)
    r.type_str("s|is|was|")
    ui_elem: list[str] = r.lines()[13:-2]
    assert "Search Results" in ui_elem[0]

    r.press("BSpace")
    assert r.lines()[-1] == ";s|is|was"
    ui_elem = r.lines()[13:-2]
    assert "~" in ui_elem[0]
