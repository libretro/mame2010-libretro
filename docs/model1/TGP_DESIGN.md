# Sega Model 1 — Reverse-Engineering & Decompilation Design Document

Status: living document. Captures everything established through tracing, disassembly,
and cross-core differential analysis of the Sega Model 1 geometry pipeline, with
emphasis on the Fujitsu MB86233 ("TGP") coprocessor and the Virtua Fighter (`vf`)
geometry firmware `315-5724.bin`.

Scope note: this is an engineering/RE reference, written from observed behaviour of
the hardware emulation and the firmware disassembly. Where a fact is inferred rather
than directly observed, it is marked (inferred). Companion runtime log:
`braid_ground_truth.md` (chronological investigation notes).

---

## 1. System overview

The Model 1 renders 3D geometry through a three-tier pipeline:

1. **V60 main CPU** — runs the game program. Builds geometry command streams, sends
   them to the TGP, reads transformed results back, and assembles the display list
   the video hardware rasterises.
2. **MB86233 "TGP" coprocessor** — a Fujitsu floating-point DSP that runs a
   game-specific microcode ROM (the "geometrizer"/copro program). It transforms
   vertices, builds matrices, projects geometry, and streams results back to the V60.
3. **Tilemap/sprite + 3D rasteriser video hardware** — consumes the display list the
   V60 builds (two ping-ponged lists) and draws polygons + the System 24 tile layers.

The V60 <-> TGP link is two unidirectional 32-bit FIFOs:
- **FIFO-in** (V60 -> TGP): the command/parameter stream.
- **FIFO-out** (TGP -> V60): the transformed-geometry result stream.

The key architectural fact for emulation: the two processors run as a tightly-coupled
**feedback loop**. The V60 sends a command, the TGP processes it and pushes results,
the V60 reads those results and decides what to send next. Correct emulation requires
the FIFO handshake (including empty-FIFO wait states on both sides) to be faithful;
small timing/handshake errors can desync or deadlock the loop and corrupt the V60's
program flow.

---

## 2. The MB86233 ("TGP") coprocessor

### 2.1 Identity & ROMs

- Fujitsu MB86233, QFP160. On Model 1 it appears as the "Geometrizer" pair
  (315-5571 / 315-5572) plus a game-specific COPRO program.
- For Virtua Fighter the copro program is **`315-5724.bin`**: 8 KB = 2048 x 32-bit
  program words. CRC `4b4f330e`, SHA1 `8809d93d47593f808faca55161999677ac7a3eb0`.
  Flagged `BAD_DUMP` upstream but functionally coherent (boots, runs).
- Clock: 40 MHz (`40_MHz_XTAL`) per the modern reference; an earlier 16 MHz value was
  tried and is too slow relative to the V60.
- The math tables ("user5" region, ~0xE0000 bytes: `opr14742`-`opr14748`) back the
  hardware math units (sin/cos, reciprocal, etc.).

### 2.2 Register / memory model

Data spaces and registers established from the disassembler reference and observed
execution:

- **Working registers:** `a`, `b`, `d`, `p` (32-bit float accumulators), with
  `p` the multiplier-product register used by the MAC ops. Index registers `x0`,`x1`
  and base registers `b0`,`b1`; loop counters `c0`,`c1`; index increments `i0`,`i1`;
  shift register `sft`; stack pointer `sp`; vsm/vsmr (modulo-addressing mask).
- **`bh` / `ah` / `dh` / `ph`** (register indices 0x11/0x14/0x1a/0x1d): read as the
  IEEE **exponent** of the corresponding float via `get_exp(val) = (val>>23)&0xff`.
  This is how the firmware extracts a command index from a float command word
  (see 3.2). `get_mant` reads the mantissa side.
- **Data address space (per-DSP):**
  - `0x000-0x0FF`: ARAM (internal scratch A).
  - `0x100`: **FIFO-in port** (reading pops a word from the V60).
  - `0x200-0x3FF`: BRAM (internal scratch B). Note the `+0x200` offset forms in
    the `lab` dual-load address these.
  - `0x400`: **FIFO-out port** (writing pushes a word to the V60).
  - `0x400000+`: external "copro RAM" (a larger work RAM, gated by address bit
    0x8000 for auto-increment — see the shipped fix in 7). VF does not read this
    during normal match geometry.
