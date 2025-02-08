import os
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

    def iris_cmd(self, cmd: str) -> None:
        super().press(self.CMD_KEY)
        super().await_text("COMMAND", timeout=2)

        for char in cmd:
            super().press(char)

        super().press("Enter")

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
