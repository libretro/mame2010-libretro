# Model 1 / Virtua Fighter Per-Character Move Catalog

This document describes the per-character command->move table used by the move
trigger FEAB4C (see RECOMPILATION.md sections W, X, Y).  It gives the table
location, the per-character pointer array, and the verified 14-byte entry layout,
so the recompilation can map an input command + game state to a move-state ID
without running the V60 matcher.

## A. Tables

    FDB820   per-character pointer array: one 32-bit pointer per fighter, each
             pointing at that character's move table.  Entries are 0x15E apart.
    FDB850   condition mask table (which entity state fields a condition tests)
    FDB854   condition expected-value table (the value to match)

Verified pointer array (Virtua Fighter, 8 fighters + extras):

    [0] -> FDB8A0   [1] -> FDB9FE   [2] -> FDBB5C   [3] -> FDBCBA
    [4] -> FDBE18   [5] -> FDBF76   [6] -> FDC0D4   [7] -> FDC232
    [8] -> FDC390   [9] -> FDC4EE  [10] -> FDC64C  [11] -> FDC7AA

Each table is 0x15E (350) bytes = 25 entries of 14 (0x0E) bytes.

## B. Entry layout (14 bytes, verified)

    +0  (word)   move-state ID, e.g. 022f, 0338, 01a6, 0117, 0105.  This indexes
                 the same unified move-state space as the move-index table and the
                 keyframe pointer table (RECOMPILATION.md section Y).
                 A value of FFFF marks an EMPTY command slot (no move bound to that
                 command position) and acts as a group separator.
    +2  (byte)   condition byte 0 (stick / direction sub-condition; 00 = none)
    +3  (byte)   condition byte 1
    +4  (byte)   condition byte 2 (e.g. 02 = a directional-input sub-state)
    +5  (byte)   cancel-window / frame threshold: the current-animation-frame
                 (entity -0x32) gate for the move; common values 0x0C, 0x0E match
                 the frame thresholds tested by the command interpreter.
    +6  (byte)   00 (reserved / high byte of the frame field)
    +7..+10      condition value block (4 bytes).  FF FF FF FF = wildcard (no state
                 gating); a specific value such as 65 00 00 00 gates on a self or
                 opponent state field (selected via the FDB850/FDB854 tables) ANDed
                 against entity 0x156D / -0x36 (self) and opponent -0x36.
    +11..+13     move modifier / flags (e.g. 00 10 00).

## C. How a command resolves to a move

The trigger FEAB4C, called from ~20 sites in the command interpreter
(FE8800-FE8F72), is given the character index from entity[-0x50+0xC], looks up the
character's table via the FDB820 array, and walks the 14-byte entries for the
command group that matches the player's current input (stick sequence + buttons,
section W).  For each candidate entry it:

1. checks the cancel-window byte (+5) against the current animation frame
   (entity -0x32);
2. checks the condition value block (+7..+10) -- FF wildcard always passes;
   otherwise the masked entity state fields must equal the expected values;
3. on the first match, stores the entry's move-state ID (+0) into entity[-0x4A],
   which the move loader (FEE1FD) then turns into the active move.

Groups with more than one non-FFFF entry are the same input mapping to different
moves depending on state (e.g. standing vs crouching vs the cancel window of a
previous move) -- the gating in steps 1-2 picks the right one.

## D. Representative char-0 entries (verified dump)

    @FDB8A0  state=022f  frame=0E  cond=wildcard          (a neutral command)
    @FDB8AE  state=0338  frame=0C  cond=wildcard
    @FDB8BC  state=01A6  frame=0C  cond=wildcard
    @FDB8CA  state=0117  frame=0E  cond=65 00 00 00  mod=10 (state-gated)
    @FDB8D8  FFFF (empty slot / group separator)
    @FDB8E6  state=0105  c0=0F c2=02  frame=0E  cond=wildcard (directional)
    ...

The FFFF slots are unused command positions in the per-character command grid; the
non-FFFF entries are the character's actual moves with their input/cancel/state
gating.

## E. Recompilation notes

- Build the same per-character array of 25 x 14-byte entries from the ROM image;
  the move-state IDs are directly usable as indices into the move-index and
  keyframe tables (sections Y, AA, and KEYFRAME_FORMAT.md).
- The matcher is a linear scan of the command group's entries applying the
  cancel-window (+5 vs entity -0x32) and the masked state comparison (+7..+10 via
  the FDB850/FDB854 mask/value tables); the first passing entry wins.
- FFFF entries are skipped (unused slots).  Per-character human-readable command
  notation (e.g. P, K, G, df+P) is not stored in ROM -- it is implied by the
  command group's position and the stick/button decode in section W; mapping a
  given entry to its notation requires correlating the group index with the
  interpreter's input decode, which is character-independent.
