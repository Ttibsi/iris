import hecate
from setup import setup


@setup()
def test_clean_startup(r: hecate.Runner):
    lines = r.screenshot().split("\n")

    assert len(lines) == 25  # TODO: Investigate why theres an extra line here
    assert str(1) in lines[0]

    for i in range(1, 22):
        assert "~" in lines[i]

    assert "1:1" in lines[22]


@setup("tests/fixture/test_file_1.txt")
def test_open_with_file(r: hecate.Runner):
    lines = r.screenshot().split("\n")

    with open("tests/fixture/test_file_1.txt") as file:
        content: list[str] = file.readlines()

    for idx, line in enumerate(content, start=1):
        expected = f" {idx}\u2502{line.rstrip().replace("\t", " " * 4)}"
        assert lines[idx - 1] == expected

    assert lines[len(content) + 3] == "~"

    status_bar = lines[22].split(" ")
    status_bar = [x for x in status_bar if x != ""]
    assert len(status_bar) == 4
    assert status_bar[1] == "tests/fixture/test_file_1.txt"
