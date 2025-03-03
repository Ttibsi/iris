import contextlib
import os
import time
from collections.abc import Callable
from typing import Final
from typing import TypeVar

from hecate import Runner


T = TypeVar("T", bound=Callable[[Runner], None])


class TmuxRunner(Runner):
    CMD_KEY: Final[str] = "\\;"

    def __init__(self, *args, **kwargs) -> None:
        super().__init__(*args, **kwargs)

    def lines(self) -> list[str]:
        return super().screenshot().split("\n")

    # idea stolen from asottile/babi - `testing/runner.py`
    def color_screenshot(self) -> list[str]:
        return self.tmux.execute_command('capture-pane', '-ept0').split("\n")

    def statusbar_parts(self, index: int = 22) -> list[str]:
        return [part for part in self.lines()[index].split(" ") if part != ""]

    def type_str(self, msg: str) -> None:
        for c in msg:
            self.press(c)

    def press_and_enter(self, s: str) -> None:
        self.type_str(s)
        time.sleep(0.1)
        self.press("Enter")

    def iris_cmd(self, cmd: str) -> None:
        super().press(self.CMD_KEY)
        super().await_text("COMMAND", timeout=2)
        self.press_and_enter(cmd)

    def cursor_pos(self) -> tuple[int, ...]:
        return tuple(
            map(
                int,
                self.tmux.execute_command(
                    "display-message",
                    "-p",
                    "'#{cursor_y},#{cursor_x}'",
                ).rstrip()
                .replace("'", "")
                .split(","),
            ),
        )

    def assert_text_missing(self, text: str, wait: float = 0.1) -> None:
        time.sleep(wait)

        for line in self.lines():
            if text in line:
                raise AssertionError(f"Text: '{text}' found")


def setup(open_with: str = "") -> Callable[[T], Callable[[], None]]:
    temp_file: str = "tests/fixture/temp_file.txt"

    def decorator(func: T) -> Callable[[], None]:
        def wrapper() -> None:
            if os.path.isfile(temp_file):
                os.remove(temp_file)

            with open(temp_file, "w") as f:
                f.write("Hello world")

            dims = {"width": 80, "height": 24}
            with TmuxRunner("build/src/iris", open_with, **dims) as r:
                r.await_text("READ", timeout=2)
                func(r)

        return wrapper
    return decorator


@contextlib.contextmanager
def temp_named_file(filename: str):
    with open(filename, "w") as f:
        f.write("This is some text")

    yield

    os.remove(filename)
