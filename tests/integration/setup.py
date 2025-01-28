import os
from collections.abc import Callable
from typing import Final
from typing import TypeVar

from hecate import Runner


T = TypeVar("T", bound=Callable[[Runner], None])
CMD_KEY: Final[str] = "\\;"


def setup(open_with: str = "") -> Callable[[T], Callable[[], None]]:

    def decorator(func: T) -> Callable[[], None]:
        def wrapper() -> None:
            with open("tests/fixture/temp_file.txt", "w") as f:
                f.write("Hello world")

            dims = {"width": 80, "height": 24}
            with Runner("build/src/iris", open_with, **dims) as r:
                r.await_text("READ", timeout=2)
                func(r)

            os.remove("tests/fixture/temp_file.txt")
        return wrapper
    return decorator


def get_statusbar_parts(lines: list[str], index: int = 22) -> list[str]:
    status_bar = lines[index].split(" ")
    status_bar = [x for x in status_bar if x != ""]

    return status_bar