- **Program space:** 0x000-0x7FF (2048 words), the copro ROM ("tgp" region).

### 2.3 Addressing modes (verified bit-identical to the modern reference)

`adx` uses `x0`/`b0`, `ady` uses `x1`/`b1`. Decoded by `ea_pre_0`/`ea_pre_1`:
- `0aaaaaaa` -> direct `$a`
- `01aaaaaa` / `10aaaaaa` -> `$a` + base + index (`(r&0x7f) + bN + xN`)
- `1 100bbbbb` -> `(bxn + $b)` ; `1 101bbbbb` -> `(xn + $b)`
- `1 110bbbbb` -> `[bxn + $b]` ; `1 111bbbbb` -> `[xn + $b]`
- the `0x180` group selects vsm/modulo variants: `bN+xN`, `xN`,
  `bN+(xN & vsmr)`, `xN & vsmr`.

Post-increment (`ea_post_0/1`): when `(r & 0x100)`, `xN += iN` (if `!(r&0x080)`) or
`xN += sext(r,5)`.

### 2.4 Instruction classes (top 6 bits)

- `0x00` **lab** — dual parallel load. Sub-ops (bits 18-20):
  - `0/1`: `a = data[ea0]`, `b = io[ea1]` (second operand from I/O space)
  - `3`: `a = data[ea0]`, `b = data[ea1 + 0x200]`
  - `4`: `a = data[ea0 + 0x200]`, `b = data[ea1]`
  An ALU op may be packed in the same instruction (the `ALU : lab ...` forms).
- `0x07` **ld / mov** — single load/store/move with sub-op forms (reg<->mem, mem<->mem,
  mem+0x200, etc.). Packs an ALU op as well (the `ALU : mov ...` forms).
- `0x0d` **stm / clm** — status/control set/clear (e.g. `set #0x800` enables the
  FIFO-in interrupt; `clr1` acknowledges it).
- branch/dispatch class — `brif` (conditional immediate branch), `brul` (branch via
  register `d`, used for the dispatch jump), `bsif` (branch-subroutine), `rtif`
  (return), `iret` (interrupt return). Condition codes include `alw` (always),
  `zrd` (`d==0`), `ged` (`d>=0`, `!SGD`), `led` (`d<=0`, `ZRD|SGD`), `!led`, gpio0/1.

### 2.5 ALU operations (all verified bit-identical to the modern reference)

Two-stage execution: `alu_pre` computes `alu_r1`/`alu_r2`; the operand load happens;
then `alu_post_1` (integer commit) and `alu_post_2` (float commit) write back. This
split — and the ordering relative to the operand load — matches the reference exactly.

Float / MAC (the geometry workhorses):
- `0x05 fcpd` — compare `d - a` (flags only)
- `0x06 fadd` — `d = d + a`
- `0x07 fsbd` — `d = d - a`
- `0x08 fml`  — `p = a * b`  (commits to `p`)
- `0x09 fmsd` — `d = d + p ; p = a * b`  (multiply-accumulate)
- `0x0a fmrd` — `d = d - p ; p = a * b`  (multiply-subtract)
- `0x0b fabd` — `d = |d|`
- `0x0c fsmd` — `d = d + p`  (accumulate, no fresh multiply)
- `0x0d fspd` — `d = p ; p = a * b`  (start sum-of-products)
- `0x0e cxfd` — convert int->float (round/ceil/floor sub-modes)
- `0x10 fdvd` — `d = d / a` (the reference does a raw divide; this port guards
  divide-by-zero by returning `d` — behaviourally equivalent where the divisor is
  nonzero, which it always is in observed VF geometry)
- `0x11 fned` — `d = -d`
- `0x13` — `d = b + a` ; `0x14` — `d = b - a`

Integer / shift:
- `0x16 lsrd` (`d >> sft`), `0x17 lsld` (`d << sft`), `0x19 asld`
  (arithmetic `s32(d) << sft`), `0x1a addd` (`d + a`), bitwise `& | ^ ~` (0x01-0x04).

Status flags from float results: `stset_set_sz_fp(v)` sets `F_SGD` if negative,
`F_ZRD` if zero (zero test ignores the sign bit). Branch conditions read these.

### 2.6 Math units (sin/cos/recip/etc.)

