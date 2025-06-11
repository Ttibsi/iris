import os

from setup import setup
from setup import TmuxRunner


@setup()
def test_quit_command(r: TmuxRunner):
    r.press(r.CMD_KEY)
    r.await_text("COMMAND", timeout=2)
    r.press("q")

    lines = r.lines()
    assert lines[-1] == ";q"

    r.press("Enter")
    r.await_exit()


@setup("tests/fixture/test_file_1.txt")
def test_quit_with_modified_buffer(r: TmuxRunner):
    r.press("x")
    r.iris_cmd("q")
    err_line: str = r.color_screenshot()[-1]
    assert "Unsaved changes. Use `;q!` to discard" in err_line
    assert "\x1b[49m" in err_line  # red text


@setup("tests/fixture/test_file_1.txt")
def test_force_quit_with_modified_buffer(r: TmuxRunner):
    r.press("x")
    r.iris_cmd("q!")
    r.await_exit()


@setup("tests/fixture/test_file_1.txt")
def test_multi_file_quit_command(r: TmuxRunner):
    r.type_str("tt")
    r.type_str("tn")
    r.assert_filename_in_statusbar("test_file_1.txt")

    # quit while modified
    r.press("x")
    r.iris_cmd("q")
    err_line: str = r.color_screenshot()[-1]
    assert "Unsaved changes. Use `;q!` to discard" in err_line
    assert "\x1b[49m" in err_line  # red text

    # force quit while modified
    r.iris_cmd("q!")
    r.assert_filename_in_statusbar("NO NAME")
    assert "|" not in r.lines()  # no tab bar


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
    assert lines[-1] == ";w"

    status_bar = r.await_statusbar_parts()
    assert status_bar[0] == "COMMAND"

    r.press("Enter")
    status_bar = r.await_statusbar_parts()
    assert status_bar[0] == "READ"

    # Check highlighting colour
    message_line: str = r.color_screenshot()[-1]
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


@setup()
def test_invalid_command(r: TmuxRunner):
    r.iris_cmd("error")

    message_line: str = r.color_screenshot()[-1]
    assert "Unknown command" in message_line
    assert "\x1b[38;2;255;0;0m" in message_line


@setup("tests/fixture/lorem_ipsum.txt")
def test_lineno_command(r: TmuxRunner):
    # Don't scroll
    r.iris_cmd("7")

    statusbar_parts: list[str] = r.await_statusbar_parts()
    assert statusbar_parts[-1] == "7:1"

    first_line: str = r.lines()[0]
    prefix: str = "  1\u2502"
    assert first_line[0:len(prefix)] == prefix

    # scroll view
    r.iris_cmd("77")

    statusbar_parts = r.await_statusbar_parts()
    assert statusbar_parts[-1] == "77:1"

    first_line = r.lines()[0]
    prefix = " 65\u2502"
    assert first_line[0:len(prefix)] == prefix


@setup("tests/fixture/lorem_ipsum.txt")
def test_lineno_command_exact_center(r: TmuxRunner):
    r.iris_cmd("11")

    statusbar_parts: list[str] = r.await_statusbar_parts()
    assert statusbar_parts[-1] == "11:1"

    first_line: str = r.lines()[0]
    prefix: str = "  1\u2502"
    assert first_line[0:len(prefix)] == prefix


@setup("tests/fixture/lorem_ipsum.txt")
def test_open_other_file(r: TmuxRunner):
    r.iris_cmd("e tests/fixture/test_file_1.txt")

    statusbar_parts: list[str] = r.await_statusbar_parts()
    assert statusbar_parts[-2] == "[2]"
    # can't check specific index as git branch is only sometimes here
    assert "...ts/fixture/test_file_1.txt" in statusbar_parts

    first_line: str = r.lines()[0]
    assert "lorem_ipsum.txt" in first_line
    assert "test_file_1.txt" in first_line

    assert r.cursor_pos() == (1, 3)
