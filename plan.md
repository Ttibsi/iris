```cpp
struct Line {
    int start;
    int end;
};

struct Cursor {
    int row;
    int col;
};

struct Change {
    int start_pos;
    int end_pos;
    std::vector<char> content;
};

struct Buffer {
    std::vector<Line> lines;
    std::string data;
    int length;

    std::string file;
    Cursor cursor;
    bool readonly;
    bool modified;
    std::vector<Change> undo;
    std::vector<Change> redo;
};

enum Mode {
    Read,
    Write
    Command
};

struct Editor {
    std::vector<Buffer> buffers;
    int active_buffer;
    Mode mode;
    std::string clipboard;
    std::pair<int, int> term_size;
};
```

This idea is this: all the text is stored as a single string in continuous
memory (data). Each line is divided up by the index of it's first character and the \n
character. We store the length as the number of line objects there are.

Cursor movement:
    - h - col-- (left)
    - j - row++ (down)
    - k - row-- (up)
    - l - col++ (right)

    - Other cursor movements will depend on the element it's interacting on

Inserting new line:
    - insert \n at cursor position,
    - Create a new Line object with the start = current location + 1,
    end = current line's end
    - upper line end pos updated to current pos
    - length ++

Deleting a line:
    - prev Line end becomes current Line end
    - delete current Line object
    - data - remove all chars up to and including next \n
    - length--

Inserting a char:
    - Add to char array at current location
    - update all Lines to end adding 1 to start and end values

Deleting a char:
    - Remove char from current location
    - update Line objects to end, decreasing start and end by 1

undo/redo:
    - Every time we enter write mode, we create a new Change
    - every keypress is appended to the content vec
    - on undo, we need to go through every char and perform the reverse action
        - need to research if this is the best method or not
    - we add the Change to the redo stack
    - on redo, we re-perform the same action

Writing to file should be simple as you can insert \n characters

```bash
❯ echo "foo\nbar\nbaz" > t.txt
❯ cat t.txt
foo
bar
baz
```

Reading from file will need to build the data structures - this may be the
slowest operation

If we start up without a file, we want to initialize a single empty line at the
start of the buffer
