#!/usr/bin/env python3
import argparse
import os
import shutil
import subprocess
import timeit
from collections.abc import Sequence


def run_shell_cmd(
        cmd: str,
        *,
        env: dict[str, str] | None = None,
        debug: bool = False,
) -> None:

    if debug:
        print(f"RUNNING COMMAND: {cmd}")

    try:
        subprocess.run(cmd.split(), env=env).check_returncode()
    except subprocess.CalledProcessError:
        print(f"CMD FAILED: {cmd}")


def loc() -> None:
    run_shell_cmd(
        " ".join([
            "cloc . ../rawterm",
            " --exclude-dir=build,.cache,cli11,examples",
            " --exclude-ext=md",
        ]),
    )


def clean() -> None:
    print("Removing build directory")
    shutil.rmtree("build", ignore_errors=True)
    shutil.rmtree(".mypy_cache", ignore_errors=True)

    files = [
        "src/version.h",
        "iris.log",
    ]

    for file in files:
        if os.path.isfile(file):
            print(f"Removing {file}")
            os.remove(file)


def test(testname: str | None, asan: bool) -> None:
    compile_cmd = "cmake -G Ninja -DRUN_TESTS=true -S . -B build"
    if asan:
        compile_cmd += " -DENABLE_ASAN=true"

    run_shell_cmd(compile_cmd, debug=True)
    run_shell_cmd("cmake --build build/")
    # TODO: `--order=rand` for fuzz testing I then need to fix any polluting
    # tests
    # (I think it's down to rawterm func calls that straight print to stdout)
    run_shell_cmd(
        f"./build/tests/test_exe -i {testname if testname else ''}",
        env={
            "RAWTERM_DEBUG": "true",
            "ASAN_OPTIONS": "symbolize=1",
            "ASAN_SYMBOLIZER_PATH": "/usr/bin/llvm-symbolizer",
        },
    )


def build() -> None:
    run_shell_cmd("cmake -G Ninja -S . -B build")
    run_shell_cmd("cmake --build build/")


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest="cmd")
    subparsers.add_parser("clean")
    subparsers.add_parser("loc")
    test_parser = subparsers.add_parser("test")
    test_parser.add_argument("testname", nargs="?", default=None)

    test_parser.add_argument("--asan", action="store_true", default=False)

    args: argparse.Namespace = parser.parse_args(argv)
    print(args)

    start = timeit.default_timer()
    if args.cmd == "clean":
        clean()
    elif args.cmd == "loc":
        loc()
    elif args.cmd == "test":
        test(args.testname, args.asan)
    else:
        build()

    print(f"Elapsed time: {round(timeit.default_timer() - start, 2)} Seconds")

    return 0


if __name__ == '__main__':
    raise SystemExit(main())
