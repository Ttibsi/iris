import time

from setup import setup
from setup import TmuxRunner


def test_suspend():
    with TmuxRunner("bash", "--norc") as r:
        r.press_and_enter("./build/src/iris t.txt")
        r.press("i")
        r.type_str("Hello world")
        r.press("Escape")

        r.press("^z")
        r.assert_text_missing("Hello")

        # Run something else
        r.press_and_enter("cat /usr/share/dict/words | less")
        time.sleep(0.1)
        r.press("q")
        time.sleep(0.1)

        r.press_and_enter('fg')
        r.press("Enter")
        time.sleep(0.1)
        r.await_text('Hello')

        r.iris_cmd("q")


@setup("tests/fixture/very_long_line.txt")
def test_resize(r: TmuxRunner):
    # Capture original statusbar content
    original_statusbar: list[str] = r.statusbar_parts()

    r.tmux.execute_command("split-window", "-hl", "5")
    time.sleep(0.5)

    split_statusbar: list[str] = r.statusbar_parts()
    assert original_statusbar == split_statusbar
