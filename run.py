#!/usr/bin/env python3
import argparse
import os
import platform
import shutil
import stat
import subprocess
import timeit
import venv
from collections.abc import Sequence


def run_shell_cmd(
        cmd: str,
        *,
        env: dict[str, str] | None = None,
) -> int:
    return subprocess.run(cmd.split(), env=env).returncode


def loc() -> int:
    if not shutil.which("cloc"):
        print("cloc not installed...")
        return 0

    return run_shell_cmd(
        " ".join([
            "cloc .",
            " --exclude-dir=build,.cache,include,venv",
            " --exclude-ext=md,log,pyc",
        ]),
    )


def clean() -> int:
    print("Removing build directory")
    shutil.rmtree("build", ignore_errors=True)

    print("Cleaning up after python")
    shutil.rmtree("venv", ignore_errors=True)
    shutil.rmtree(".mypy_cache", ignore_errors=True)
    shutil.rmtree("tests/integration/.pytest_cache", ignore_errors=True)
    shutil.rmtree("tests/integration/__pycache__", ignore_errors=True)
    shutil.rmtree(".pytest_cache", ignore_errors=True)
    shutil.rmtree("release", ignore_errors=True)
    shutil.rmtree("include/catch2/lib64", ignore_errors=True) 

    files = [
        "src/version.h",
        "iris.log",
        "tests/fixture/read_only.txt",
        "tests/fixture/temp_file.txt",
        "tests/fixture/does_not_exist.txt",
    ]

    for file in files:
        if os.path.isfile(file):
            print(f"Removing {file}")
            os.remove(file)

    return 0


def create_read_only_file() -> None:
    file_name: str = "tests/fixture/read_only.txt"
    if os.path.exists(file_name):
        return

    print("Creating readonly file...")
    with open(file_name, "w") as f:
        f.write("This is a read-only file\n")
        f.write("We have some more text here")

    os.chmod(file_name, stat.S_IREAD)
    return


def integration_tests(test_name: str = "") -> int:
    def create_venv() -> int:
        if os.path.isdir("venv"):
            return 0
        venv.create("venv", with_pip=True)

        hecate_repo = "git+https://github.com/ttibsi/hecate.git@upgrades"
        return run_shell_cmd(f"venv/bin/pip install pytest {hecate_repo}")

    ret: int = build()
    if ret:
        return ret

    ret = create_venv()
    if ret:
        return ret

    create_read_only_file()
    test_folder_subpath: str = os.path.join(os.getcwd(), "tests/integration")
    test_paths: list[str] = [
        os.path.join(test_folder_subpath, file_path)
        for file_path in os.listdir("tests/integration")
        if "__pycache__" not in file_path
    ]

    if test_name:
        return run_shell_cmd(f"venv/bin/pytest -v -k {test_name}")
    else:
        return run_shell_cmd(f"venv/bin/pytest -v {' '.join(test_paths)}")


def create_symlink() -> None:
    location = "include/catch2/lib64"
    if os.path.exists(location):
        return

    src: str = "lib64"

    if platform.machine() == "x86_64":
        src += "_x86_64"
        print("Creating symlink (x86_64)...")
    elif platform.machine() == "aarch64":
        src += "_aarch64"
        print("Creating symlink (aarch64)...")
    else:
        raise OSError("Architecture not found")

    shutil.copytree(f"include/catch2/{src}", location) 


def test(testname: str | None, asan: bool) -> int:
    create_symlink()
    if not os.path.exists("include/catch2/lib64/libCatch2.a"):
        raise AssertionError("The symlink doesn't exist?") 
         
    compile_cmd = "cmake -G Ninja -DRUN_TESTS=true -S . -B build"
    if asan:
        compile_cmd += " -DENABLE_ASAN=true"

    ret: int = 0
    ret = run_shell_cmd(compile_cmd)
    if ret:
        return ret

    ret = run_shell_cmd("cmake --build build/")
    if ret:
        return ret

    create_read_only_file()
    testname = testname if testname is not None else ""
    test_flags: str = "-sr compact --order rand"
    shell_cmd: str = f"./build/tests/test_exe {test_flags} {testname}"

    return run_shell_cmd(
        shell_cmd, env={
            "RAWTERM_DEBUG": "true",
            "ASAN_OPTIONS": "symbolize=1",
            "ASAN_SYMBOLIZER_PATH": "/usr/bin/llvm-symbolizer",
        },
    )


def build(release: bool = False) -> int:
    ret: int = 0
    cmd: str = "cmake -G Ninja -S . -B build"
    if release:
        cmd += " -DRELEASE=true"

    ret = run_shell_cmd(cmd)
    if ret:
        return ret

    ret = run_shell_cmd("cmake --build build/")
    if ret:
        return ret

    if release:
        ret = run_shell_cmd("strip build/src/iris")
        if ret:
            return ret

        os.mkdir(os.getcwd() + "/release")
        shutil.copyfile("build/src/iris", "release/iris")
        os.chmod("release/iris", 0o755)

    return 0


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--release", action="store_true", default=False)

    subparsers = parser.add_subparsers(dest="cmd")
    subparsers.add_parser("clean")
    subparsers.add_parser("loc")

    test_parser = subparsers.add_parser("test")
    test_parser.add_argument("testname", nargs="?", default=None)
    test_parser.add_argument("--asan", action="store_true", default=False)
    test_parser.add_argument(
        "-I", "--integration",
        action="store_true", default=False,
    )

    args: argparse.Namespace = parser.parse_args(argv)
    print(args)

    start = timeit.default_timer()
    ret = 0
    if args.cmd == "clean":
        ret = clean()
    elif args.cmd == "loc":
        ret = loc()
    elif args.cmd == "test":
        if args.integration:
            ret = integration_tests(args.testname)
        else:
            ret = test(args.testname, args.asan)
    else:
        ret = build(args.release)

    print(f"Elapsed time: {round(timeit.default_timer() - start, 2)} Seconds")

    return ret


if __name__ == '__main__':
    raise SystemExit(main())
