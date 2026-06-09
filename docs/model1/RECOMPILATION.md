# Sega Model 1 / Virtua Fighter — Static Recompilation Reference
Goal: translate the V60 + TGP code to native code, running the game with NO V60/TGP/68000 emulation.
This documents every hardware boundary, entry point, and code region a recompiler must handle. Pair
with: MODEL1_ARCHITECTURE.md (system), MODEL1_DESIGN.md (TGP CPU/ISA), MODEL1_FORMAT.md (data formats),
MODEL1_TGP_braid_RE.md + v60_braid_routine.txt (worked example).

## A. Processors to recompile (3 code streams)
1. **V60 main CPU** (NEC uPD70615, 16 MHz). Game logic, I/O, builds display lists, drives the TGP.
   ROMs: epr-16080.4 @0xfc0000 (0x20000), epr-16081.5 @0xfe0000 (0x20000). Reset vector at 0xfffffff0
   region; observed entry jmp at 0xfe0000 (boot). This is the PRIMARY recomp target.
2. **TGP geometry coprocessor** (Fujitsu MB86233, on Sega 315-5571/5572). Internal program ROM
   315-5571.bin (0x2000 words). Does all 3D math. Recomp target #2 (or keep as a faithful C library
   of its command handlers — see section E).
3. **Sound CPU** (Toshiba TMP68000, 10 MHz). ROM 0x000000-0x0bffff. Drives 2x MultiPCM + YM3438.
   Independent; communicates with V60 via latches. Recomp target #3 (or HLE the sound).

## B. V60 memory map (what the recomp must model for every load/store)
  0x000000-0x0fffff  ROM (program/data, low)
  0x100000-0x1fffff  ROM bank "bank1" (banked via 0xe00004 bank_w; selects maincpu+0x1000000+0x100000*n)
  0x200000-0x2fffff  ROM
  0x400000-0x40ffff  RAM (mr2)            - work RAM
  0x500000-0x53ffff  RAM (mr)             - work RAM (larger)
  0x600000-0x60ffff  Display list 0 (md0_w)  } double-buffered display list the TGP/render consumes
  0x610000-0x61ffff  Display list 1 (md1_w)  }
  0x680000-0x680003  listctl (model1_listctl_r/w) - display-list buffer select/swap + render trigger
  0x700000-0x70ffff  sys24 tilemap (text/2D layer)
  0x720000/0x740000/0x760000/0x770000  video sync regs (write-nop in HLE)
  0x780000-0x7fffff  sys24 char RAM (tile graphics)
  0x900000-0x903fff  palette RAM (p_w)
  0x910000-0x91bfff  color translation table (model1_color_xlat)
  0xc00000-0xc0003f  I/O read (io_r): inputs/DIPs; write-nop
  0xc00040-0xc00043  network ctl (link play)
  0xc00200-0xc002ff  NVRAM (battery-backed settings)
  0xc40000           sound latch to 68k (snd_latch_to_68k_w)
  0xc40002           sound 68k ready (snd_68k_ready_r)
  0xd00000-0xd00001  TGP copro ADDRESS port (copro_adr_r/w)  - model-ROM read pointer
  0xd20000-0xd20003  TGP copro DATA RAM window (copro_ram_r/w)
  0xd80000-0xd80003  TGP copro COMMAND/RESULT FIFO (copro_w write cmd, copro_r read result), mirror +0x10
  0xdc0000-0xdc0003  FIFO-in status (fifoin_status_r) - V60 polls this to pace the TGP
  0xe00000-0xe00001  watchdog / IRQ-ack (write 0x20 on IRQ)
  0xe00004-0xe00005  ROM bank select (bank_w)
  0xe0000c-0xe0000f  write-nop
  0xfc0000-0xffffff  ROM (the main program incl. the TGP-driver routines like FFC764 braid chain)

## C. V60 interrupts (recomp timing model)
  - VBLANK: model1_interrupt runs 2x per frame (MDRV_CPU_VBLANK_INT_HACK ...,2).
    iloop!=0 -> irq_raise(level 1)  = the RENDER/geometry interrupt (main per-frame work).
    iloop==0 -> irq_raise(sound_irq) (level 0 for VF, 3 for VR/SWA) + signal 68k (line 2) if sound FIFO
      non-empty.
  - irq_callback returns last_irq (the V60 reads the level). 0xe00000 write = IRQ ack.
  A recomp models: per-frame, run V60 main-loop body, service IRQ1 (build+submit display list via TGP),
  then render the resulting display list.

