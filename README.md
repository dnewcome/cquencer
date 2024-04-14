# Cquencer

tiny super accurate MIDI sequencer written in portable c.

## Goals
curses terminal based sequencer
vim keyboard shortcuts
accurate timing
fast small program

## building

### dependencies

- rtmidi
- libdispatch
- libblocksruntime
- ncurses

### Ubuntu

libdispatch instead of GCD
clang with -fblocks to compile


### OSX

GCD grand central dispatch, should be installed already.

```
brew install rtmidi
brew install ncurses
```

### cross compile notes for Windows

https://stackoverflow.com/questions/44389963/how-to-install-mingw32-on-ubuntu

