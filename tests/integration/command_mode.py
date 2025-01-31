import hecate
from setup import CMD_KEY
from setup import enter_cmd
from setup import get_statusbar_parts
from setup import setup


@setup()
def test_quit_command(r: hecate.Runner):
    r.press(CMD_KEY)
    r.await_text("COMMAND", timeout=2)
    r.press("q")

    lines = r.screenshot().split("\n")
    assert lines[-2] == ";q"  # There's an extra \n so the last line is empty

    r.press("Enter")
    r.await_exit()


@setup("tests/fixture/temp_file.txt")
def test_write_command(r: hecate.Runner):
    r.press("i")
    r.press("f")
    r.press("o")
    r.press("o")
    r.press(" ")
    r.press("b")
    r.press("a")
    r.press("r")

    r.press('Escape')

    r.press(CMD_KEY)
    r.press("w")
    lines = r.screenshot().split("\n")
    assert lines[-2] == ";w"

    status_bar = get_statusbar_parts(lines)
    assert status_bar[0] == "COMMAND"

    r.press("Enter")
    lines = r.screenshot().split("\n")
    status_bar = get_statusbar_parts(lines)
    assert status_bar[0] == "READ"

    with open("tests/fixture/temp_file.txt") as f:
        text = f.read()
    assert text.split()[0] == "foo"


@setup("tests/fixture/does_not_exist.txt")
def test_write_to_new_file(r: hecate.Runner):
    r.press("i")
    r.press("f")
    r.press("o")
    r.press("o")
    r.press(" ")
    r.press("b")
    r.press("a")
    r.press("r")

    r.press('Escape')

    enter_cmd(r, "w")
    enter_cmd(r, "q")

    r.await_exit()

    with open("tests/fixture/does_not_exist.txt") as f:
        text = f.readlines()

    assert len(text) == 1
    assert text[0] == "foo bar\n"