## D. The per-frame flow (V60 -> TGP -> display list -> render)
  1. V60 game logic updates object/skeleton state in work RAM (mr/mr2).
  2. On IRQ1, V60 walks the scene; for each object it issues TGP commands (matrix push/transform/...,
     section 3 of MODEL1_ARCHITECTURE) over the 0xd80000 FIFO, polling 0xdc0000 for pacing.
  3. matrix_read (cmd 0x11) makes the TGP return the 12-word world matrix; V60 writes it to the active
     display list (0x600000/0x610000) as a 0xB command, followed by object draw commands (0x2/4/6).
  4. listctl swaps buffers; the renderer walks the completed display list and rasterizes (the renderer
     itself is hardware, modeled by MODEL1_FORMAT.md's pipeline; a recomp ports it as a SW/GL rasteriser).

## E. TGP recompilation (the hard part)
The TGP is HLE'd in MAME2010 via ftab_vf (104 command handlers). For a recomp there are two routes:
  (route 1) Recompile the MB86233 microcode (315-5571.bin) to native, driven by the FIFO command words.
            Requires the full ISA (MODEL1_DESIGN.md: register/mem model, addressing, ALU, math units)
            and the command dispatch (FIFO cmd>>23 -> handler). The chain/skeletal commands (mve_calc)
            carry internal state (the R21 6-word accumulator) that must be modeled.
  (route 2) Reimplement each TGP command as a faithful C function (extend ftab) verified bit-exact
            against the real-DSP oracle. Most commands are simple matrix/vector math (exact already);
            the open work is the chain integration (mve_calc) — see MODEL1_TGP_braid_RE.md. This route
            is closer to MAME's HLE but made bit-exact per command.
A recomp that drops ALL emulation = route 1 for V60 + route 1 for TGP + route 1-or-HLE for sound 68k.

## F. Recompilation work checklist (status)
  [done] V60 memory map (section B) — complete.
  [done] V60 interrupt/timing model (section C) — complete.
  [done] Per-frame V60->TGP->displaylist->render flow (section D) — complete.
  [done] TGP ISA + ALU + addressing (MODEL1_DESIGN.md) — verified bit-identical to reference.
  [done] TGP command set (104, MODEL1_ARCHITECTURE.md section 3) — enumerated.
  [done] Display-list / geometry / polygon / color format (MODEL1_FORMAT.md) — verified.
  [done] Chain-setup + per-link sequence (v60_braid_routine.txt; ARCHITECTURE section 9) — disassembled.
  [WIP ] mve_calc chain-integration exact math (the R21 accumulator transform, copro 0x62a) — decode in
         progress; the one TGP command not yet bit-exact (braid detachment symptom).
  [todo] Full V60 ROM control-flow map (entry, main loop, per-mode handlers) — partially done (TGP
         driver routines @ FFC7xx mapped); the game-logic body needs a full disassembly pass.
  [todo] Sound 68k program map + MultiPCM/YM3438 command protocol.
  [todo] sys24 tilemap + palette + color-xlat exact behavior (2D layer) for the recomp rasteriser.

## G. V60 control-flow map (disassembled from epr-16081.5, for the recompiler's function graph)
Reset/boot (entry 0xfe0000):
  FE0000: jmp FE39F8            ; (reset lands here via vector; early redirect)
  FE0006: updpsw #0,#40000      ; set PSW (mask IRQs during init)
  FE000E: jsr FE3BFC            ; init subsystem 1
  FE0014: jsr FE3C7E            ; init subsystem 2
  FE001A: jsr FF45CC            ; init subsystem 3
  FE0020: mov #D80000,R23 ; mov #D80000,R24   ; set TGP FIFO ports: R23=read, R24=write (used everywhere)
  FE002E: poll DC0000 / in [R23] loop          ; TGP handshake: drain FIFO until ready (DC0000 status)
  FE0041: mov #4000000,[R24]    ; first TGP command (stack/clear region)
  FE0049..FE0094: chain of init jsr's (FE3E05, FE42C5, FE4B7B, FE3D86, FF4685, FF3DCC, FF3B48, FE44F9,
                  FE3C14, FE4746, FFDB85, FC91CE, FCC9B8) - subsystem/table init.
  FE009A..FE00BA: zero work-RAM globals (0x501139, 0x5011E4, 0x5011EC, 0x501188, 0x501189) [RAM@0x500000].
  FE00C2: updpsw #FFFFFFFF,#40000   ; ENABLE interrupts (IRQ1 vblank now fires)
