# Sega Model 1 — Reverse-Engineering Documentation

This directory documents the Sega Model 1 hardware and the Virtua Fighter software as
reverse-engineered against this core and a real-coprocessor reference. The goal is a complete
description of how the game works — the V60 main program, the TGP geometry coprocessor, the data
formats, and the game logic — sufficient to eventually drive a static recompilation that needs no
V60/TGP/68000 emulation.

## System documents

- **ARCHITECTURE.md** — System overview: hardware, the V60↔TGP memory-mapped interface, the full TGP
  command set (the geometry coprocessor's instruction set), the skeletal/vmat animation bank, the
  display list and renderer, the chain-setup sequence, and per-game notes (VF/SWA/VR).
- **TGP_DESIGN.md** — The MB86233 ("TGP") coprocessor in detail: register/memory model, addressing
  modes, instruction classes, the ALU operations, the math units, and the VF geometry firmware
  (boot, command dispatch, the matrix-build handler, the V60-side result format).
- **DISPLAY_LIST_FORMAT.md** — The display-list and geometry data formats: opcodes, the polygon
  record layout, the flags bitfield, the rendering pipeline, and the color/lighting pipeline, with
  regression anchors.
- **RECOMPILATION.md** — The recompilation reference: the three processors to translate, the complete
  V60 memory map, the interrupt/timing model, the per-frame V60→TGP→display-list→render flow, the V60
  control-flow/function map (boot, init, main loop), the game state machine and work-RAM layout, the
  character entity struct, and the move/animation system.

## Worked example: the Virtua Fighter hair bone-chain (braid)

`braid_reverse_engineering/` contains the deep RE of the VF hair bone-chain, used as the worked
example that drove much of the system reversing:

- **TGP_braid_RE.md** — Master RE notes: root cause, the FIFO/matrix diffs, the forward-kinematics
  analysis, the firmware decode, and the feasibility analysis for a bit-exact fix.
- **ground_truth.md** — Ground-truth comparison notes (real-coprocessor reference vs. HLE).
- **v60_braid_routine.txt** — Disassembly of the real V60 skeletal-chain driver routine.
- **braid_command_sequence.txt** — The captured V60→TGP command stream for one chain render.
- **braid_chain_ram_9500.txt** — The 25-word joint block the chain reads from coprocessor RAM.
- **\*_f1200.txt / coord_diff / f102_args_parent.txt** — Captured coordinate/matrix datasets used to
  diff the HLE against the reference.

These notes are working documents; later sections supersede earlier ones where they conflict.
