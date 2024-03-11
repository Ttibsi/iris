[ ] Switch the unit tests to catch2
[ ] Replace highlighting with lexical analysis
[ ] Rework buffer and viewport relationship
[ ] Create minbuffer for command palete
[ ] Introduce Actions system
[ ] Introduce undo/redo

```
Text editor restructure architecture
* A core Editor object that handles “global” state content
* The Editor holds multiple Viewports and multiple Buffers
* Each Viewport holds a pointer to a Buffer, but isn’t dependant on it - this will allow us to create Viewports for things that aren’t text Buffers (such as a file manager or command palette)
* Each Viewport will own a Cursor, which controls movement within that Viewport.
* Any key press (or other commands, but that I’m less clear on) will trigger an Action, usually sent from the Viewport to the Buffer (the Buffer object will have a method to handle parsing the Action)

I want to steal Emacs mini-buffer for things like “show errors”, searching, or switching to other open buffers. I also want to use vim’s `:make` as inspiration for a build command per project, or potentially combine it with something like Emacs compilation mode, but coming from vim, I still want to use vim keybindings.
```
