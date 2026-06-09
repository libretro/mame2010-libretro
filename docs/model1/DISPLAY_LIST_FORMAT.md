# Sega Model 1 TGP Display-List & Geometry Format

Reverse-engineering notes toward (a) accurate MAME 2010 HLE emulation and
(b) eventual VF/VR decompilation. Derived from the HLE renderer
(`src/mame/video/model1.c`) cross-checked against upstream `model1_v.cpp`,
plus live instrumentation of Virtua Fighter (`vf`) frame captures.

Status legend: [V] verified by instrumentation, [S] from source semantics,
[?] unknown / needs decomp-level confirmation.

---

## 1. Memory regions

- `poly_rom`  : geometry ROM, `user1` region. Polygon data for built-in models. [S]
- `poly_ram`  : 0x400000 words, uploaded geometry (list opcode 5). [S]
- `tgp_ram`   : 0x40000.. texture/color indirection table (list opcode 4). [S]
- Two display lists (`model1_display_list0/1`), double-buffered via `listctl`. [S]
- `poly_adr` bit 0x800000 selects RAM vs ROM for a given object. [V]

## 2. Display list = stream of 16-bit words, opcode = word0 & 15

| op  | size (words) | meaning                                        | status |
|-----|--------------|------------------------------------------------|--------|
| 0   | 2            | NOP / padding                                  | [V]    |
| 1   | 8            | draw object: w2=tex_adr, w4=poly_adr, w6=seq   | [V]    |
| 2   | var          | draw direct (inline screen-space polys); NOT used by VF/VR | [V/S] |
| 3   | 16           | viewport: flushes quads, sets clip/center      | [V]    |
| 4   | 6+len*2      | color/tex-table write to tgp_ram               | [S]    |
| 5   | 6+len*2      | poly_ram upload (uploaded geometry)            | [S]    |
| 6   | 6+len*2      | light-param upload (d,a,s,p per entry)         | [S]    |
| 7   | 4            | select plane/layer: w2 = plane id (see 2.2)    | [V]    |
| 8   | 4            | select mode: w2 = float 4.0 in VF & VR (const) | [V]    |
| 9   | 6            | zoom (zoomx,zoomy as float*4)                  | [V]    |
| 0xa | 8            | light vector (x,y,z float)                     | [S]    |
| 0xb | 26           | 3x4 transform matrix (12 floats)               | [V]    |
| 0xc | 6            | translate (transx,transy float)                | [S]    |
| 0xf / -1 | -       | end of list                                    | [V]    |

Source comments tie object "planes" to opcode-1 usage:
`1=plane1, 3=plane2, 5=decor` (render layers). [S]

### 2.2 Opcode 7 — plane/layer select [V]
VR emits opcode 7 with w2 payloads `1` and `3`; VF does not emit it. These
match the plane ids in the opcode-1 source comment (1=plane1, 3=plane2,
5=decor). Opcode 7 therefore selects the render plane for subsequent
objects; the HLE only counts it (`zz++`) and is otherwise plane-agnostic,
which is why VF (single effective plane) works without honoring it. A
decomp must treat planes as ordered layers (later planes composite over
earlier — cf. Model 2-B "later windows draw after earlier"). [V]/[?]

### 2.3 Opcode 8 — mode select [V]
w2 = 0x40800000 (= float 4.0) in both VF and VR, emitted once/few times per
frame. Constant value suggests a fixed render-mode or scale selector
(note zoom is also stored as float*4). Exact effect unconfirmed. [V value, [?] effect]

### 2.1 Opcode 1 third parameter (w6) — "seq", NOT a vertex count [V]
Originally treated as `size` (vertex count) by the HLE, with `if(!size)
size=0xffffffff`. Instrumentation disproves the count interpretation:

- w6 is **monotonically increasing** across objects in a frame:
  0x6, 0xcb, 0x196, 0x1b8, 0x204, 0x254, 0x2ca, ... 0x1027 (89 objects),
  then 2 resets near end (second render pass).
