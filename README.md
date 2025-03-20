# IRIS
Yet another terminal text editor. Taking inspiration from vim and friends as
well as other popular text editors (nano, emacs, VScode) to a lesser extent.
This only currently is designed to work in macOS and Linux.

All PRs are welcome

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

# How to use
`iris` is a vim-inspired modal text editor. As with vim, any you'll need to
switch to `write` mode to insert any characters. In `read` mode, you can
perform the following actions (alphabetically ordered):

| Key | Description                                                     |
|-----|-----------------------------------------------------------------|
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
| <kbd>k</kbd>  | Move cursor up                                        |
| <kbd>l</kbd>  | Move cursor right                                     |
| <kbd>o</kbd>  | Enter newline below and go to write mode              |
| <kbd>O</kbd>  | Move cursor right                                     |
| <kbd>r</kbd>  | Replace char under cursor with next entered character |
| <kbd>w</kbd>  | Move cursor forward one word                          |
| <kbd>x</kbd>  | Delete character under the cursor                     |
| <kbd>z</kbd>  | Center the current line in the screen                 |
| <kbd>;</kbd>  | Go to command mode                                    |
| <kbd>~</Kbd>  | Switch case of char under cursor                      |
| <kbd>[</Kbd>  | Go to beginning of paragraph (previous empty line)    |
| <kbd>]</Kbd>  | Go to end of paragraph (next empty line)              |
| <kbd>_</Kbd>  | Go to beginning of line                               |
| <kbd>$</Kbd>  | Go to end of line                                     |

Likewise, you can switch to `command` mode with the semicolon `;` key. The
following commands in alphabetical order are available:

| Command | Description                   |
|---------|-------------------------------|
| `;ping` | `pong` (for testing purposes) |
| `;q`    | Quit without saving           |
| `;wq`   | Save and quit                 |
| `;w`    | Save file                     |
