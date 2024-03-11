#ifndef CURSOR_H
#define CURSOR_H

// NOTE: Cursor represents the location in the terminal that the cursor is
// This isn't the location in the file, or the location in the string vector
// Cursor should be 1-indexed
struct Cursor {
    int row;
    int col;
};

#endif // CURSOR_H