- Actual vertices processed per object vary independently (197, 203, 34,
  76, 80, 118, 24, ...) and do NOT equal w6.
- The strip is terminated by a per-record end marker (record `type==0`),
  never by w6.

=> w6 is a **per-object draw-sequence / priority value**. The HLE ignores
it for sorting (uses it only as an unused loop bound). For emulation this is
harmless today; for a decomp this is a real field of the object descriptor
and likely the per-object Z-priority described in Namco's sorting patent
(US5522018) and the Sega Model 2-B CRX sort documentation. [V]/[?]

## 3. Object geometry block (at poly_adr)

```
header:  6 floats = 2 seed vertices (old_p0, old_p1)
           [+0..+2] old_p0.xyz
           [+3..+5] old_p1.xyz
records: repeating 10-word polygon records until a record with type==0
```

### 3.1 Polygon record (10 words) [V]
```
word 0  : flags (uint32, bitfield — see 3.2)
word 1-3: vn.xyz   face normal (model space)
word 4-6: p0.xyz   vertex
word 7-9: p1.xyz   vertex
```
Each record adds (up to) 2 new vertices to a triangle/quad strip; the quad
emitted is (old_p1, old_p0, p0, p1). Strip continuation is governed by
`link` (see 3.2). After each record, old_p0/old_p1 advance per link mode.

### 3.2 flags bitfield (word 0) — mapped from VF object 3 [V where noted]
```
bits 0-1   type      : 0 = end-of-strip marker; nonzero = draw. type==2 in
                       draw_direct means single-point (p1=p0).            [V]
bit  8-9   link      : strip connectivity / advance mode:                 [V]
                         0 = strip restart (no draw; reseed old_p0/old_p1)
                         1 = old_p1 = p0   (fan-ish)
                         2 = advance both  (old_p0=p0, old_p1=p1)
                         3 = old_p0 = p1
bit  10-11 zmode     : sort-Z source for the quad:                        [V]
                         0 = carry previous quad's z (old_z)
                         1 = min of 4 corner camera-z (min4f)
                         2 = max of 4 corner camera-z (max4f)
                         3 = 0.0
bit  12    tex_inc   : if set, tex_adr++ before this record               [S]
bit  13    moire     : stipple/transparency (draw_hline_moired)           [S]
bit  14    no_cull   : if set, skip back-face determinant cull            [V]
bits 17-20 lightmode : index into lightparams[] (ambient/diff/spec/power) [V]
bit  23    [?]       : varies within object; not type/light/link. unknown [?]
bit  24    [?]       : varies within object; unknown                      [?]
other bits           : not observed set in VF obj3                        [?]
```
Note bits 23/24 do NOT correlate with cap-vs-dome priority (tested); their
role is unidentified. A decomp must resolve them. [V that they vary, [?] role]

## 4. Rendering pipeline (HLE; hardware-faithful at polygon granularity)

1. Walk list; opcode 1 -> push_object builds quads into quaddb.
2. Per quad: transform verts by trans_mat (3x4) + view; project; back-face
   cull via `view_determinant > 0` unless flags bit14; compute sort-z by
   zmode; light via lightmode; look up color via tgp_ram[tex_adr].