External-port reads with `EB==0` and offset `0x20-0x2F` sample the hardware math
units backed by the user5 ROM tables. Offsets `0x20-0x23` are SIN/COS: the firmware
writes an angle to `extport[0x20]` in `0x4000`/PI steps and reads sin/cos at the
adjacent offsets (`value += (offset-0x20)<<14; off = value & 0x3fff`). Observed
working with sensible angle inputs (`0x4000` = quarter turn, `0x0`, `0xffffc000`).

---

## 3. The VF geometry firmware (`315-5724.bin`)

### 3.1 Boot

On reset the firmware spins reading the FIFO-in command port. The boot spin is at
PC `0x1e`: it loads `x1 = data[4]` (a pointer that equals `0x100`, the FIFO-in port),
then `b = data[x1] = data[0x100]` — i.e. it polls the FIFO for the first command.
Until the V60 pushes, the FIFO is empty and the DSP waits. (Emulation note: if the
core does not map `0x100` to the FIFO and instead returns 0, the DSP never advances
and the game falls to the service/test menu — this was a real bug, fixed in 7.)

### 3.2 Command dispatch (PC 0x27-0x2f)

The dispatch loop:
```
0027: ldi #0x200, b1
0028: mov $4, x0
0029: mov $4, x1
002a: clr0 d, ?5
002b: mov (x1), b        ; b = data[4]  -> reads the FIFO-in command word
002c: mov bh, d          ; d = get_exp(b)  -> the command index
002d: lia #0x30          ; a = 0x30
002e: addd               ; d = d + 0x30   -> dispatch-table index
002f: brul alw d         ; jump to table[d]
0030: brif alw #0xa6     ; table[0x30] = handler for command 0x00
0031: brif alw #0xab     ; table[0x31] = command 0x01
...
```

So: **the command index is the IEEE exponent of the command word** (`bh = get_exp(b)`),
and the handler is reached via a jump table based at PC `0x30`. Command `N` -> jump at
`0x30 + N` -> `brif` to that command's handler.

This is the single most important structural fact: command words are *floats* whose
**exponent field encodes the opcode**. Example: a command word `0x24000000` has
exponent `(0x24000000>>23)&0xff = 0x48`, so it dispatches command `0x48`.

### 3.3 Command map (partial, from the dispatch table + observed use)

Observed commands in a correct frame's geometry program (the modern reference):
`5, 0x10, 0x12, 0x14, 0x15, 0x16, 0x11 (repeated), 0x6`, plus matrix-family
`0x30/0x31/0x32` and `0x48-0x4c`.

Identified / inferred roles:
- `0x05` — matrix/transform setup (frame/camera). Appears once per object group.
- `0x10`,`0x12`,`0x14`,`0x15`,`0x16` — vertex/transform operations (the per-object
  transform chain). (inferred from position in the stream)
- `0x11` — vertex/draw emit; appears in long runs (one per polygon/vertex batch).
- `0x06` — end / flush of an object or list.
- `0x30`/`0x31`/`0x32` — matrix operations (table base `0x30` handlers at
  `0xa6/0xab/0xb0`). (inferred)
- `0x48-0x4c` — **matrix-table family**, dispatch table `0x78-0x7c`:
  - `0x48` -> handler `0x264` — matrix *build* (cross/dot-product orthonormal basis,
    see 3.4), writes into the matrix table.
  - `0x49` -> `0x29d`, `0x4a` -> `0x29e`, `0x4b` -> `0x2ab`, `0x4c` -> `0x2b9` —
    matrix-table *readout/copy* operations (12-word `rep #0xc` copies between BRAM
    and ARAM, indexed per object).

### 3.4 The matrix-build handler (cmd 0x48, PC 0x264)

Handler `0x264` builds a rotation matrix from direction vectors:
- A subroutine at `0x24e` computes a **dot product** (an `fml/fspd/fmsd/fsmd` MAC
  chain over 3 elements) and a **cross product** (`0x254-0x261`, `fml/fspd/fmrd`
  pattern producing the 3 cross components into ARAM `$9/$a/$b`).
- Normalization via `fdvd` (divide by length), `fned`, `fabd`, with `led`/`!led`
  conditional branches selecting normalization paths.
