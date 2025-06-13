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
    filename: str

    def __init__(self, *args, **kwargs) -> None:
        super().__init__(*args, **kwargs)

    def lines(self) -> list[str]:
        return super().screenshot().split("\n")[:-1]

    # idea stolen from asottile/babi - `testing/runner.py`
    def color_screenshot(self) -> list[str]:
        return self.tmux.execute_command(
                'capture-pane',
                '-ept0',
        ).split("\n")[:-1]

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
            time.sleep(0.1)

    def press_and_enter(self, s: str) -> None:
        self.type_str(s)
        time.sleep(0.1)
        self.press("Enter")

    def iris_cmd(self, cmd: str) -> None:
        super().press(self.CMD_KEY)
        super().await_text("COMMAND", timeout=2)
        self.press_and_enter(cmd)

    def await_cursor_pos(self, x: int, y: int) -> None:
        for _ in self.poll_until_timeout(5):
            pos = self.cursor_pos()
            if pos == (x, y):
                return

        else:
            raise AssertionError(
                f"Timeout searching for cursor pos: ({x}, {y})"
                f" - Found: {pos}",
            )

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

    def await_tab_bar_parts(self) -> list[str]:
        for _ in self.poll_until_timeout():
            parts = self.color_screenshot()[0]
            if "|" not in parts:
                continue

            return [p.strip() for p in parts.split("|") if p.strip()]

        raise AssertionError("Timeout while waiting for tab bar")

    def assert_inverted_text(self, inverted: str, text: str) -> None:
        expected = f"\x1B[7m{text}\x1B[0m"
        if inverted != expected:
            raise AssertionError(f"{inverted} does not equal {expected}")

    # As the filename in statusbar gets truncated
    def assert_filename_in_statusbar(self, filename: str) -> None:
        if not any(filename in string for string in self.statusbar_parts()):
            raise AssertionError(f"{filename} not found in Statusbar")

    def get_lineno(self, line_num: int) -> str:
        line = self.color_screenshot()[line_num]
        return line.split("\u2502")[0]


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
                r.filename = open_with
                r.await_text("READ", timeout=2)

                if multi_file:
                    r.type_str("tt")
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
