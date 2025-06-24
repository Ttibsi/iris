# Milestones

* [x] Display a text file on the screen
* [x] Display line numbers and a statusbar
* [x] Move cursor and scroll through a file
* [x] Open an empty buffer
* [x] Modify and save the file
* [x] Create and save a new file
* [x] Handle linux backgrounding (CTRL+Z)
* [x] Handle SIGWINCH
* [x] Handle displaying tab characters with specific widths
* [x] Handle undo/redo as two stacks of Actions, and a system to know how
    to perform the opposite of the action
    * [x] Mark the buffer as modified and update quitting logic as appropriate
* [x] Open a file with the cursor on a specified line using `-l` on the CLI
* [x] Handle viewing files you don't have access rights to edit as readonly
* [x] Show the current git branch on the statusbar
* [x] Switch file we're editing from inside the editor (ex vim's `:e`)
* [x] Open a second file in a tab and switch tabs
* [x] Open a second file in a separate buffer and switch between views
* [ ] View a list of open files and switch between them
* [ ] Handle tab characters in text
    * [ ] Toggle an `expandtab` config variable with a command (default true)
    * [ ] Ensure pressing <TAB> inserts the right character where needed
    * [ ] If `expandtab` is false, have a way of calculating the logical
    and visual positions of the cursor separately for vertical movement and
    char insertion
* [ ] Highlight trailing whitespace at the end of a line and delete on save
* [ ] Open a second file in a split
* [ ] Open a command pallete at the bottom third of the screen
* [ ] Syntax highlighting via lexical analysis and regex
* [ ] Use the command pallete to run a shell command - investigate forking
* [ ] Find and replace in the current buffer using the command pallete
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
* [x] `_`
* [x] `$`
* [x] `w`
* [x] `b`
* [x] `o`
* [x] `O`
* [x] `[`
* [x] `]`
* [x] `r`
* [x] `f`
* [x] `F`
* [x] `g` (vim `gg`)
* [x] `G`
* [x] `z` (vim `zz`)
* [x] `~`
* [ ] `d`
    * [ ] `l` (delete line)
    * [ ] `w` (delete word)
* [x] `J`
* [x] `K`