- The resulting 12-word matrix is stored into a **matrix table at data `0xb0+`**,
  indexed per object: `idx = 0xb0 + (object_index << 4)` (computed via
  `ldi #4,sft; asld; lia #0xb0; addd`), then copied with `rep #0xc` (12-word) loops
  between BRAM (`+0x200`) and ARAM.
- A later command in the `0x48` family reads the table back out to the FIFO-out.

This is the routine that produces character body-bone matrices in the correct program.

### 3.5 The display-list / result format (V60 side)

The V60 reads the FIFO-out result stream and assembles the display list at video
addresses `0x600000` (list 0) / `0x610000` (list 1), ping-ponged via the `listctl`
register at `0x680000`. The rasteriser walks the list; entry types (low bits of the
first word) include:
- type `0` — terminator/no-op
- type `1` — **draw object**: `push_object(tex_addr, ?, ?)` using the *current* matrix
- type `0xa`, `0xb` — **matrix load**: 12 floats (`readf(list+2+2*i)`, then `list+=26`)
  set the active transform `trans_mat[12]` (3x3 rotation in [0..8], translation in
  [9..11]). All subsequent draw objects use this matrix until the next matrix entry.
- type `0xc`, `0xf` — other control entries.

A correct character body part is a type-0xb matrix entry with non-zero rotation and a
per-bone translation, followed by draw objects. A degenerate (broken) body part is a
zero-rotation matrix with only the camera/base translation — every vertex collapses
to a point.

---

## 4. The V60 <-> TGP FIFO link (emulation-critical)

### 4.1 Ports

- **FIFO-in (V60 -> TGP):** the V60 writes a 32-bit word as two 16-bit halves
  (low half first, high half second; the push happens on the high-half write). The
  TGP reads it at data port `0x100`.
- **FIFO-out (TGP -> V60):** the TGP writes at data port `0x400`. The V60 reads it at
  I/O `0xd80000` as two 16-bit halves (offset 0 pops the 32-bit word and returns the
  low half into a latch; offset 1 returns the high half from the latch).

### 4.2 Wait-state semantics (the hard part)

The modern reference uses true blocking FIFO devices (depth 16): reading an empty
FIFO stalls the reading CPU until the other side pushes; the two halves of a 32-bit
read are served atomically. This port approximates it with:
- **TGP side:** reading empty `0x100` sets a `stall` flag; the execute loop
  re-executes the same instruction (PC rewound to `ppc`) on the next slice.
- **V60 side:** reading empty `0xd80000` calls `v60_stall` (sets `stall_io`) +
  `timer_call_after_resynch`; the V60's `IN` instruction re-executes (its
  `F12WriteSecondOperand`/`F12END` are skipped, so PC does not advance).

This approximation is the chief source of subtle bugs: the two-halves read is not
atomic w.r.t. scheduling, and the stall/resync ordering can let one CPU run ahead or
(under heavy load) deadlock. See 6.

### 4.3 FIFO-in interrupt

The firmware can enable a FIFO-in interrupt (`set #0x800` -> vector 4 / PC 4) and
acknowledge it (`clr1`). This port pushes the interrupt on FIFO-in write
(`mb86233_set_fifo_in_irq`). The ISR updates status registers. This is a side path,
not the primary command-flow mechanism (the firmware mostly polls the FIFO directly).

---

## 5. Differential methodology (how facts here were established)

Two MB86233 implementations were run side by side on identical ROMs:
- **This port** (mame2010-based libretro core), and
- **a modern reference core** (libretro/mame build) that renders VF correctly — used
  as a ground-truth oracle.

Techniques:
- **Static op-by-op comparison** of every addressing mode, ALU/MAC/shift op, flag
  rule, branch condition, dual-load, and the alu_pre/post split — all verified
  bit-identical. (Conclusion: no single-instruction implementation bug.)
- **Disassembly** of `315-5724.bin` against the documented opcode encoding to
  reconstruct the dispatch loop, command map, and the matrix-build handler.
- **Runtime instrumentation** of both cores: FIFO push/pop streams, dispatched
  command sequences, per-frame command histograms, per-instruction register/RAM
  state, and display-list matrices.
- **Frame-by-frame bisection** to localise a divergence in wall-clock time.

The differential method was decisive: because every instruction is individually
correct, only comparing the *running programs* (command streams, frame histograms)
and bisecting revealed the true bug class.

