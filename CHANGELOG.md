### main / head
* Use `;e` to open a different file from within iris
* Keyboard commands `tt`, `tn`, `tp` to create and switch between tabs
* Display modified marker in the tab bar
* We now log the shell command called or the command entered in command mode
* `;wa` now saves all files and `;qa` quits all files
* `;lb` now shows all open buffers
* `dl` and `dw` now delete the current line and word

* Resolved issue with iris crashing after opening an existing file with 0 bytes

### v0.0.2
* Added ability to jump to a line number entered as a command: `;42`
* Ensure the tab key inserts a correct number of spaces
* Updated how the version string displays
* Added warning when opening files that rely on tab chars (Makefiles, go files)
* Added new `-l` flag to cli to specify what line number to open a file at
    * Ensured this line number doesn't overflow the file length
* Added new undo and redo commands
* Added `J` and `K` to move current line up or down
* Added querying for the current git branch in a thread for the statusbar
* Added handling opening files in READONLY mode if the current user doesn't
have correct access rights

* Resolved bug with cursor entering incorrect text after recentering screen
* Resolved bug with screen not redrawing correctly when entering the middle
line number in the command bar
* Resolved bug where app would crash when moving the cursor vertically after
editing text
* Resolved issue with backspace at 0th char moving backwards over line numbers

### v0.0.1
* Initial user input handling, drawing basic UI to the screen
* Added line number display and handling
* Added opening a file from the CLI, or starting an empty file
* Added the following vim commands (see readme for any differences)
    * `a`, `A`, `b`, `f`, `F`, `g`, `G`, `h`, `i`, `j`, `k`, `l`
    * `o`, `O`, `r`, `w`, `x`, `z`, `;`, `~`, `[`, `]`, `_`, `$`
* Added unit testing with catch2 and integration testing with python/hecate
* Added handling of SIGWINCH and backgrounding/forwarding in linux
