import os

from setup import setup
from setup import TmuxRunner


@setup()
def test_quit_command(r: TmuxRunner):
    r.press(r.CMD_KEY)
    r.await_text("COMMAND", timeout=2)
    r.press("q")

    lines = r.lines()
    assert lines[-2] == ";q"  # There's an extra \n so the last line is empty

    r.press("Enter")
    r.await_exit()


@setup("tests/fixture/temp_file.txt")
def test_write_command(r: TmuxRunner):
    r.press("i")
    r.press("f")
    r.press("o")
    r.press("o")
    r.press(" ")
    r.press("b")
    r.press("a")
    r.press("r")

    r.press('Escape')

    r.press(r.CMD_KEY)
    r.press("w")
    lines = r.lines()
    assert lines[-2] == ";w"

    status_bar = r.statusbar_parts()
    assert status_bar[0] == "COMMAND"

    r.press("Enter")
    status_bar = r.statusbar_parts()
    assert status_bar[0] == "READ"

    # Check highlighting colour
    message_line: str = r.color_screenshot()[-2]
    assert "Saved" in message_line
    assert "bytes" in message_line
    assert "\x1b[38;2;0;128;0m" in message_line

    with open("tests/fixture/temp_file.txt") as f:
        text = f.read()
    assert text.split()[0] == "foo"


@setup("tests/fixture/does_not_exist.txt")
def test_write_to_new_file(r: TmuxRunner):
    r.press("i")
    r.press("f")
    r.press("o")
    r.press("o")
    r.press(" ")
    r.press("b")
    r.press("a")
    r.press("r")

    r.press('Escape')

    r.iris_cmd("w")
    r.iris_cmd("q")
    r.await_exit()

    with open("tests/fixture/does_not_exist.txt") as f:
        text = f.readlines()

    assert len(text) == 1
    assert text[0] == "foo bar\n"

    os.remove("tests/fixture/does_not_exist.txt")
