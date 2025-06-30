# IRIS
Yet another terminal text editor. Taking inspiration from vim and friends as
well as other popular text editors (nano, emacs, VScode) to a lesser extent.
This only currently is designed to work in macOS and Linux.

All PRs are welcome

[![asciicast](https://asciinema.org/a/twEuFoNwp6hMrbffdX9BpEVBt.svg)](https://asciinema.org/a/twEuFoNwp6hMrbffdX9BpEVBt)

#### Why "Iris"?
* it's an easy combination of letters to type on an ascii keyboard without being
uncomfortable on the fingers
* The Iris is part of the eye, the "window to the soul" -- your text editor is
usually the window into your code
* Iris is the greek goddess of the rainbow, represents a bridge between
different aspects, much like your editor bridges text and code. (Thanks
ChatGPT)

### How to run
Prerequisites: `cmake`, `ninja`, `Python`

I provide a wrapper python script to run all the required commands.
* To build just the binary, run `./run.py`
* To also run the unit tests: `./run.py test`
* For integration tests using hecate and tmux, run `./run.py test -I`

To make a release binary, run the following commands on a Fedora-based system.
(Adjust as needed for your local system)
```console
$ sudo dnf install glibc-static libstdc++-static
$ ./run.py --release
```

# How to use
### Normal mode
`iris` is a vim-inspired modal text editor. As with vim, any you'll need to
switch to `write` mode to insert any characters. In `read` mode, you can
perform the following actions (alphabetically ordered):

| Key           | Description                                           |
|---------------|-------------------------------------------------------|
| <kbd>a</kbd>  | Go to write mode right of the current character       |
| <kbd>A</kbd>  | Go to write mode at the end of the current line       |
| <kbd>b</kbd>  | Move cursor back one word                             |
| <kbd>f</kbd>  | Find next entered char ahead in file                  |
| <kbd>F</kbd>  | Find next entered char back in file                   |
| <kbd>g</kbd>  | Go to top of file                                     |
| <kbd>G</kbd>  | Go to bottom of file                                  |
| <kbd>h</kbd>  | Move cursor left                                      |
| <kbd>i</kbd>  | Go to write mode left of the current character        |
| <kbd>j</kbd>  | Move cursor down                                      |
| <kbd>J</kbd>  | Move current line down one                            |
| <kbd>k</kbd>  | Move cursor up                                        |
| <kbd>K</kbd>  | Move current line up one                              |
| <kbd>l</kbd>  | Move cursor right                                     |
| <kbd>o</kbd>  | Enter newline below and go to write mode              |
| <kbd>O</kbd>  | Move cursor right                                     |
| <kbd>r</kbd>  | Replace char under cursor with next entered character |
| <kbd>R</kbd>  | Redo                                                  |
| <kbd>tn</kbd> | Go to next tab                                        |
| <kbd>tp</kbd> | Go to previous tab                                    |
| <kbd>tt</kbd> | Open new tab                                          |
| <kbd>u</kbd>  | Undo                                                  |
| <kbd>w</kbd>  | Move cursor forward one word                          |
| <kbd>x</kbd>  | Delete character under the cursor                     |
| <kbd>z</kbd>  | Center the current line in the screen                 |
| <kbd>;</kbd>  | Go to command mode                                    |
| <kbd>~</Kbd>  | Switch case of char under cursor                      |
| <kbd>[</Kbd>  | Go to beginning of paragraph (previous empty line)    |
| <kbd>]</Kbd>  | Go to end of paragraph (next empty line)              |
| <kbd>_</Kbd>  | Go to beginning of line                               |
| <kbd>$</Kbd>  | Go to end of line                                     |

### Command mode
Likewise, you can switch to `command` mode with the semicolon `;` key. The
following commands in alphabetical order are available:

| Command | Description                                                     |
|---------|-----------------------------------------------------------------|
| `;ls`   | See all open buffers                                            |
| `;ping` | `pong` (for testing purposes)                                   |
| `;e`    | Open a new buffer - specify a filename to open an existing file |
| `;q`    | Quit                                                            |
| `;q!`   | Force Quit without saving                                       |
| `;wq`   | Save and quit                                                   |
| `;w`    | Save file                                                       |
| `;wa`   | Save all files                                                  |
| `;qa`   | Quit all                                                        |

### CLI
The below flags are available in the CLI as well. See `--help` for more info.

| Flag        | Description                                  |
|-------------|----------------------------------------------|
| `-l<int>`   | Open the named file on the given line number |
| `--version` | Print the current version and exit.          |
