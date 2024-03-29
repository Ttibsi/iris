# IRIS
Yet another terminal text editor. Taking a ton of inspiration from vim and
friends as well as other popular text editors (nano, emacs, VScode) to a lesser
extent. This only currently is designed to work in macOS and Linux.

All PRs are welcome

#### Why "Iris"?
* it's an easy combination of letters to type on an ascii keyboard without being
uncomfortable on the fingers
* The Iris is part of the eye, the "window to the soul" -- your text editor is
usually the window into your code
* Iris is the greek goddess of the rainbow, represents a bridge between
different aspects, much like your editor bridges text and code. (Thanks
ChatGPT)

### CONTROLS
Just like vim, Iris has a few modes: `read`, `write`, and `command`. You can
switch to `write` mode by pressing `i`, then back to `read` with the `esc` key.
Likewise, you can switch to `command` with the `;` (semicolon) key.

#### Read mode

| Key           | Description                                      |
|---------------|--------------------------------------------------|
| <kbd>h</kbd>  | Move cursor left                                 |
| <kbd>j</kbd>  | Move cursor down                                 |
| <kbd>k</kbd>  | Move cursor up                                   |
| <kbd>l</kbd>  | Move cursor right                                |
| <kbd>[</kbd>  | Move cursor to top of file                       |
| <kbd>]</kbd>  | Move cursor to bottom of file                    |
| <kbd>{</kbd>  | Move to previous empty line                      |
| <kbd>}</kbd>  | Move to next empty line                          |
| <kbd>;</kbd>  | Go to command mode                               |
| <kbd>i</kbd>  | Go to insert mode left of the current character  |
| <kbd>a</kbd>  | Go to insert mode right of the current character |
| <kbd>A</kbd>  | Go to insert mode at the end of the current line |
| <kbd>w</kbd>  | Move cursor forward one word                     |
| <kbd>b</kbd>  | Move cursor back one word                        |
| <kbd>z</kbd>  | Center the current line in the screen            |
| <kbd>x</kbd>  | Delete character under the cursor                |
| <kbd>dl</kbd> | Delete entire line                               |
| <kbd>dw</kbd> | Delete current word (between whitespace)         |
| <kbd>da</kbd> | Delete from beginning of line to cursor          |
| <kbd>de</kbd> | Delete to end of line                            |
| <kbd>f</kbd>  | Find string ahead in file                        |
| <kbd>r</kbd>  | Replace until next whitespace                    |

#### Command mode
All commands start with a semicolon

| Command      | Description                    |
|--------------|--------------------------------|
| `;w`         | Save current file              |
| `;w foo.txt` | Save current file to `foo.txt` |
| `;q`         | Quit                           |
| `;q!`        | Quit without saving            |
| `;wq`        | Write and quit                 |
| `;1`         | Jump to the given line number  |

### HOW TO RUN
Prerequisites: `cmake`

```bash
cmake -S . -B build/ && cmake --build build/
./build/src/iris
```
Add the `-DRUN-TESTS=OFF` flag to cmake to disable the tests being included.

To run unit tests, simply run `ctest` in the `build/`directory.
```bash
ctest --test-dir build/tests
```
