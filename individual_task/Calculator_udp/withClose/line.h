#ifndef LINE_H
#define LINE_H

// similar to getLineLN(), but ended '\n'-symbol always
// removed from returned buffer
char *getLine();

// returns line from keyboard (with memory auto-allocation)
// or NULL if there was some error: input or memory.
// line can be ended not with '\n' if it is the line before the EOF
// if getLineNL() returns not NULL value, you should use free(res)
// to release memory after using your line.
//
// If this function returns NULL (input or memory error) it is considered
// that you don't want to work with user after that.
// But you can still invoke this function if needed.
// Do we need extra code to test what error exactly happened? often we do not.
//
// Also, if some '\0'-symbols entered from keyboard, this can lead to
// missing some entered symbols, but it is ok
// NL means that NewLine character will not be discarded from the buffer.
char *getLineNL();

#endif