3. At viewport (op 3) and list end: `draw_objects` = sort_quads (qsort by z
   descending, buffer-index tiebreak) + draw_quads (painter's, flat fill).
4. fill_quad rasterizes screen x/y spans ONLY — no per-pixel depth. This is
   faithful to Model 1's per-polygon depth sort (NOT a z-buffer). [V]

Upstream `model1_v.cpp` `quad_t::compare` and the zmode switch are
byte-identical to this HLE. [V]

## 5. Color & lighting pipeline (per quad, VF/VR-exercised) [V/S]

Surface color resolution chain (push_object):
```
1. tex_adr indexes tgp_ram. palette index = 0x1000 | (tgp_ram[tex_adr-0x40000] & 0x3ff)
2. base = paletteram16[index]  -> 15-bit color: r=bits0-4, g=5-9, b=10-14
3. lighting scalar:
     dif = dot(face_normal, light_vector)
     ln  = lightparams[lightmode].a            (ambient)
         + lightparams[lightmode].d * max(0,dif)  (diffuse)
         + specular                              (SEE NOTE — currently 0)
     lumval = clamp(255*ln, 0, 255) >> 2        (-> 0..63)
4. per-channel luma/gamma via model1_color_xlat (hardware table):
     r' = (model1_color_xlat[(r<<8)|lumval|0x0000] >> 3) & 0x1f
     g' = (model1_color_xlat[(g<<8)|lumval|0x2000] >> 3) & 0x1f
     b' = (model1_color_xlat[(b<<8)|lumval|0x4000] >> 3) & 0x1f
5. final RGB555 = (r'<<10)|(g'<<5)|b'
```
- `model1_color_xlat`: 3 planes (R/G/B at +0/+0x2000/+0x4000), indexed by
  (channel_value<<8 | luma). This is the hardware luma/gamma translation
  table the source comment was unsure existed ("there must be a luma
  translation table somewhere"). The `>>2` and `>>3` shifts are empirical;
  hardware-exact scaling is unconfirmed. [V/?]

### 5.1 Light params (opcode 6 upload) [S]
`struct lightparam { float a (ambient); float d (diffuse); float s
(specular); int p (power); }`. a/d/s packed as 8-bit fields, p in high byte.

### 5.2 Specular is DISABLED in the HLE [V]
`compute_specular()` returns 0 unconditionally; the real implementation is
`#if 0`'d with the note that the Model 2 geo-program formula
`s = 2*(dif*normal.z - light.z)`, raised to a power from `lightparams.p`,
scaled by `lightparams.s`, "doesn't work fine." So `lightparams.s` and `.p`
are uploaded by the game but unused in rendering. For a decomp these are
real material fields; for emulation they are currently dropped (cosmetic). [V]

## 6. Known-good ground truths for emulation (regression anchors)

- VF and VR render correctly under the current pipeline.
- The "detached" knob on Pai's hat is genuine model geometry (a narrow
  raised finial above a wide brim, connecting stalk records 150-172 present
  and drawn). NOT a sort/cull/depth bug; renders identically upstream. A
  per-pixel z-buffer does not change it. Do not "fix" in-core. [V]

## 7. Gaps a VF/VR decompilation must still close

1. Opcode 7 confirmed as plane/layer select (values 1,3 in VR); confirm the
   full plane set and compositing order vs hardware. Opcode 8 carries a
   constant float 4.0 — identify the mode it selects. [partial V]
2. flags bits 23/24 (and any unused high bits in other games). [?]
3. The exact meaning/use of the per-object seq value (w6 of opcode 1) on
   real hardware — is it consumed by the sort hardware, a priority, or a
   DSP-side bookkeeping value? Cross-check against the TGP microcode. [?]
4. draw_direct (opcode 2): confirmed NOT emitted by VF or VR (0 occurrences
   instrumented). Used by other Model 1 titles (e.g. billboards/sprites).
   Header = 18 words (tex_adr, v1, v2, two seed verts as floats); records are
   12 words (type==2, single point, p1=p0) or 20 words (full, explicit z at
   list+12); uses project_point_direct (NO perspective divide — coords are
   already screen-space). Irrelevant to a VF/VR decomp; documented for
   completeness. [V/S]
5. Texture/color indirection: how tgp_ram entries map to palette + the
   color_xlat luma table (model1_color_xlat) — documented in fill path but
   not as a standalone format. [partial S]
6. Light-param record packing (op 6): d/a/s in 8-bit fields, p in high byte
   — verify scaling against hardware. [S]

## 8. Emulation-vs-decomp scope note

For MAME 2010 accurate emulation, items in section 6 are mostly cosmetic or
already approximated well enough; the pipeline reproduces VF/VR faithfully.
For a full VF/VR decompilation, every [?] and [partial S] above is a hard
requirement, because the game/DSP code writes these fields deliberately and
a decomp must reproduce the exact data the original tools emitted.

## 9. VF TGP microcode (315-5724.bin) — reversed & cross-checked [V]

The VF coprocessor program ROM (315-5724.bin, 8KB / 2048 MB86233 words, CRC
4b4f330e, MAME-flagged BAD_DUMP) was found to be COHERENT and usable: clean
6-entry jump table at 0x0, math constants (pi=40490fd0, 2.0, 0.5), only 1
zero word, disassembles cleanly (3 minor UNKOP gaps = disassembler coverage,
not corruption). Full disassembly: vf_tgp_5724_disasm.asm.

Dispatch: V60 sends function id; ROM indexes a jump table at 0x30+id. This
table aligns 1:1 with the HLE ftab_vf[]. Verified handler addresses:
  0x0a anglev->0xf4, 0x0f anglep->0x112, 0x12 matrix_trans->0x12e,
  0x13 matrix_scale->0x13d, 0x14/15/16 rotx/y/z->0x147/0x14a/0x14d,
  0x1a transform_point->0x150 (core 0x1c3).

Cross-checked HLE vs real DSP (decompiled from microcode):
- transform_point: out[i]=cmat[i]*x+cmat[i+3]*y+cmat[i+6]*z+cmat[i+9].
  HLE IDENTICAL. [V]
- matrix_rotx/y/z: rotation new_first=first*cos-second*sin,
  new_second=first*sin+second*cos; column pairs rotx{3,6} roty{6,0}
  rotz{0,3}. HLE IDENTICAL in math, sign, and column selection. [V]
- matrix_trans: cmat[9..11] += cmat[0..8].(x,y,z). HLE IDENTICAL. [V]
- anglev/anglep: real DSP uses a polynomial atan (shared sub @0x1ad);
  HLE uses libm atan2. Differ only at sub-degree precision. [V approx]

CONCLUSION: the VF geometry math the HLE implements is faithful to the real
DSP microcode for the functions that build model/sub-part transforms. The
Pai-hat divergence is therefore NOT in these per-function math ops. Remaining
unverified VF functions are the higher-numbered helpers (vmat_* family
0x4a-0x4f, f80/f89/f92-103, matrix_unrot 0x4f, matrix_rtrans 0x52) and the
matrix stack discipline — these are the next decomp targets.

## 10. Authoritative disassembly (upstream MB86233 decoder) [V]

The mame2010 disassembler has 57 undecoded opcodes (UNKOP/UNKDUAL). Upstream
MAME's rewritten MB86233 disassembler (Olivier Galibert) decodes the same VF
ROM with only 3 gaps. Both read an IDENTICAL opcode stream (verified: zero
diff on all 2048 addr:word pairs), so the ROM dump and decode are sound.
Authoritative disassembly: vf_tgp_5724_upstream_disasm.asm.

The upstream decode reveals precise ALU ops (fml, fmsd, fspd, fsmd, fmrd =
float multiply / multiply-sub / etc.) and the +0x200 BRAM bank offset on
matrix addressing that mame2010's decoder omitted.

### 10.1 Re-confirmed faithful (authoritative decode)
- transform_point core (0x1c3): matrix*vector + translation. HLE matches. [V]

### 10.2 matrix_rtrans (fn 0x52, handler 0x307) — VERIFIED FAITHFUL [V]
CORRECTION of an earlier mis-read. The handler 0x307-0x310 uses (xn+$b)
POST-INCREMENT addressing (confirmed against the verified transform_point
core, which uses the same mode). Decoded correctly:
  x0=cmat base; read cmat[0] (side), then post-inc to read cmat[9],cmat[10],
  cmat[11]; output those three.
=> fn 0x52 returns cmat[9..11], EXACTLY matching the HLE matrix_rtrans.
The fml/fspd multiply-accumulate block at 0x311 belongs to fn 0x53 (a
DIFFERENT function, HLE=NULL), which VF does not dispatch. The earlier
"divergence" was reading fn 0x53's body as part of fn 0x52 before the
function boundary (brif #0x27 at 0x310) and post-increment semantics were
established. Lesson: confirm addressing mode + handler boundary before
claiming a divergence.

### 10.3 Status of VF geometry functions vs real DSP [V]
All VF-dispatched transform/matrix functions decoded and cross-checked so
far MATCH the real DSP: transform_point, matrix_rotx/y/z, matrix_trans,
matrix_rtrans. anglev/anglep differ only by atan implementation (sub-degree).
No gross geometry divergence has been found in the per-function math. If the
cap artifact is TGP-side, it is more likely in call sequencing / matrix-stack
discipline or one of the not-yet-decoded VF functions (0x54-0x67 block) than
in the core transforms.


### 10.4 vmat_flatten (fn 0x5f, handler 0x474) — HLE Y-zeroing is CORRECT [V]
Investigated as a cap suspect. The ROM handler loops over 16 vectors calling
the shared matrix-multiply core (0x218) — superficially like vmat_mul with no
visible Y-column zero. Hypothesis: HLE's m[1]=m[4]=m[7]=m[10]=0 was an
HLE-only addition. TESTED by removing the zeroing and gating through the
harness: result SCRAMBLED both fighters (Pai +23%, Jacky +12%, floor +14%,
hat +18%) — visibly worse. So the HLE Y-zeroing IS correct; the real DSP must
perform equivalent zeroing (likely the 0x218 core's stride skips the Y term,
which the interleaved x0/x1 addressing obscures in static disasm). vmat_flatten
is NOT the cap bug. Note: vmat_flatten operates on the mat_vector[] bank
(secondary, used for ground/shadow projection), not the primary cmat that
positions character geometry — so it was unlikely to be the cap cause anyway.

### 10.5 Verified-faithful tally (VF-dispatched, decoded so far) [V]
transform_point, matrix_rotx/y/z, matrix_trans, matrix_rtrans, matrix_unrot,
vmat_mul (structural), vmat_flatten (behaviorally, via harness). No confirmed
geometry divergence found in any VF TGP function decoded to date. The cap
artifact remains unattributed to a specific TGP-math divergence.


### 10.6 Full VF geometry-function audit COMPLETE [V]
Decoded from 315-5724.bin and cross-checked against the HLE, all VF-dispatched
matrix/vector functions:

  fn    name            ROM    verdict
  0x05  matrix_push     0xba   (stack) faithful
  0x10  matrix_ident    0x11e  faithful
  0x12  matrix_trans    0x12e  faithful (cmat[9..11] += cmat.(x,y,z))
  0x14  matrix_rotx     0x147  faithful (cols {3,6}, cos/sin signs match)
  0x15  matrix_roty     0x14a  faithful (cols {6,0})
  0x16  matrix_rotz     0x14d  faithful (cols {0,3})
  0x1a  transform_point 0x150  faithful (core 0x1c3)
  0x4a  vmat_store      0x29e  faithful (mat_vector[a]=cmat)
  0x4b  vmat_restore    0x2ab  faithful (cmat=mat_vector[a])
  0x4d  vmat_mul        0x2d0  faithful (mat_vector[b]=mat_vector[a].cmat, core 0x218)
  0x4e  vmat_read       0x2e8  faithful (output mat_vector[a])
  0x4f  matrix_unrot    0x2f6  faithful (zero rot, diag=1, keep trans)
  0x52  matrix_rtrans   0x307  faithful (output cmat[9..11])
  0x54  vmat_save       0x355  faithful (16x16-stride block to ext RAM)
  0x55  vmat_load       0x360  faithful (block from ext RAM)
  0x5f  vmat_flatten    0x474  faithful (HLE Y-zero proven correct via harness)
  0x61  ram_trans       0x495  faithful (ram_get translation -> cmat trans)
  0x0a  anglev          0xf4   faithful (atan, sub-degree precision diff only)
  0x0f  anglep          0x112  faithful (atan, sub-degree precision diff only)

CONCLUSION: The VF TGP HLE math is FAITHFUL to the real MB86233 microcode
across the entire set of geometry functions VF uses. There is NO TGP-math
divergence responsible for the Pai-hat artifact.

=> The mame2010-vs-upstream hat difference is therefore NOT in the TGP geometry
math. It must lie elsewhere: (a) the V60-side data VF feeds the TGP (display
list / model selection / matrix inputs), (b) TGP RAM-port hookup timing (which
upstream explicitly flags as imperfect and the reason VF is MACHINE_NOT_WORKING
upstream), or (c) it is not actually a defect — the two emulators make different
but individually-defensible approximations and "upstream looks right here" may
not generalize. Further work should trace the V60->TGP data for the head object,
not the TGP math functions (now exhaustively verified).

## 11. End-to-end pipeline verification: mame2010 vs upstream [V]

Hypothesis tested: the Pai-hat difference comes from V60 emulation gaps or
the model-data path. Every stage was checked against upstream:

1. V60 CPU core: unimplemented ops are ROTC/UPDATE/UPDPTE (MMU/system only);
   documented inaccuracies are MUL-overflow-flags / DIVX-width — none affect
   float geometry. VF hits no unimplemented-op fatalerror. [V]
2. V60 -> renderer transforms: traced the display-list object matrices
   (opcode 0xb). ALL have determinant exactly 1.0000 (perfect orthonormal
   rotations) with sane translations, incl. Pai head (obj 3, T=[-0.58,0.20,
   4.72]). The V60 feeds CLEAN matrices. [V]
3. TGP geometry math: all 18 VF-dispatched functions decompiled from
   315-5724.bin and verified faithful (section 10). [V]
4. Polygon model ROMs: identical files (mpr-16096..16103), identical CRCs,
   identical ROM_LOAD32_WORD interleave + ROM_REGION32_LE, in both trees. [V]
5. Polygon read path: poly_data[poly_adr+0..5] as float, byte-identical in
   both trees. [V]
6. Renderer (transform/project/cull/zmode/sort/clip): byte-identical to
   upstream model1_v.cpp (section 4). [V]

CONCLUSION: every stage of the VF geometry pipeline in mame2010 is either
byte-identical to upstream or verified faithful to the real hardware. The
character meshes are transformed by the RENDERER via the display-list matrix
(not by TGP transform_point), and that matrix is clean.

Given this, the residual mame2010-vs-upstream hat difference is most
consistent with: TGP RAM-port hookup/timing differences (which upstream
itself flags as imperfect and is why upstream VF is MACHINE_NOT_WORKING), OR
the two builds differing in a subtle runtime-state way not reachable by
static/data comparison. It is NOT explained by V60 completeness, the model
data, the TGP math, or the renderer — all now verified.


## 12. TGP RAM-port (copro_ram) hookup — divergence found, behaviorally inert for VF [V]

A REAL code divergence exists in the V60<->TGP shared-RAM port auto-increment:
  - mame2010 VF HLE (model1_tgp_copro_ram_r/w): increments ram_adr
    UNCONDITIONALLY on write and on high-half read.
  - upstream (v60_copro_ram_r/w) AND mame2010's own VR path
    (model1_vr_tgp_ram_r): increment only if (ram_adr & 0x8000); address
    masked & 0x1fff.
  => mame2010 is internally inconsistent (VF port != VR port).

TESTED whether it matters for VF: instrumented all copro-RAM accesses during
gameplay. Result: 4538 writes, 0 reads, and EVERY access had bit 0x8000 SET.
When 0x8000 is set, unconditional-increment == conditional-increment, so the
two implementations are behaviorally IDENTICAL for VF's actual usage. VF never
hits the divergent case (bit clear). Therefore this is NOT the hat cause,
though it is a latent correctness bug worth fixing for general accuracy
(could affect other Model 1 titles that use the port with bit 0x8000 clear).