---

## 6. Known emulation pitfalls (Model 1 specifically)

1. **FIFO port mapping.** Data `0x100` must pop FIFO-in and `0x400` must push
   FIFO-out. Returning 0 for `0x100` causes the boot poll to hang -> service menu.
2. **FIFO wait-state faithfulness.** The blocking semantics matter. Approximations
   (return-0 + resync, perfect-quantum, etc.) can:
   - inject zeros into the result stream when the V60 outruns the TGP (every-Nth-word
     zeros -> zero-rotation matrices), or
   - misalign the two 16-bit halves of a 32-bit read when interleaving is too fine
     (garbage matrices), or
   - **deadlock under heavy load**: the TGP waits on empty FIFO-in while the V60 waits
     on empty FIFO-out, neither progressing. Observed at the onset of character
     rendering (see 8). This corrupts the V60 program for the rest of the scene.
3. **Clock ratio.** The TGP runs at 40 MHz; too slow a TGP lets the V60 drain the
   result FIFO faster than the TGP fills it.
4. **Command words are floats.** The command index is the *exponent* of a float word
   (`get_exp`). Do not treat command words as integers.
5. **Copro-RAM auto-increment** must be gated on address bit `0x8000` (see 7).
6. **Different cores use different camera frames** and **desync in the geometry
   stream** — same-wall-frame comparison between two emulators is only valid before
   they diverge; use input-aligned or pre-divergence comparison.

---

## 7. Confirmed fixes / findings (shipped or validated)

- **Copro RAM auto-increment gate** (shipped): gate the TGP copro-RAM auto-increment
  on `(ram_adr & 0x8000)` in the copro-RAM read/write handlers. Patch upstreamed.
- **FIFO ports** (validated): mapping data `0x100`->FIFO-in pop and `0x400`->FIFO-out
  push takes VF from the test menu to a live in-match render on the real DSP.
- **TGP interrupt support** (implemented): FIFO-in interrupt (vector 4), `iret`,
  `set/clr #0x800` enable/ack, and the FIFO-in irq hook.
- **Command decode confirmed correct**: `bh = get_exp(b)` matches the reference;
  `0x24000000 -> 0x48` is a correct decode.
- **All DSP arithmetic/addressing verified** bit-identical to the reference (see 2.3-2.6).

---

## 8. Open problem: in-match character rendering (current frontier)

### 8.1 Symptom (precise)

