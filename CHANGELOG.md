### main / head
* Added ability to jump to a line number entered as a command: `;42`
* Ensure the tab key inserts a correct number of spaces
* Updated how the version string displays

* Resolved bug with cursor entering incorrect text after recentering screen
* Resolved bug with cursor entering incorrect text after recentering screen
* Resolved bug with screen not redrawing correctly when entering the middle
line number in the command bar

### v0.0.1
* Initial user input handling, drawing basic UI to the screen
* Added line number display and handling
* Added opening a file from the CLI, or starting an empty file
* Added the following vim commands (see readme for any differences)
    * `a`, `A`, `b`, `f`, `F`, `g`, `G`, `h`, `i`, `j`, `k`, `l`
    * `o`, `O`, `r`, `w`, `x`, `z`, `;`, `~`, `[`, `]`, `_`, `$`
* Added unit testing with catch2 and integration testing with python/hecate
* Added handling of SIGWINCH and backgrounding/forwarding in linux
