import contextlib
import os
import time
from collections.abc import Callable
from collections.abc import Generator
from typing import Final
from typing import TypeVar

from hecate import Runner


T = TypeVar("T", bound=Callable[[Runner], None])


class TmuxRunner(Runner):
    CMD_KEY: Final[str] = "\\;"
    SELECTED_LINE_ANSI: Final[str] = "\x1b[38;2;255;221;51m"

    def __init__(self, *args, **kwargs) -> None:
        super().__init__(*args, **kwargs)

    def lines(self) -> list[str]:
        return super().screenshot().split("\n")[:-1]

    # idea stolen from asottile/babi - `testing/runner.py`
    def color_screenshot(self) -> list[str]:
        return self.tmux.execute_command('capture-pane', '-ept0').split("\n")

    def await_statusbar_parts(self, index: int = 22) -> list[str]:
        for _ in self.poll_until_timeout():
            if "|" not in self.lines()[index]:
                continue

            return self.statusbar_parts()

        raise AssertionError("Timeout while waiting for statusbar")

    def statusbar_parts(self, index: int = 22) -> list[str]:
        return [
            part.strip()
            for part in self.lines()[index].split("|")
            if part != ""
        ]

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

    # NOTE: zero-indexed
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

    def await_cursor_pos(self, x: int, y: int) -> None:
        for _ in self.poll_until_timeout(timeout=10):
            pos = self.cursor_pos()
            if pos == (x, y):
                return
        else:
            raise AssertionError(
                    f"Timeout searching for cursor pos: ({x}, {y})"
                    f" - Found: {pos}",
            )

    def assert_text_missing(self, text: str, wait: float = 0.1) -> None:
        time.sleep(wait)

        for line in self.lines():
            if text in line:
                raise AssertionError(f"Text: '{text}' found")

    def assert_internal_cur_pos(self, x: int, y: int) -> None:
        pos = ""
        for _ in range(5):
            pos = self.await_statusbar_parts()[-1]
            if pos == f"{x}:{y}":
                return
        else:
            raise AssertionError(
                    f"Timeout searching for cursor pos: ({x}, {y})"
                    f" - Found: {pos}",
            )


def setup(
        open_with: str = "",
        *, width: int = 80,
        multi_file: bool = False,
) -> Callable[[T], Callable[[], None]]:
    temp_file: str = "tests/fixture/temp_file.txt"

    def decorator(func: T) -> Callable[[], None]:
        def wrapper() -> None:
            if os.path.isfile(temp_file):
                os.remove(temp_file)

            with open(temp_file, "w") as f:
                f.write("Hello world")

            dims = {"width": width, "height": 24}
            file: str = open_with if not multi_file else temp_file
            with TmuxRunner("build/src/iris", file, **dims) as r:
                r.await_text("READ")
                if multi_file:
                    r.iris_cmd(f"e {open_with}")

                func(r)

        return wrapper
    return decorator


@contextlib.contextmanager
def temp_named_file(filename: str) -> Generator[None]:
    with open(filename, "w") as f:
        f.write("This is some text")

    yield

    os.remove(filename)
