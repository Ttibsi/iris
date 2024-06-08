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
) -> int:

    if debug:
        print(f"RUNNING COMMAND: {cmd}")

    subprocess.run(cmd.split(), env=env).check_returncode()


def loc() -> None:
    run_shell_cmd(
        (
            "cloc . ../rawterm" +
            " --exclude-dir=build,.cache,cli11,examples" +
            " --exclude-ext=md"
        ),
    )


def clean() -> None:
    print("Removing build directory")
    shutil.rmtree("build", ignore_errors=True)

    print("Removing log file")
    try:
        os.remove("iris.log")
    except FileNotFoundError:
        pass


def test(testname: str, asan: bool, coverage: bool) -> None:
    compile_cmd = "cmake -DRUN_TESTS=true -S . -B build"
    if asan:
        compile_cmd += " -DENABLE_ASAN=true"
    if coverage:
        compile_cmd += " -DENABLE_COVERAGE=true"

    run_shell_cmd(compile_cmd, debug=True)
    run_shell_cmd("cmake --build build/")
    run_shell_cmd(
        f"./build/tests/test_exe {testname}",
        env={"RAWTERM_DEBUG": "true"},
    )

    # if coverage:
    #     onlyfiles = [
    #         test_file
    #         for test_file in os.listdir("src/")
    #         if os.path.isfile(
    #             os.path.join("src/", test_file)
    #         ) and test_file.endswith(".cpp")
    #     ]
    #     for file in onlyfiles:
    #         run_shell_cmd(
    # f"gcov -o build/src/CMakeFiles/iris_src.dir/ src/{file} -m -t",
    # debug=True)


def build() -> None:
    run_shell_cmd("cmake -S . -B build")
    run_shell_cmd("cmake --build build/")


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest="cmd")
    subparsers.add_parser("clean")
    subparsers.add_parser("loc")
    test_parser = subparsers.add_parser("test")
    # test_parser.add_argument("testname", type=str)
    test_parser.add_argument("testname", action="store_const", const="")

    test_parser.add_argument("--asan", action="store_true", default=False)
    test_parser.add_argument("--coverage", action="store_true", default=False)

    args: argparse.Namespace = parser.parse_args(argv)
    print(args)

    start = timeit.default_timer()
    if args.cmd == "clean":
        clean()
    elif args.cmd == "loc":
        loc()
    elif args.cmd == "test":
        test(args.testname, args.asan, args.coverage)
    else:
        build()

    print(f"Elapsed time: {timeit.default_timer() - start} Seconds")

    return 0


if __name__ == '__main__':
    raise SystemExit(main())
