import time

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
        r.await_text('Hello')

        r.iris_cmd("q")
