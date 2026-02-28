# Cquencer

tiny super accurate MIDI sequencer written in portable c.

## Goals
curses terminal based sequencer
vim keyboard shortcuts
accurate timing
fast small program

## Architecture

The program is structured across four files that are `#include`d into a single translation unit:

```
main.c
  └── clock.c         (timer, MIDI, ncurses UI, sequencer logic)
        └── array.c   (dynamic array for event* pointers)
              └── cseq.h  (event struct definition)
```

Entry point is `main()` in `main.c`, which calls `init_array()`, `init_midi()`, then `clk_main()` which blocks on the ncurses input loop.

## Timing

Timing is driven by a GCD (`libdispatch`) timer source on a dedicated serial queue. The timer fires at **sub-step resolution** — 24 ticks per quarter note (standard MIDI clock PPQN).

```
tick interval = (1 second * 60) / BPM / time_signature / ticks_per_quarter
```

At default 120 BPM, 4/4, 24 PPQN this is ~5208 µs per tick. The `vector1()` handler fires on every tick, checks which steps are active, fires MIDI, and increments the tick counter. The grid display and step counter (`i`) only advance every `ticks_per_quarter` ticks, keeping the UI at quarter-note resolution while MIDI fires at higher precision.

BPM changes take effect on the next `draw_grid()` call via `dispatch_source_set_timer()` — the `timer_reset` flag is the handoff between the input loop and the timer handler.

## Data Model

There are two representations of sequence data in-flight:

**`notes[4][16]`** — the active step grid. A 4-track × 16-step boolean matrix. This is what the UI edits and the playback engine reads.

**`events[]`** — a static array of `struct event {start, end, chan}` used for the sparse/piano-roll style renderer (`draw_grid_sparse`). Currently hardcoded; the dynamic `Array` (from `array.c`) is scaffolded to replace this but not yet wired into playback.

`array.c` implements a generic growable array typed to `struct event*` that doubles capacity on overflow via `realloc`.

## MIDI

RtMidi's C API (`rtmidi_c.h`) is used. On startup, `init_midi()` enumerates available APIs and ports and opens port 4 ("Bus 1"). Notes are sent as immediate note-on + note-off pairs (velocity 0x7f, note 0x24 = C1) on the channel corresponding to the track index.

MIDI channel is encoded in the status byte: `0x90 + ch` for note-on, `0x80 + ch` for note-off.

## UI / Keybindings

ncurses in raw mode. The display shows BPM, current step, raw tick count, and the 4×16 step grid. A reversed-video `=` character marks the current playhead column.

| Key | Action |
|-----|--------|
| `h` / `l` | move cursor left / right (columns 0–15) |
| `j` / `k` | move cursor down / up (rows 0–3, clamped) |
| `x` | toggle step at cursor |
| `dd` | clear all steps on the current track |
| `gg` | reset playhead to step 0 |
| `+` / `-` | increment / decrement BPM |
| `space` | play / pause (suspend/resume the dispatch timer) |
| `q` / F2 | quit |

Cursor row 3 maps to track 0 (`cursor_y - 3` is the track index).

## Building

```sh
# macOS
gcc main.c -lncurses -lrtmidi && ./a.out

# Ubuntu (clang required for Blocks extension)
clang -fblocks main.c -ldispatch -lncurses -lrtmidi -lBlocksRuntime && ./a.out
```

### Dependencies

- rtmidi
- libdispatch (GCD — pre-installed on macOS; `libdispatch-dev` on Linux)
- libblocksruntime (Linux only, for Clang Blocks)
- ncurses

### macOS

GCD is pre-installed. Install the rest with Homebrew:

```sh
brew install rtmidi ncurses
```

### Ubuntu

Use `clang` with `-fblocks` instead of gcc. Install:

```sh
apt install libdispatch-dev libncurses-dev librtmidi-dev libblocksruntime-dev
```

### cross compile notes for Windows

https://stackoverflow.com/questions/44389963/how-to-install-mingw32-on-ubuntu