On the real-DSP path VF boots to a live match; the stage, HUD, sky render correctly.
Character geometry itself is NOT broken in general: on the CHARACTER-SELECT screen the
full 3D character head models (e.g. Pai Chan's head, hat, hair) render PERFECTLY -
proof that the DSP's transform/emit pipeline works for complex character geometry.
The corruption is specific to IN-MATCH rendering and sets in at the
character-select -> match TRANSITION: the first in-match geometry frame renders
characters as a degenerate fragment, and stays broken for the rest of the match.

### 8.2 Root cause (bisected and confirmed)

The V60 and TGP run as a feedback loop over the two FIFOs. Using a command-stream
relative clock (Nth dispatched command) and a tag-free push-PC sequence, the two cores
(this port vs the modern reference) were shown to execute IDENTICALLY for the first
~1500 fifo-out pushes / first 299 dispatched commands. The TRUE first divergence is at
fifo-out push #1501: this port pushes at PC 0x12c (the cmd-0x11 handler) while the
reference pushes at PC 0xb3 (the cmd-5 handler). The PC-path trace around that point
shows the proximate trigger: at the dispatch-loop FIFO-in read (PC 0x2b), the reference
gets data within ~2 cycles and proceeds; this port spins ~22 cycles on an empty FIFO-in
and a FIFO-in interrupt fires, perturbing the dispatch loop. From there the command
processing desyncs and the in-match characters degenerate.

This divergence event IS the select->match transition (where the command mix/volume
changes and the FIFO timing asymmetry first bites). It is NOT a DSP-math bug (every
MB86233 primitive is verified bit-identical), NOT the matrix-build handler (0x264 is a
downstream symptom), and NOT a simple deadlock.

### 8.3 Why it happens: the FIFO handshake is not faithfully atomic + blocking

This port approximates the FIFO with: TGP-side a re-execute stall flag on empty-read;
V60-side v60_stall + timer_call_after_resynch on empty-read; depth-256 with
fatalerror-on-overflow (NO back-pressure). The modern reference uses a true blocking
generic_fifo (depth 16) wired to INPUT_LINE_HALT assert/clear callbacks: a reader that
finds the FIFO empty is HALTED and resumed the instant the other side pushes; a writer
that finds it full is halted (back-pressure) and resumed on the other side's pop; each
32-bit transfer is atomic. This is correct irrespective of scheduler interleave.

The approximation fails in two complementary ways, both observed:
- Too-loose scheduling: the TGP outruns the V60, spins ~22 cycles on empty FIFO-in
  (vs ~2), the dispatch loop is perturbed (and a spurious FIFO-in interrupt fires) ->
  divergence at push #1501 -> degenerate in-match characters (a tiny fragment).
- Too-tight scheduling (QUANTUM_PERFECT_CPU): the two half-word (offset 0/1) FIFO
  accesses interleave with TGP pushes/pops -> corrupted 32-bit words -> garbage
  geometry (huge corrupt polygons).
No quantum setting recovers correct geometry; both extremes are symptoms of the same
non-atomic, non-blocking handshake.

### 8.4 Attempted fixes (results)

- Disable the FIFO-in interrupt: VF still boots, characters still broken (the interrupt
  firing is a side effect of the long wait, not the sole cause).
- Quantum tuning (QUANTUM_TIME HZ 6000..60000, QUANTUM_PERFECT_CPU): no value renders
  correctly; light = unchanged, heavy = lost geometry, perfect = garbage.
- Trigger-based blocking (spinuntil_trigger/cpuexec_trigger), the mame2010-era analogue
  of the HALT callbacks, implemented fully (depth-16, bidirectional reader blocking,
  writer back-pressure, correct trigger-after-data ordering): STABLE (no crash, no
  deadlock; TGP very active, 56k+ pushes) but renders empty/abundant-but-wrong, and by
  the objective push-PC metric it diverges EARLIER (push #1316 vs #1501). Conclusion:
  the trigger mechanism does not reproduce the reference's HALT-line timing granularity
  and is a dead end for faithful timing.

### 8.5 The faithful fix (remaining work, scoped)

Replicate the reference's HALT-line FIFO, not the trigger approximation:
1. Depth-16 FIFOs (both directions).
2. Reader blocks on empty by ASSERTING INPUT_LINE_HALT on the reading CPU; the push
   side CLEARS it. The blocked read must not advance the instruction; it resumes and
   re-reads cleanly when un-halted.
3. Writer blocks on full by ASSERTING INPUT_LINE_HALT on the writing CPU; the pop side
   CLEARS it (back-pressure).
4. The V60 fifo-out half-word read (offset 0 pop+low into a latch, offset 1 high from
   the latch) must be atomic across the block/resume.
This is a genuine integration into the V60 and MB86233 execution cores (the HALT line
must pause exactly at the point of needing data and resume re-reading), not a
peripheral callback swap.

Validation criterion (from the bisection): with the fix, the first 1500 fifo-out
push-PCs must still match the reference AND push #1501 must follow the reference to
PC 0xb3 (not 0x12c). Then the select->match transition should no longer corrupt, and
in-match characters should render.

Note: the modern reference MB86233 core already renders VF fully correctly and remains
the definitive functional reference and fallback.

## 9. Glossary

- **TGP** — "Geometry Processor", the MB86233 copro.
- **FIFO-in / FIFO-out** — the V60->TGP / TGP->V60 32-bit FIFOs.
- **copro RAM** — the TGP's external work RAM at `0x400000+`.
- **display list** — the V60-built command list (at `0x600000`/`0x610000`) the
  rasteriser walks; matrix entries (type 0xa/0xb) set the active transform, draw-object
  entries (type 1) emit polygons under it.
- **command word** — a 32-bit float pushed to FIFO-in; its exponent is the opcode.
- **matrix table** — TGP-internal per-object matrix store at data `0xb0+`, indexed
  `0xb0 + (obj<<4)`.

---

*Companion: `braid_ground_truth.md` (chronological notes),
`vf_tgp_5724_upstream_disasm.asm` (authoritative disassembly).*
