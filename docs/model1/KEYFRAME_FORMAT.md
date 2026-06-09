# Model 1 / Virtua Fighter Keyframe Stream Format

This document describes the per-channel keyframe (animation) stream consumed by
the playback routine at FF597C and the channel decoder that follows it.  It
extends the move/animation material in RECOMPILATION.md sections N, S, Z and AA
with the channel type-byte grammar, derived from the V60 disassembly.

## A. Locating a move's keyframe stream

The keyframe data lives in banked ROM, selected by writing the bank index to the
bank register E00004.  FF597C banks in 0x21 and then indexes by the current
move-state held in the entity at offset -0x38:

    state = entity[-0x38]
    if state <  0xF3:  base = 0x100000,  index = state - 1
    if state >= 0xF3:  base = 0x200004,  index = state - 0xF3

i.e. the move-state ID space is split across two bank windows at state 0xF3, and
the keyframe pointer is read from base[index].  A flag (R20 = 1 or 2) is derived
from whether that pointer is below 0x200000; it becomes the per-channel
interpolation-rate shift applied later.

## B. Channel block layout

Each keyframe record is processed as a fixed array of channels with stride 0x2B
(43).  Two cursors are set up one stride apart (entity 0x4AE = current record,
0x4B2 = current + 0x2B) so the decoder can interpolate between the current and
next keyframe.  A per-period step is computed from the animation period at entity
0x650:

    step = ((period - 1) << shift?) ... (FF59D3-FF59DE: dec, shift, inc)

## C. The 43-channel pass and the type byte

For each of the 0x2B channels the decoder reads a type byte from the channel
cursor (R9) and decides whether that channel advances:

    type = channel[k].type        (test.b [R9])
    if type == 0 or type == 1:    channel is STATIC this frame (no step added)
    else:                          channel is INTERPOLATED (add the step to the
                                   running offset R6)

So type 0/1 mark a held/non-animated channel and any other value marks an
animated channel.  The running per-channel offsets are written back to the work
block at entity 0x4B2 (+4 per channel) and the terminating offset stored at 0x55E.

## D. Per-channel value decode (FF5A40 onward)

For each animated channel the decoder then:

1. reads the next byte as a duration/sub-count (movz.bw [R9+], R5); if zero it
   defaults to the animation period (entity 0x650);
2. scales it by the rate shift R20 (shl.w R20, R5) and advances the running
   offset;
3. reads the channel's sample value.  Depending on the channel sub-type it is
   either a raw 16-bit word ([R6] -> R1) or a SIGNED 16-bit value that is
   sign-extended and converted to float (movs.hw [R6], R1; cvt.ws R1, R1) before
   being streamed to the TGP as a command argument (the 0x1000000 / float-constant
   sequence to port R24, result read back from R23).

A second inner pass (R7 = 0x27 = 39) copies the resolved channel halfwords into
the pose/matrix work area at entity 0x45C, again defaulting a zero duration byte
to the period.

## E. Summary of the per-channel grammar

A channel entry in the keyframe stream is, in order:

    [type byte]        0/1 = static (held), other = interpolated
    [duration byte]    0 = use the move's period (entity 0x650); else the count,
                       scaled by the rate shift (1 or 2) from the bank split
    [16-bit value]     raw word for some channels; signed -> float for the TGP-fed
                       channels (sign-extend + cvt.ws)

43 such channels per keyframe record (stride 0x2B); the decoder interpolates
between the current record and the one 0x2B ahead, feeding the resolved values to
the TGP for the skeletal transform and copying the pose channels to the render
work area.

## F. Recompilation notes

- Resolve the move-state to a keyframe pointer with the 0xF3 two-window split and
  the rate-shift flag exactly as in B; off-by-one on the index or the split
  boundary shifts the whole animation.
- For each of the 43 channels apply the type-byte rule (0/1 static, else
  interpolate), the zero-duration-defaults-to-period rule, and the signed-16 ->
  float conversion for the TGP-fed channels.
- The interpolation is a linear blend between the current and next keyframe record
  driven by the per-period step; the recomp can compute the same blend in floats
  directly and skip the TGP round-trip for channels whose only consumer is the
  pose work area.
