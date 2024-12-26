#!/usr/bin/env python3
import argparse
import os
import shutil
import subprocess
import sys
import timeit
import venv
from collections.abc import Sequence


def run_shell_cmd(
        cmd: str,
        *,
        env: dict[str, str] | None = None,
        debug: bool = False,
) -> int:

    if debug:
        print(f"RUNNING COMMAND: {cmd}")

    return subprocess.run(cmd.split(), env=env).returncode


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

    print("Cleaning up after python")
    shutil.rmtree("venv", ignore_errors=True)
    shutil.rmtree(".mypy_cache", ignore_errors=True)
    shutil.rmtree("tests/integrations/.pytest_cache", ignore_errors=True)
    shutil.rmtree("tests/integrations/__pycache__", ignore_errors=True)
    shutil.rmtree(".pytest_cache", ignore_errors=True)

    files = [
        "src/version.h",
        "iris.log",
    ]

    for file in files:
        if os.path.isfile(file):
            print(f"Removing {file}")
            os.remove(file)


def integration_tests(test_name: str = "") -> None:
    def create_venv() -> None:
        if os.path.isdir("venv"):
            return
        venv.create("venv", with_pip=True)

        hecate_repo = "git+https://github.com/ttibsi/hecate.git@upgrades"
        run_shell_cmd(f"venv/bin/pip install pytest {hecate_repo}")

    if not os.path.isdir("build"):
        print("ERROR: Can't run integration tests without binary")
        build()

    create_venv()

    test_folder_subpath: str = os.path.join(os.getcwd(), "tests/integrations")
    test_paths: list[str] = [
        os.path.join(test_folder_subpath, file_path)
        for file_path in os.listdir("tests/integrations")
        if "__pycache__" not in file_path
    ]

    if test_name:
        sys.exit(run_shell_cmd(f"venv/bin/pytest -v {test_name}"))
    else:
        sys.exit(run_shell_cmd(f"venv/bin/pytest -v {' '.join(test_paths)}"))


def test(testname: str | None, asan: bool, success: bool) -> None:
    compile_cmd = "cmake -G Ninja -DRUN_TESTS=true -S . -B build"
    if asan:
        compile_cmd += " -DENABLE_ASAN=true"

    run_shell_cmd(compile_cmd, debug=True)
    run_shell_cmd("cmake --build build/")

    if testname is None:
        testname = ""
    shell_cmd: str = f"./build/tests/test_exe {testname} --order rand"
    if success:
        shell_cmd += " -s"

    run_shell_cmd(
        shell_cmd, env={
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
    test_parser.add_argument(
        "--success", "-s", action="store_true", default=False,
    )
    test_parser.add_argument("--asan", action="store_true", default=False)
    test_parser.add_argument(
        "-I", "--integration",
        action="store_true", default=False,
    )

    args: argparse.Namespace = parser.parse_args(argv)
    print(args)

    start = timeit.default_timer()
    if args.cmd == "clean":
        clean()
    elif args.cmd == "loc":
        loc()
    elif args.cmd == "test":
        if args.integration:
            integration_tests(args.testname)
        else:
            test(args.testname, args.asan, args.success)
    else:
        build()

    print(f"Elapsed time: {round(timeit.default_timer() - start, 2)} Seconds")

    return 0


if __name__ == '__main__':
    raise SystemExit(main())
