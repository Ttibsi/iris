from setup import setup
from setup import TmuxRunner


@setup()
def test_enter_command_mode(r: TmuxRunner):
    r.press(r.CMD_KEY)

    assert r.lines()[-1] == ";"
    assert r.statusbar_parts()[0] == "COMMAND"


@setup()
def test_add_char_to_cmd(r: TmuxRunner):
    r.press(r.CMD_KEY)
    r.type_str("ping")

    assert r.lines()[-1] == ";ping"
    assert r.statusbar_parts()[0] == "COMMAND"


@setup()
def test_remove_char_to_cmd(r: TmuxRunner):
    r.press(r.CMD_KEY)
    r.type_str("ping")
    r.press("BSpace")

    assert r.lines()[-1] == ";pin"
    assert r.statusbar_parts()[0] == "COMMAND"


@setup()
def test_submit_command(r: TmuxRunner):
    r.press(r.CMD_KEY)
    r.type_str("ping")
    r.press("Enter")

    assert r.lines()[-1] == "pong"
    assert r.statusbar_parts()[0] == "READ"


@setup()
def test_escape_cmd_mode(r: TmuxRunner):
    r.press(r.CMD_KEY)
    r.type_str("ping")
    r.press("Escape")

    assert r.lines()[-1] == ""
    assert r.statusbar_parts()[0] == "READ"
