# Milestones

* [x] Display a text file on the screen
* [x] Display line numbers and a statusbar
* [x] Move cursor and scroll through a file
* [x] Open an empty buffer
* [x] Modify and save the file
* [x] Create and save a new file
* [x] Handle linux backgrounding (CTRL+Z)
* [ ] Handle SIGWINCH
* [ ] Handle displaying tab characters with specific widths
    This may be a function to add to Rawterm instead
* [ ] Handle replacing tab chars with spaces on save - make it togglable
* [ ] Handle undo/redo as two stacks of Actions, and a system to know how
    to perform the opposite of the action
* [ ] Open a second file in a split
* [ ] Highlight trailing whitespace at the end of a line and delete on save
* [ ] Add handling for terminal resizing
* [ ] Handle viewing files you don't have access rights to edit as readonly
* [ ] Open a command pallete at the bottom third of the screen
* [ ] Syntax highlighting via lexical analysis and regex
* [ ] Use the command pallete to run a shell command - investigate forking
* [ ] Switch file we're editing from inside the editor (ex vim's `:e`)
* [ ] Find and replace in the current buffer using the command pallete
* [ ] Open a file with the cursor on a specified line using `-l` on the CLI
* [ ] Some form of simple file manager like vim's netrw
* [ ] Open multiple tabs (delineated at the bottom of the screen) using a new
    PaneManager class for each tab
* [ ] Configuration file (potentially JSON) - global at `XDG_CONFIG_HOME/iris`
    and a repo-specific config to be found with `git grep` potentially
* [ ] Lua plugins and custon themes extensibility

## Vim commands

* [x] `h`
* [x] `j`
* [x] `k`
* [x] `l`
* [x] `x`
* [x] `a`
* [x] `A`
* [ ] `_`
* [ ] `$`
* [ ] `w`
* [ ] `b`
* [ ] `o`
* [ ] `O`
* [ ] `r`
* [ ] `f`
* [ ] `F`