MAIN LOOP (FE00CE+):
  FE00CE: bsr FE03A2            ; wait-for-sync / frame gate
  FE00D1: mov #189C,5011B0      ; set frame/mode global
  FE00DD: jsr FE3DAD            ; per-frame work
  FE00E3: bsr FE03A2            ; wait again
  FE00E6: jsr FE3E40            ; returns mode/result in R0
  FE00EC: test R0; blt FE039C; bgt FE0000   ; dispatch: <0 -> FE039C handler, >0 -> reboot, 0 -> fall through
  (FE00F4+: mode setup - clears 40BF00, sets 500514, loads tables via FF4685/FF4EFD with src ptrs
   #2E0308/#2E0390 etc - these are attract/menu/game mode handlers.)

KEY RECOMP ENTRY POINTS (V60 functions to translate first):
  FE0000  reset/boot + main loop
  FE03A2  frame-sync wait (IRQ1 gate)
  FE3E40  mode poll (returns dispatch code)
  FFC764  TGP skeletal-chain driver (braid etc.) - see v60_braid_routine.txt
  FFC790  per-link mve_calc issue+readback
  IRQ1 handler: (vector TBD) builds the scene's display list via TGP commands each frame.

NOTE: work RAM globals live at 0x500000+ (0x5011xx = frame/mode state). The TGP FIFO ports D80000
(R24 write / R23 read via the 0xd80000 region) and status DC0000 are the V60<->TGP boundary the recomp
replaces with direct calls into the recompiled/HLE'd TGP.

## H. Recomp strategy summary
A no-emulation build = (1) recompiled V60 functions (graph rooted at FE0000, section G) with memory
accesses lowered to native (RAM arrays for 0x400000/0x500000, ROM as const, MMIO as function calls);
(2) the TGP FIFO writes (to D80000) lowered to direct calls into a recompiled/bit-exact TGP command
library (sections E + MODEL1_DESIGN/FORMAT); (3) sound 68k similarly recompiled or HLE'd; (4) the
hardware rasteriser reimplemented per MODEL1_FORMAT.md. The remaining bit-exactness gap is the single
mve_calc chain transform (WIP); everything else is mapped.

## I. Game logic (VF fight engine) — reverse-engineering status & method
INPUT HARDWARE (decoded, exact):
  - Read via io_r at 0xc00000 region (V60). 0xc00000-0x0f = 8 analog AN0-7 (VR steering/pedals; VF
    unused). 0xc00010/12/14 = digital IN0/IN1/IN2 (active-low).
  - VF control map (the fight-engine input bits):
      IN0 (0xc00010): bit0 COIN1, bit1 COIN2, bit3 SERVICE, bit4 START1, bit5 START2.
      IN1 (0xc00012): P1 - bit0 GUARD(Btn1), bit1 PUNCH(Btn2), bit2 KICK(Btn3),
                      bit4 DOWN, bit5 UP, bit6 RIGHT, bit7 LEFT (8-way stick).
      IN2 (0xc00014): P2 - identical layout.
  So VF is the classic 3-button (Guard/Punch/Kick) + 8-way joystick scheme. A recomp maps host
  controller -> these bits -> the 0xc00010-14 reads.

CODE LOCATION:
  - Game-logic code lives in epr-16080.4 (@0xfc0000). epr-16081.5 (@0xfe0000) holds boot + the TGP
    driver routines (FFC7xx) + display-list building. (Confirmed: boot/main-loop disassembles cleanly
    at 0xfe0000; the lower ROM is the bulk of game logic + data tables.)

METHOD NOTE (important for the recomp effort):
  - The V60 uses register-relative / PC-relative / pointer-table addressing for I/O and state, NOT
    flat absolute immediates. Scanning the ROM for I/O address immediates (e.g. 0xc00010) yields FALSE
    POSITIVES (coincidental byte alignment in data/code), so it does NOT reliably locate the input-read
    code. The correct method is CONTROL-FLOW TRACING from real entry points: follow the boot init chain
    (section G) and the IRQ1 handler, decoding each jsr/bsr target, building the call graph. The
    dual-ROM disassembler (v60dis, now loads BOTH epr-16080.4@0xfc0000 + epr-16081.5@0xfe0000) is the
    tool; v60dis <start> <end> from an instruction-aligned entry.

FIGHT-ENGINE STRUCTURE (to be mapped by control-flow tracing — framework):
  Per frame (driven by IRQ1, section C/D):
    (1) sample inputs (0xc00010-14) into per-player input state in work RAM (0x500000+),
    (2) run per-player state machine: parse stick+button into MOVES (the VF command interpreter -
        directional inputs + P/K/G into attack/throw/block states), advance the current animation,
    (3) physics/positioning (stage bounds, pushback, ring-out),
    (4) HIT DETECTION (the TGP colbox_set/colbox_test/col_testpt commands - section 3 of ARCHITECTURE -
        are the engine's collision primitives; the V60 sets hit/hurt boxes per animation frame and
        tests them via the TGP), apply damage/stun,
    (5) update each character's SKELETON (joint matrices in the vmat bank; body parts + hair chains)
        for rendering,
    (6) build the display list (issue TGP transform+matrix_read per object) -> render.
  The match/round flow (intro, round timer, KO, win poses, continue) is the outer state machine around
  this, dispatched from the main loop (FE3E40 mode poll, section G).

STATUS: input hardware exact; code region identified; boot+main-loop control flow mapped (section G);
the fight-engine function graph (state machine, move interpreter, hit detection) requires a control-
flow disassembly pass from IRQ1 - that is the main remaining game-logic reversing work. The collision
primitives are already enumerated (TGP colbox/col commands); the move-interpreter + animation tables
in epr-16080.4 are the next disassembly target.

## J. V60 function map (control-flow traced, expanding the recomp call graph)
Verified by disassembly (dual-ROM v60dis). Reset/boot validated end-to-end:
  RESET VECTOR: 0xFFFFF0: jmp FE0000   (confirms boot entry; reset -> FE0000).

Functions mapped so far (recomp translation units):
  FE0000  boot + main loop (section G). updpsw, init chain, enable IRQ, loop: sync/work/dispatch.
  FE39F8  early init: jmp FE3A06.
  FE3A06  HARDWARE/RAM init: point R21=0x400000, clear 0x6800 bytes work RAM, configure the E00000
          control block (writes 0x10,0x00,0xFF,0x38 to E00000-3 = watchdog/IRQ/timer setup).
  FE3E4C  VIDEO init: write terminator 0xF to both display lists (0x600000 & 0x610000); config display
          regs at 0x680000+2/+0 (0x1F, 0x85); clear sys24 tilemap (0x700000, 0x5008 words) + a 0x1000
          region at +0xC000; copy palette table from ROM 0xFD0770 to palette RAM 0x900000 (0x10 words).
  FE3DAD  ASSET STREAMING: loop 96 times - set ROM bank (E00004 = 0x31..), copy 0x1000 words from
          banked ROM (0x100000 bank window) to the display-list/FIFO ([FP+]), with sync (FE03A2)
          between banks; restore bank 1. Streams the model/texture ROM banks to the geometry/video.
  FE03A2  frame-sync wait (IRQ1 gate; called throughout).
  FE3E40  mode poll (returns dispatch code in R0; main loop branches on it).
  FE3E05  (table init: writes ASCII 'SE..' bytes to C00034/36 region - a string/header setup.)
  FFC764  TGP skeletal-chain driver (braid) -> FFC790 per-link (v60_braid_routine.txt).
  Init subsystem calls from boot (targets to translate): FE3BFC, FE3C7E, FF45CC, FE3E05, FE42C5,
  FE4B7B, FE3D86, FF4685, FF3DCC, FF3B48, FE44F9, FE3C14, FE4746, FFDB85, FC91CE, FCC9B8.

OBSERVED HARDWARE-REGISTER SEMANTICS (for the recomp's MMIO model):
  E00000-3 : control block - 0x10/0x00/0xFF/0x38 written at init (watchdog/IRQ/timer config).
  E00004   : ROM bank select (bank_w) - 0x31 during asset streaming, 0x01 normal. Banks the 0x100000
             window to maincpu+0x1000000+0x100000*((val>>4)&0xf).
  680000   : display list control - +0 and +2 take 0x85 / 0x1F (buffer/mode); the listctl swap.
  600000/610000 : the two display-list buffers; 0xF = list terminator.
  900000   : palette RAM (loaded from ROM table 0xFD0770 at init).
  700000   : sys24 tilemap (cleared at init).

NEXT (game-logic body): trace the mode-handler targets that the main-loop dispatch (FE3E40 result)
branches to - those lead to attract/menu vs the in-match fight loop. The fight loop is where input
sampling (0xc00010-14), the move interpreter, and the TGP collision calls live. Continue decoding the
boot init-subsystem call targets + the FE00F4+ mode setup to reach the match state machine.

## K. Game state machine & work-RAM layout (control-flow traced from main loop)
GAME STATE DISPATCH (the mode/scene state machine):
  - State byte at work RAM 0x40FF38 indexes a pointer table at ROM 0xFD2F2D (in epr-16080.4).
  - Table entries (32-bit LE handler addresses), 8 states:
      [0]=0xFD2EF7  [1]=0xFD2F00  [2]=0xFD2F09  [3]=0xFD2F12  [4]=0xFD2F1B  [5]=0xFD2F24
      [6]=0xFD95A1  [7]=0xFC72EC
    The first six are 9-byte-spaced small stubs (likely jmp trampolines to the real handlers); FD95A1
    and FC72EC are larger handlers. These are the top-level game states (attract / title / character-
    select / match / result / continue, etc - exact mapping by decoding each handler).
  - Mode-setup (FE00F4+): loads data tables from the user2 data ROM (0x2E0000-0x2FFFFF) via the table
    loader FF4F18(src R0, idx R1, count R2) - tables at 0x2E0308, 0x2E0390, 0x2FF630, 0x2FF6D0; sets
    state flags (0x500514, 0x501296), seeds camera/physics float params (see below), then dispatches.

WORK-RAM VARIABLE MAP (discovered; the recomp's game-state struct):
  0x400000-0x40ffff : main work RAM (mr2).
    0x40B200 = 0.011f, 0x40B204 = 1.0f  : camera/scale params.
    0x40BF00, 0x40BFA0 : state/control bytes.
    0x40FF36, 0x40FF38 : game STATE selectors (40FF38 = the dispatch index above).
  0x500000-0x53ffff : larger work RAM (mr).
    0x500514, 0x501150, 0x501283, 0x501296 : state flags.
    0x501D60=0.17f, 0x501D64=0.04f, 0x501D6C=0.49f, 0x501D68=0xF700, 0x501D6A=0 : camera/positioning.
    0x50126C : current sub-state pointer (loaded from table FD2F2D-indexed). 0x5012A4, 0x501F00 : counters.
    0x50130C : entity/structure base pointer (used by the per-state handler).
  These regions are zero-cleared at boot (FE3A06 clears 0x6800 bytes from 0x400000).

PER-STATE / ENTITY HANDLER (FF5FA1, an in-match handler):
  - Sets TGP FIFO ports (R23=0xD80000 read, R24=0xD80010 write).
  - Reads entity-table base (0x50130C -> R19), loads a per-entity sub-handler pointer from table 0xFD2EA3,
    calls FEABD1 (entity setup), then per-entity processing via FF6062 / FF607B (bsr). Writes entity
    flags (0x158F[R25]=1) and reads 0x1594[R25] (per-entity struct fields at offsets 0x158F/0x1594).
  - This is the entity/character processing loop: for each active entity, run its handler -> issue TGP
    transforms -> emit to display list. The fight characters (P1/P2) are entities here.

NEXT: decode the 8 state handlers (FD2EF7.. / FD95A1 / FC72EC) to label the states (attract/select/
match/...); decode the in-match handler chain (FF6062/FF607B/FEABD1) to reach input sampling + the
move interpreter + the TGP colbox hit-detection. The per-entity struct (fields at 0x158F/0x1594/...)
is the character state block (position, facing, animation frame, health) - map its layout next.

## L. Verified entity->TGP processing pipeline (control-flow, code-confirmed)
CORRECTION: FD2EA3 and FD2F2D are DATA (a pointer constant stored into the entity struct, and a 9-byte-
record data table), NOT handler tables. (Disassembling them as code gives garbage; FF5FB6 loads FD2EA3
as a 32-bit IMMEDIATE into struct offset -50.) Earlier table-pointer guesses are retracted. Only code
reached via verified jsr/bsr from clean-disassembling anchors is trusted below.

ENTITY HANDLER FF5FA1 (verified code) - processes one character/entity per call:
  - R23=0xD80000 (TGP read port), R24=0xD80010 (TGP write port). R19 = entity base (from 0x50130C).
  - Stores data ptr FD2EA3 into entity[-50]; reads sub-struct ptr from entity[-50+4] -> R17.
  - jsr FEABD1 (entity setup). Sets entity[0x158F]=1; reads entity[0x1594] (a component-buffer ptr) -> R4.
  - For each body component (fields at R17+0, +4, +0x14C, then a 10-entry loop over table @0x53B000 via
    FF5F6F index): bsr FF6062 ; bsr FF607B / FF609E.
  -> So a character is a set of components; each is transformed via the TGP and copied to the display
     buffer. The "10-entry loop" = the character's ~10 body segments (matches the skeletal model).

CORE PRIMITIVES (verified code):
  FF6062 : pushm; jsr FEE054; jsr FEDB20; popm; rsr.  Runs two transform/compute steps (FEE054, FEDB20)
           on the current component. (FEE054/FEDB20 = the matrix-build + projection helpers - decode next.)
  FF607B : block copy from entity field (-10[R25] -> R3) into work buffer R4: 3 longwords + a 0x28(40)-
           iteration half-word loop. = copies a transform matrix + component data into the render buffer.
  FF609E : the per-component TGP TRANSFORM call. Writes TGP command 0x800000 to FIFO [R24], sends two
           operands ([R12], [R5+]), then reads the transformed result back: in.w [R23],[R12+]. Repeated
           per coordinate. 0x800000>>23 = 1 = TGP cmd index 1 (transform/anglev-class op) - the entity's
           vertices/points get transformed by the TGP and read back here.

So the per-frame character render path (recomp-relevant) is:
  state dispatch -> FF5FA1 per entity -> per component { FF6062 (build/compute) ; FF607B (copy matrix) ;
  FF609E (TGP transform each point, cmd 0x800000, read back) } -> results land in the display buffer.

NEXT: decode FEE054 + FEDB20 (the matrix-build/projection helpers) and FEABD1 (entity setup) - these
plus the component table @0x53B000 define how a character's skeleton maps to TGP transforms. Then locate
the GAME-LOGIC update (input -> move -> animation-frame -> which component table) that runs BEFORE this
render pass; it sets the entity struct fields (position/facing/anim-frame) that FF5FA1 consumes.

## M. Character entity struct & animation logic (verified code)
The character/entity state block is pointed to by R19 (base) / R25 (working ptr) throughout the entity
functions (FF5Fxx, FEE054, FEDB20, FEABD1). It is a large struct (offsets from -0x80 to +0x1714).
Verified field accesses + decoded meanings:

  Entity struct (negative offsets = header/state, large positive = per-component/animation data):
    -0x80 : flags word (FEABD1 inits to 0x80000000; FF5F17 tests bit 0x15 to pick behavior FF6530 vs
            FF6552 = entity-type / active dispatch).
    -0x50 : data pointer (set to FD2EA3 const; +4 = sub-struct ptr R17 = component list base).
    -0x3C : active/enable flag (FEDB20 tests it; if 0 -> skip to FEDFA6).
    -0x32 : CURRENT ANIMATION FRAME/timer (FEDB20: cmp #1; cmp vs 0x650; subtract 0x650; store back).
    -0x38 : a second anim/state counter (compared vs 0x671).
    -0x36, -0x2C, -0x2E, -0x46, -0x54, -0x5A, -0x5E, -0x62, -0x64, -0x76, -0x7C, -0x10, -0x8, -0xC :
            state/position/velocity fields (further decode needed for exact semantics).
    +0x4C, +0x54, +0x60A, +0x640, +0x650, +0x671, +0x10C : animation parameters
            (0x650 = anim length/period; 0x671 = anim sub-param; used by the frame-advance in FEDB20).
    +0x158F : "processed" flag (FF5FA1 sets =1). +0x1594 : component render-buffer ptr.
    +0x14B1, +0x1567, +0x156B, +0x16DC, +0x16E5, +0x1714 : per-component / skeletal data.

ANIMATION ADVANCE (FEDB20, verified): if entity[-3C] active, advance entity[-32] (current frame) by
comparing/subtracting entity[0x650] (anim period) - i.e. frame counter with wraparound; entity[0x671]
gates a sub-state. This is the per-character ANIMATION update, run inside the entity processing each
frame (called via FF6062). FEE054 is the companion transform/matrix-build step (tests entity[-36] flags).

So the per-frame character pipeline (game-logic + render, interwoven in the entity system):
  FF5FA1(entity): setup (FEABD1) -> for each component: FF6062 { FEE054 (build transform from anim
  state) ; FEDB20 (advance animation frame) } ; FF607B (copy matrix to buffer) ; FF609E (TGP transform
  the component's points, cmd 0x800000, read back) -> display buffer.

The MOVE INTERPRETER (input -> which animation/move) sets entity[-32]=0 + entity[0x650]=new anim period
+ the component table for the new move; it runs from the per-state game handler BEFORE/around this. The
input bits (0xc00010-14, section I) feed it. Locating the exact move-select code (which reads inputs and
writes entity[-32]/[0x650]) is the next target; the animation playback (FEDB20) is now mapped.

This entity struct IS the character state a recomp must model: per-fighter, holds flags, current move/
animation frame + period, position/velocity, and the skeletal component list. Two instances (P1/P2)
plus the camera/stage entities, iterated by the state handler.

## N. Move / animation system (verified code + data)
The fight engine's moves are a DATA-DRIVEN animation system. Verified call chain & data:

ANIMATION PLAYBACK STATE MACHINE (FEDB20, verified):
  - entity[-0x32] = current frame; entity[0x650] = period; entity[0x647] = anim MODE (tb/cmp #2/#3
    dispatch -> loop / once / transition modes); entity[0x671/0x673/0x675/0x712] = anim sub-counters.
  - Steps the frame; on completion (FEDB88: reload -32 from 0x650) calls FEE1C9 (keyframe advance).
KEYFRAME TRACKER (FEE1C9, verified): caches prev state in entity[0x44C]/[0x44E]; on change calls
  keyframe handlers FF5B59 / FF597C.
MOVE LOADER (FEE1FD, verified) - the move-select target:
  - Input: move ID in R20. `and #0x3FFF` (14-bit ID), `dec`, index the MOVE-INDEX TABLE at 0xFC0000
    (16-bit offsets), `add #0xFC06E4` -> pointer to the move's DATA record; store in entity[-0x3C]
    (the active-animation pointer that FEDB20 plays). Then zero anim sub-counters (0x671/0x673/0x675/
    0x712). I.e. "start move N on this character."

MOVE DATABASE (verified data, epr-16080.4):
  - 0xFC0000 : MOVE-INDEX TABLE - array of 16-bit offsets. move[N] data @ 0xFC06E4 + table[N-1].
    Decoded entries: move1->0xFC0C80, move2->0xFC1402, move3->0xFC0E07, ... (each a distinct move).
  - 0xFC06E4 : MOVE DATA region - per-move animation records (keyframe/timing/parameter words; e.g.
    move data begins 0400 0000 001E 0008 4700 0013 ... = frame params + keyframe stream). Each record
    drives FEDB20 playback (period 0x650, mode 0x647, etc. are filled from here).

So a "move" = an entry in the 0xFC0000 index -> a data record at 0xFC06E4+ describing the animation
(frames, timing, and—per the engine—the per-frame skeletal poses + hit/hurt box activation). The MOVE
INTERPRETER reads inputs (0xc00010-14, section I), decides a move ID, and calls FEE1FD(moveID) to start
it; FEDB20 then plays it each frame; FF5FA1/FF609E render the resulting skeleton via the TGP.

RECOMP DATA STRUCTURES now identified:
  - character entity struct (section M): runtime state.
  - move-index table @0xFC0000 + move data @0xFC06E4 (this section): static move/animation database.
  - the 8-state game dispatch (section K): outer flow.
NEXT: the move-SELECT logic (input bits -> move ID passed to FEE1FD) - the command interpreter that
turns stick+P/K/G sequences into move IDs (VF's input-buffer/command system). It reads the per-player
input state (sampled from 0xc00010-14) and the current entity state, and calls FEE1FD. Trace callers of
FEE1FD to find it. Also: decode one full move data record (0xFC0C80) to document the move/keyframe
format (frames, hitboxes, cancel windows) - the last big data format for the recomp.

## O. Move loader contract & move record (verified code + data, fully decoded)
MOVE LOADER FEE1FD(moveID in R20) — complete, verified:
  - R20 & 0x3FFF = 14-bit move ID (top 2 bits = flags). Range up to 16383 moves.
  - (id-1) indexes the 16-bit MOVE-INDEX TABLE at 0xFC0000; result + 0xFC06E4 = move-data pointer,
    stored into entity[-0x3C] (the ACTIVE-MOVE pointer that FEDB20 plays).
  - Zeros the entity's animation-channel accumulators: entity[0x671,0x673,0x675,0x677,0x679,0x67B,
    0x67D,0x67F,0x681,0x683,0x685,0x687] (12 halfword channels) and entity[0x712]. = reset all bone/
    channel animation state for the new move. (So the character skeleton has ~12 animated channels;
    matches the ~10-component render loop + extras.)
  So "execute move N" = FEE1FD(N): point the entity at move N's data and reset its animation channels.

MOVE RECORD FORMAT (at 0xFC06E4 + index; example move 1 @0xFC0C80):
  Header/param block of 16-bit words, e.g. move1: 2003 0040 010C 0020 C000 0605 050A 0C00 3333 3F33 ...
  - word0 (0x2003) = move type/flags header.
  - subsequent words = animation parameters incl. embedded floats (e.g. 0x3F33xxxx ~ 0.7f as the high
    half of a 32-bit float = movement/velocity/scale), frame counts, and a keyframe/channel stream that
    fills the playback fields (period 0x650, mode 0x647, the 0x671+ channels). Exact per-word semantics
    need a cross-move diff (decode several records and correlate fields with observed playback).

ENTITY ANIMATION-CHANNEL BLOCK (verified): entity[0x671..0x687] = 12 halfword per-channel animation
accumulators; entity[0x712] = an extra channel/flag. entity[-0x3C] = active-move data pointer;
entity[-0x32] = current frame; entity[0x650] = period; entity[0x647] = playback mode; entity[0x44C/
0x44E] = previous-state cache (keyframe tracker FEE1C9).

MOVE-SELECT (input -> move ID): FEE1FD is invoked with a move ID chosen by the command interpreter.
That interpreter is reached via INDIRECT dispatch (function-pointer / jsr [Rn]) so it is not located by
a direct branch-target scan; it reads the per-player input state (sampled from 0xc00010-14, section I)
+ the current entity state and computes the move ID. Locating it requires xref/indirect-call analysis
(the disassembler resolves only direct targets). This is the remaining game-logic gap; the move
EXECUTION path (loader + playback + render) is now fully mapped and verified.

RECOMP STATUS (game logic): boot/init, main loop, state machine, work-RAM layout, entity struct,
animation playback, the move loader, and the move database are all mapped & verified. Open items: the
input->move command interpreter (indirect-dispatched), the exact per-word move-record semantics, and
the TGP collision (colbox) wiring for hit detection.

## P. Collision / hit-detection wiring (verified code)
The TGP provides the collision primitives (ARCHITECTURE section 3); the V60 issues them over the FIFO.
Verified collision routine at ~0xFEF180 (epr-16081.5) - per-frame ground/point collision:
  FEF188: cmd 0x0B000000 (idx 0x16) - matrix op (load rotz/component matrix), arg from [R10].
  FEF19D: cmd 0x0D000000 (idx 0x1A = transform_point) - send a point ([R14],+4,+8), read the
          transformed point back ([R7+],[R8+],[R9+]). = transform the test point into world space.
  FEF1BF: cmd 0x2B800000 (idx 0x57 = groundbox_test) - send 3 coords ([R14+]x3), read 3 results
          (R0,R1,R2). = test the point against the stage GROUND box (floor height / ring boundary).
  FEF206: cmd 0x02800000 (matrix_push) + FEF212: cmd 0x09000000 (idx 0x12 = matrix_trans) - set up the
          next test's matrix.
So the engine, per relevant point (feet, body), transforms it via the TGP then runs groundbox_test to
get floor height / out-of-ring status; the result (R0-R2) drives foot placement, landing, and ring-out.
The character-vs-character hit test uses colbox_set/colbox_test (idx 0x31/0x32) similarly: set hit/hurt
boxes from the current animation frame, test attacker box vs defender box, apply damage on overlap.

VERIFIED TGP COMMAND-WORD ENCODING (cmd = index<<23), confirmed from emitted immediates:
  0x02800000 matrix_push(5)   0x03000000 matrix_pop(6)   0x08800000 matrix_read(0x11)
  0x09000000 matrix_trans(0x12)  0x0B000000 (0x16 matrix_rotz)  0x0D000000 transform_point(0x1A)
  0x19000000 colbox_test(0x32)   0x20000000 col_setcirc(0x40)   0x2B800000 groundbox_test(0x57)
  0x32800000 groundbox_set(0x65)  0x33000000 mve_calc(0x66, VF chain)  0x33800000 mve_setadr(0x67)
  (These confirm the ftab index<<23 dispatch end-to-end: V60 emits index<<23; TGP dispatch does
   cmd>>23 -> ftab[index]. A recomp lowers each FIFO write to the matching TGP-command function call.)

NOTE on false positives: many collision command-word byte-matches in epr-16080.4 fall inside the
move-data region (0xFC06E4+) and are DATA, not code; only emissions verified as 'mov.w #cmd,[R24]' in
the code ROM (epr-16081.5) are real FIFO writes (e.g. the FEF180 routine above).

## Q. Input sampling & edge-detection (verified code, located at runtime)
The per-frame input routine is at 0xFE41CC-0xFE424F (epr-16081.5), found by instrumenting the input
read at runtime (V60 PC fe41e1/e7/f7 read IN2/IN1/IN0). Decoded:
  FE41CC: save last frame's input: 0x40BF90 (current) -> 0x40BF94 (previous).
  FE41DA: R11 = 0xC00016 (just past the digital ports); read backwards via pre-decrement [-R11]:
          FE41E1 IN2 (P2, 0xC00014) -> R2; <<8; FE41E7 IN1 (P1, 0xC00012) -> R2; <<8;
          FE41ED IN0 (system, 0xC00010) byte; <<8; FE41F7 final byte. Assembles all inputs into R2.
  FE41FA: not -> active-high; store to 0x40BF90 (CURRENT input word).
  FE4204: 0x40BF98 = current AND NOT previous = JUST-PRESSED (rising edge). [move-trigger source]
  FE4227: 0x40BF9C = HELD inputs (current).
  FE422E-4F: 0x40BF7C = a toggle/edge-latched input set (XOR logic) for another consumer.
  Also 0x5012C0 |= pressed, masked &0x30 (the START1/START2 bits 0x10/0x20) - coin/start handling.

INPUT STATE VARIABLES (work RAM, the recomp's input block):
  0x40BF90 = current input (active-high; P2<<16 | P1<<8 | system, per the assembly packing).
  0x40BF94 = previous-frame input.
  0x40BF98 = just-pressed (edge) - what the move/command interpreter keys on.
  0x40BF9C = held input.
  0x40BF7C = edge-latched/toggle input set.
  0x5012C0 = start/coin latch (bits 0x10/0x20 = START1/START2).
  Bit layout within each player byte (from section I): b0 GUARD, b1 PUNCH, b2 KICK, b4 DOWN, b5 UP,
  b6 RIGHT, b7 LEFT.

So the MOVE/COMMAND INTERPRETER reads 0x40BF98 (just-pressed) + 0x40BF9C (held) + per-player stick
history to recognize VF command inputs (e.g. punch, kick, throw = P+G, crouch = down+attack, directional
specials) and calls the move loader FEE1FD(moveID) (section O). The interpreter itself is reached via
the per-state entity update; with the input block now located (0x40BF90-9C), the interpreter is the code
that consumes 0x40BF98/9C and writes the entity's move - traceable from here. The input ACQUISITION +
edge-detection (this section) is fully mapped and verified.

GAME-LOGIC RECOMP STATUS: input acquisition+edges (this section), state machine, entity struct,
animation playback, move loader, move database, collision/ground test - all verified. Remaining: the
command-recognition logic that maps 0x40BF98/9C patterns -> move IDs (the VF input-buffer system), and
the per-word move-record semantics.

## R. Input consumers: coin/credit & menu front-end (verified code)
Tracing the readers of the input-state block (section Q) found the FRONT-END input handlers (verified):

COIN / CREDIT handler (FE4820, verified):
  - Reads 0x40BF98 (just-pressed) & 0x808 (service/test-type bits) -> bsr FE48FE/FE48D3 (service menu).
  - Reads 0x40BF9C (held) & 0x3 (COIN1/COIN2 bits) -> per-coin: bsr FE4933 (add credit), using credit
    counters in work RAM 0x40BD00/0x40BD10/0x40BD18 and 0x40FA00/0x40FA04/0x40FA08/0x40FA0C.
  - FE4AAF = coin-timer/credit routine; writes the hardware coin counter at 0xC0001E (clr1/set1).
  So coins -> credits is: read held coin bits -> debounce/timer (FE4AAF) -> increment credit counter.

MENU / MODE navigation (FE42E6, verified):
  - Reads 0x40BF98 (pressed); test bit 0x10 of 0x40BFA0 (START1) -> menu advance.
  - Reads 0x40BF88 and table-branches (tb R0, FE43A6) on it; dispatches on menu state 0x5012AC
    (cmp #1/#2/#3 -> different screens) = the attract/title/character-select navigation state machine.
  0x5012AC = front-end mode/screen state; 0x40BF88 = a processed-input/direction word for menus.

IN-MATCH MOVE INTERPRETER (location note): the per-player FIGHT command interpreter is NOT these
front-end handlers; it runs inside the match state (entity update) and operates on the per-player
packed bytes of the input block (P1 = byte at 0x40BF9x>>8, P2 = >>16; bits b0 GUARD/b1 PUNCH/b2 KICK/
b4-7 stick per section I/Q). It recognizes button+stick patterns and calls the move loader FEE1FD
(section O). It is reached only during the match game-state; to locate it precisely, trap reads of the
P1/P2 input bytes during an actual fight frame (runtime PC trap, as used for the input routine in Q),
rather than the front-end which dominates the attract/menu frames sampled here.

GAME-LOGIC RECOMP MAP (verified, cumulative):
  boot/init (G) -> main loop (G) -> state dispatch (K) -> { front-end: coin (R) / menu (R) } |
  { match: entity update (L,M) -> [move interpreter -> FEE1FD move loader (O)] -> animation playback
  (N) -> skeleton render via TGP (L) -> ground/hit collision (P) }.
  Input acquisition+edges (Q). All blocks verified except the in-match move-interpreter internals
  (located to the match entity-update path; exact pattern-match code is the last item) and the
  per-word move-record semantics.

## S. Move record format (cross-validated from two moves)
Comparing move1 (@0xFC0C80) and move2 (@0xFC1402) header words:
  move1: 2003 0040 010C 0020 C000 0605 050A 0C00
  move2: 0083 0000 0119 0020 C000 0908 0A14 1E00
  word0 = move TYPE/property flags (2003 vs 0083 - per-move).
  word1 = param (0040 vs 0000 - per-move; flag/aux).
  word2 = a per-move ID/param (010C vs 0119; both ~0x100-0x120 - likely animation/pose-set selector).
  word3 = 0020 in BOTH = fixed structural field (count/stride marker).
  word4 = C000 in BOTH = fixed header constant/marker (record-start sentinel).
  word5+ = per-move animation/timing/keyframe stream (differs: 0605 050A 0C00 vs 0908 0A14 1E00).
So a move record = a small fixed header (with constant markers word3=0x0020, word4=0xC000) + per-move
type/param words + a keyframe stream that drives the playback fields (period 0x650, mode 0x647, the
per-channel 0x671+ accumulators). Exact keyframe-stream grammar (frame deltas, hit-box activation,
cancel windows) is the remaining data-format item; the header layout is confirmed.
