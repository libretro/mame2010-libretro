# Sega Model 1 — System Architecture & TGP Reverse-Engineering Reference
(Derived from reversing mame2010-libretro + the real hardware ROMs/firmware vs the upstream-MAME
real-DSP oracle. This documents how the WHOLE geometry/render pipeline works; the VF braid is one
worked example at the end.)

## 1. Hardware overview
Common PCB for all Model 1 games (Virtua Fighter, Virtua Racing, Star Wars Arcade, Wing War, etc.).
- **Main CPU:** NEC V60 (uPD70615GD-16) @ 16 MHz (32MHz/2). Program ROMs epr-16080.4 @0xfc0000,
  epr-16081.5 @0xfe0000 (VF). Big 0x2000000 "maincpu" region; banked model/data access at 0x1000000.
- **TGP (geometry coprocessor):** Fujitsu MB86233 ("TGP"), on Sega custom 315-5571/5572. A floating-
  point DSP with its own internal program ROM (315-5571.bin, 0x2000 words for VF) + external data RAM
  + a large model/table ROM region. Does ALL 3D math: matrix transforms, projection, vector ops,
  collision, track/physics.
- **Renderer:** custom rasterizer fed by a DISPLAY LIST the V60 builds from TGP results. Outputs the
  framebuffer. (In MAME: model1_v.cpp tgp_render walks the list.)

## 2. V60 <-> TGP interface (memory-mapped, the heart of the system)
From the V60 program memory map (model1.c io map):
- `0xd00000` : TGP copro ADDRESS port (model1_tgp_copro_adr_r/w) - sets model-ROM read pointer.
- `0xd20000` : TGP copro DATA RAM window (copro_ram_r/w) - V60 reads/writes the DSP's data RAM.
- `0xd80000` : TGP copro COMMAND/FIFO port (copro_r = read result FIFO, copro_w = write command FIFO),
  mirrored at +0x10.
The V60 talks to the TGP as a command stream: write a command word + args to the FIFO (0xd80000),
read result words back from the FIFO. Internally a 32-bit value is sent as two 16-bit halves
(upstream v60_copro_fifo_w assembles `(hi<<16)|lo`; reads return lo then hi).

In MAME2010 the TGP is **HLE'd**: the real MB86233 is NOT emulated; instead a C table (ftab_vf)
intercepts each command word and computes the result in C, pushing results to a software fifoout the
(real, emulated) V60 reads back. The V60 program ROM IS run for real. So MAME2010 = real V60 + fake TGP.
Upstream MAME = real V60 + real MB86233 (the "oracle" used here for ground truth).

## 3. The TGP command set (ftab_vf — the TGP "instruction set")
Command dispatch: V60 writes `cmd_word` to FIFO; `function_get_vf` does `f = cmd_word>>23` and calls
`ftab_vf[f].cb`, which then pops `ftab_vf[f].count` argument words. 104 entries. Key commands:

Matrix stack & transforms (the core 3D pipeline):
  0x05 matrix_push        - push current matrix (cmat) onto stack
  0x06 matrix_pop         - pop
  0x07 matrix_write (12)  - load a full 3x4 matrix
  0x10 matrix_ident       - identity
  0x11 matrix_read        - EMIT the current matrix: pushes cmat[0..11] to fifoout (the V60 then writes
                            it as a 0xB display-list command). THIS is how an object's world matrix
                            reaches the renderer.
  0x12 matrix_trans (3)   - translate
  0x13 matrix_scale (3)   - scale
  0x14/15/16 matrix_rotx/y/z (1) - rotate about axis
  0x4f matrix_unrot, 0x52 matrix_rtrans, 0x60 vmat_load1 ... - matrix utilities
  0x1a transform_point(3) - transform a point by cmat
Vector / scalar math:
  0x0a anglev(2), 0x0f anglep(4)   - angle computations
  0x1f distance3(6), 0x2e vlength(3), 0x43 distance(4)
  0x24-0x29 acc_set/get/add/sub/mul/div - an accumulator ALU
