# IRIS
Yet another terminal text editor. Taking a ton of inspiration from vim and
friends as well as other popular text editors (nano, emacs, VS*ode) to a lesser
extent

#### Why "Iris"?
* it's an easy combination of letters to type on an ascii keyboard without being
uncomfortable on the fingers
* The Iris is part of the eye, the "window to the soul" -- your text editor is
usually the window into your code
* Iris is the greek goddess of the rainbow, represents a bridge between
different aspects, much like your editor bridges text and code. (Thanks
ChatGPT)

### HOW TO RUN
Prerequisites: `cmake`

```bash
cmake -S . -B build/ && cd build && make
./src/iris
```
Add the `-DRUN-TESTS=OFF` flag to cmake to disable the tests being included.

To run unit tests, simply run `ctest` in the `build/`directory.
