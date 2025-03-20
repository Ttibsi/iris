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

### HOW TO RUN
Prerequisites: `cmake`, `ninja`, `Python`

I provide a wrapper python script to run all the required commands.
* To build just the binary, run `./run.py`
* To also run the unit tests: `./run.py test`
* For integration tests using hecate and tmux, run `./run.py test -I`