Collision / physics (used heavily by VR racing + VF hit detection):
  0x31 colbox_set(12), 0x32 colbox_test(3), 0x40 col_setcirc(3), 0x41 col_testpt(2),
  0x57 groundbox_test(3), 0x65 groundbox_set(7)
Track / driving (Virtua Racing):
  0x0d track_select(1), 0x18 track_read_quad(1), 0x30 track_read_info(1), 0x36 track_lookup(4),
  0x47 car_move(4)
"vmat" virtual-matrix bank (skeletal animation - used for character joints incl. hair):
  0x4a vmat_store(1), 0x4b vmat_restore(1), 0x4d vmat_mul(2), 0x4e vmat_read(1), 0x54 vmat_save(1),
  0x55 vmat_load(1), 0x5f vmat_flatten, 0x60 vmat_load1(1)
RAM-scan / chain (skeleton joint data; the hair bone-chains):
  0x56 ram_setadr(1), 0x61 ram_trans, plus the VF-specific f102 "mve_calc"(8) / f103 "mve_setadr"(1).
  vmat slot 15 = the head joint (parent of the hair chains).

## 4. Skeletal animation & the vmat bank
Characters are skeletons of joints. The TGP keeps a "vmat" bank of per-joint matrices (vmat_load/store/
mul/read, slots indexed; slot 15 = head). Body parts (object IDs 0x467xx for VF Pai's body) are drawn
by loading the relevant joint matrix and emitting via matrix_read. Hair/ponytails are BONE CHAINS hung
off the head joint, computed by the mve_calc chain mechanism (see section 6).

## 5. Display list & renderer
The V60 builds a display list the renderer walks (tgp_render). List command = `(word) & 15`:
  0x0/0x1/0x3/0x5 : draw a plane/decor (background, floor, scenery)
  0x2/0x4/0x6     : draw an OBJECT (model from the model ROM), args = (model_addr, tex, ...)
  0xB             : load the per-object world MATRIX (12 floats) used to transform the next object's
                    vertices. This is what matrix_read (0x11) produces. trans_mat[0..11].
The renderer transforms model vertices by trans_mat, projects, and rasterizes. trans_mat IS the final
world matrix; in MAME2010 it equals the cmat the TGP HLE emitted (verified: f102 EMIT cmat == renderer
trans_mat, no hidden transform between).

## 6. Hair bone-chains (VF) — worked example of the chain mechanism
The VF character hair/ponytail is a bone chain. Per the real V60 routine (epr-16081.5 @ FFC764):
  - FFC764: mve_setadr (0x33800000) + scanadr 0x9500 -> points the TGP at the chain's joint block in
    copro data RAM (a single ~25-word joint spec: scale [9504,9505]=0.85, a rotation row
    [9507..9]=(-0.487,0.835,0.592), lengths [950e]=1.14/[9510]=1.406, base pos [9516..18]).
  - FFC774-FFC785: read link COUNT, loop per link calling FFC790.
  - FFC790 per link: matrix_push; mve_calc (0x33000000) sending 8 args (2 from arg ptr + a 6-word
    block R21); then READ 6 VALUES BACK into R21[0,4,8,C,10,14]. The 6-word R21 is BOTH the copro
    return AND the next link's input -> the chain ACCUMULATES through R21 (forward kinematics).
  - matrix_read emits the resulting 12-word matrix as the link's 0xB object matrix.
  - FFC787: matrix_pop ends the chain.
So mve_calc is a chain joint integrator: R21[n] = transform(args, R21[n-1], joint block). The copro
routine at 0x62a (315-5571.bin) computes it (a matrix-vector loop over the joint block).

MAME2010's HLE f102 does NOT implement this; it passes (c,d,e) through and fabricates a look-at, so the
per-link rotations are wrong (flat base, sign-flipped, no Z-tilt) and the hair detaches during motion.
Bit-exact fix = implement the 0x62a transform + the R21 accumulation in f102. (Full braid analysis:
MODEL1_TGP_braid_RE.md.)

## 7. Per-game notes
- **Virtua Fighter (VF):** ftab_vf; characters via vmat skeleton; hair = mve_calc chains (Pai braid,
  Wolf braid, etc., object IDs 0x471xx for Pai; char2/Wolf base 0x43xxx). Body 0x467xx.
- **Star Wars Arcade (SWA):** ftab_swa (separate command table); shares matrix_read etc.
- **Virtua Racing:** heavy use of track_* and car_move + collision commands; separate TGP program
  (the internal TGP ROM still undumped; MAME uses a Daytona-derived stand-in).

## 8. MAME2010 HLE vs real hardware (for accuracy work)
- Real V60 program: run for real (ROMs present).
- TGP: HLE (ftab C funcs). This is where accuracy diverges from real silicon. Commands that are pure
  matrix ops are exact; commands involving the TGP's internal state machine / chain integration
  (mve_calc) are approximations.
- The upstream-MAME real-MB86233 build is the ground-truth oracle; body parts + simple transforms match
  bit-exactly between HLE and oracle (confirmed by object-matrix diff), divergence is in chain math.

## 9. Chain setup sequence (V60, decoded FFC720-FFC764)
  FFC729-FFC738: pack chain header words from arg ptr R15 into a work buffer (R18).
  FFC73D: bsr FFC977 (compute chain params).
  FFC740: mov.h #9500, D00000      - set copro address port to 0x9500.
  FFC751: mov.w #0x19, R0          - count = 25.
  FFC758: mov.w [R1+], D20000 (loop x25) - upload the 25-word JOINT BLOCK from V60 RAM (R1) into copro
                                    data RAM at 0x9500 (D20000 = 0xd20000 copro_ram window).
  FFC764+: mve_setadr 0x9500, then the per-link loop (section 6).
So: V60 uploads a 25-word joint spec to copro RAM 0x9500, points the TGP at it (mve_setadr), then per
link sends 8 args (2 header + 6-word R21 chain state) and reads back the new 6-word R21. The copro
routine at 0x62a transforms (args, R21, jointblock@0x9500) -> new R21, using MB86233 float ALU
(fadd/fsbd/fml/fmsd multiply-accumulate; the 0x631-0x634 loop is a matrix x vector).

This whole mechanism (upload joint block -> setadr -> per-link integrate) is the TGP's generic
skeletal-chain primitive; VF hair is one user of it.

## 10. TGP chain-integration execution (mve_calc copro routine, decode status)
The copro routine for mve_calc (entered from the FIFO command dispatch) reads inputs then runs a
float-ALU loop. MB86233 ALU ops (from the interpreter decode): 0x05 fcpd, 0x06 fadd, 0x07 fsbd,
0x08 fml (p=a*b), 0x09 fmsd (d=d+p; p=a*b -> multiply-accumulate). The 0x631-0x634 loop is a
matrix x vector MAC using the joint-block rows; 0x636+ post-processes.

Execution-capture status (mb_run interpreter, real microcode + injected 0x9500 block):
  - Feeding mve_calc(0x33000000) + the real 8 args reaches the routine (pc 0x161) but STALLS waiting
    for more FIFO input (out=0). The routine is count/mode-driven: it expects a data stream whose
    length/mode is set by PRIOR commands the real V60 issues (the full per-frame command context),
    not just the isolated per-link command. So isolating one mve_calc call under-specifies the routine.
  - To capture the true 6 outputs, the interpreter must replay the FULL command context the V60 sends
    (the mode-setting commands before the chain), OR the routine's count must be seeded from the chain
    header. This is the remaining execution blocker; it is a matter of replaying enough of the real
    command stream, not a fundamental unknown.
Bit-exact braid = this routine's 6-output transform, carried via the R21 6-word accumulation, ported
into the HLE f102.
