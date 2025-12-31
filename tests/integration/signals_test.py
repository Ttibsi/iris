from setup import setup
from setup import TmuxRunner


def test_suspend():
    with TmuxRunner("bash", "--norc") as r:
        r.press_and_enter("./build/src/iris t.txt")
        r.await_text("READ")
        r.press("i")
        r.type_str("hello world")
        r.press("Escape")

        r.press("^z")
        r.assert_text_missing("hello")

        # Run something else
        r.press_and_enter("cat /usr/share/dict/words | less")
        r.press("q")

        r.press_and_enter('fg')
        r.await_text('hello')

        r.iris_cmd("q")


@setup("tests/fixture/very_long_line.txt", width=100)
def test_resize(r: TmuxRunner):
    # Capture original statusbar content
    original_statusbar: list[str] = r.await_statusbar_parts()

    r.tmux.execute_command("split-window", "-hdl", "5")

    split_statusbar: list[str] = r.await_statusbar_parts()
    assert original_statusbar[0] == split_statusbar[0]
    assert original_statusbar[-1] == split_statusbar[-1]


def test_resize_while_suspended():
    dims = {"width": 100, "height": 24}
    with TmuxRunner("bash", "--norc", **dims) as r:
        # Open file
        r.press_and_enter("./build/src/iris tests/fixture/very_long_line.txt")
        r.await_text("READ")
        assert r.lines()[0].endswith("23\u00BB")

        # Background
        r.press("^z")
        r.assert_text_missing("READ")

        # Resize
        r.tmux.execute_command("split-window", "-hdl", "5")

        # Foreground
        r.press_and_enter("fg")
        r.await_text("READ")

        # Check
        assert r.lines()[0].endswith("67\u00BB")
