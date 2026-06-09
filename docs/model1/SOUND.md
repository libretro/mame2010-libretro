# Model 1 Sound Subsystem

This document describes the Sega Model 1 sound board: the TMP68000 sound program,
the main-CPU-to-sound command protocol, and the register-level protocols for the
two MultiPCM chips and the YM3438. It is written for a recompilation that replaces
the emulated 68000 + sound chips with native code, and is derived from a
disassembly of Virtua Fighter's sound ROM (epr-16120 / epr-16121) cross-checked
against the driver's memory map and machine configuration.

## A. Hardware

- TMP68000N-10 sound CPU at 10.000 MHz (20 MHz crystal / 2). Verified on hardware.
- Two Sega 315-5560 "MultiPCM" PCM chips ("sega1", "sega2"), each clocked at
  8.000 MHz, each driving both stereo outputs at unity gain.
- One Yamaha YM3438 (OPN2C, a CMOS YM2612) at 8.000 MHz (16 MHz / 2), routed to
  both speakers at 0.60 gain.
- The board generates sound only; amplification is on a separate amp PCB.
- Sound communication from the main PCB arrives over a 6-pin connector (CN2).

## B. Sound 68000 memory map

    000000-0bffff   ROM (program).  epr-16120 at 000000, epr-16121 at 020000,
                    with epr-16121 reloaded at 080000 (so 080000-09ffff mirrors
                    020000-03ffff).
    c20000-c20001   Host command latch (read) / write-latch-1.  Reading pops one
                    byte from the main-CPU->sound FIFO (the byte the V60 wrote via
                    its sound latch).  The status/command byte lives in the low 8
                    bits; the program reads it at c20001 (odd byte).
    c20002-c20003   "V60 ready" (read, always returns 1) / write-latch-2.  The
                    program writes the FIFO read-acknowledge / handshake byte to
                    c20003 (see the reset routine, which pulses c20003 with
                    0/0x40/0x4e/0x37).
    c40000-c40007   MultiPCM #1 ("sega1"), byte-wide on the low data byte.
                    +1 = data port, +3 = slot/key port, +5 = register-address
                    port, +1 (read) = status (bit 0 = busy).
    c40012-c40013   MultiPCM #1 control (write).  The program writes 0x03 here at
                    init.
    c50000-c50001   MultiPCM #1 bank select.  multipcm_set_bank(0x100000*(d&3)).
    c60000-c60007   MultiPCM #2 ("sega2"), same port layout as #1.
    c70000-c70001   MultiPCM #2 bank select.
    d00000-d00007   YM3438, byte-wide on the low data byte.  +1 = part-0 address,
                    +3 = part-0 data, +5 = part-1 address, +7 = part-1 data;
                    reading +1 returns the status byte (bit 7 = busy, bit 1 =
                    timer-A flag, used as the engine tick).
    c10001          Board control latch (write).  0x0f at the very start, then
                    toggled with the queue-rotate value (see 0x618).
    f00000-f0ffff   Work RAM (64 KB).  Initial SSP = 0f0fffe.

## C. Reset and exception vectors

The 68000 vector table occupies 000000-0003ff.

    vector 0 (SSP)        = 0f0fffe   (top of work RAM)
    vector 1 (reset PC)   = 000200
    bus error / addr err / illegal / trace / TRAP#0-3 / all unused IRQ levels
                          = 000100   (a bare "nop; rte" catch-all)
    autovector IRQ2       = 000120   (the host command receiver)

Only IRQ level 2 is used.  The main CPU raises it (HOLD_LINE on the sound CPU's
IRQ2) every time it writes a sound command, so the sound program receives commands
asynchronously and queues them; the foreground loop consumes the queue.

## D. The IRQ2 handler (host command receiver), 000120

The handler is a ring-buffer producer.  In pseudo-code:

    save d0-d7/a1-a6; mask interrupts (sr = 0x2700)
    status = [c20003] & 0x38;  if status != 0 -> hard reset (set SP, jump 000200)
    cmd = [c20001]
    if cmd == 0xff            -> ignore (spurious / idle)
    if cmd >= 0xf0            -> control range, skip enqueue
    if [f01001] (pending len) != 0 -> we are mid-multibyte; just append the byte
    else:
        a0 = queue write pointer (via 0x618 board-rotate helper)
        if cmd has bit 7 set  -> it is a status/length-bearing command:
            store cmd at f01002
            length = 2; if cmd in [c0,e0) length=2; if cmd >= e0 length=3
            store length at f01000
        append cmd to ring buffer at a0++ (wrapping f01000 -> f00000)
        decrement pending length at f01001; when it reaches 0, bump the
        command-count at f01007 (tells the foreground a full command arrived)
    sr = 0x2100; restore; rte

Ring buffer: bytes are stored from f00000 upward and wrap at f01000 back to
f00000.  f01000 = current command's total length, f01001 = bytes still expected,
f01002 = the command's lead byte, f01007 = count of complete commands waiting.

## E. Initialization (000200 onward)

1. Write 0x0f to c10001 (board enable).
2. Clear d0-d7, set up base pointers, call 0x4c8 (pulses the c20003 handshake with
   0,0,0,0x40,0x4e,0x37 to sync the host FIFO).
3. Zero all of work RAM f00000-f0ffff.
4. MultiPCM #1: write 0x03 to c40013, then run a 28-entry (0x1b+1) init table at
   ROM 0x63e, writing three registers per voice via the helper at 0x538:
   register 0x00 = 0x80, register 0x04 = 0x00, register 0x05 = 0xfe.
5. MultiPCM #2: identical, control at c60013, table at 0x65a, helper at 0x57e.
6. YM3438: a 10-entry table at 0x67c then a 36-entry (0x23+1) table at 0x692,
   written via the YM helpers 0x5cc / 0x5f2.
7. Seed the per-channel state blocks at f01300 (0x50 + 0x32 bytes) from the
   pointer table at ROM 0x42ce, push the two MultiPCM bank bytes to c50001/c70001.
8. Enable the YM timer (write 0x2a to YM register 0x27) and drop into the main
   loop at 0x3f0.

## F. Chip-access protocols (the register-write helpers)

MultiPCM (315-5560), helper at 0x538 (sega1, ports c40001/3/5) and 0x57e (sega2,
ports c60001/3/5):

    poll [base+1] until bit 0 (busy) clears
    write address/register index (d7) to [base+5]
    poll [base+1] until bit 0 clears
    write data (d6) to [base+1]

A separate per-voice helper (0x139a) selects sega1 (c40003) or sega2 (c60003) by
bit 3 of the voice's control byte and writes the slot/key value to port +3 -- i.e.
the low voices live on one chip and the high voices on the other, chosen by that
bit.  Bank selection is a single byte to c50001 / c70001 (0x100000-granular).

YM3438 (OPN2C), helper at 0x5cc (part 0) and 0x5f2 (part 1):

    poll [d00001] until bit 7 (busy) clears
    write register index (d7) to address port (part0 = d00001, part1 = d00005)
    poll [d00001] until bit 7 clears
    write data (d5) to data port (part0 = d00003, part1 = d00007)

This is the standard OPN2 two-port address/data sequence, duplicated for the two
register banks (channels 1-3 on part 0, channels 4-6 on part 1).

## G. The main loop and engine tick (0003f0)

The foreground loop is driven by the YM3438 timer-A flag:

    loop:
      st = [d00001]
      if (st & 2):                      # timer-A expired -> one engine tick
          re-arm timer (YM reg 0x27 = 0x2a)
          call 0x424   (sequencer / envelope step)
          jsr  0x1736  (per-voice MultiPCM volume/pan refresh)
      else:                             # no tick due -> service the host queue
          jsr 0x1290
          jsr 0x1308
          jsr 0x6da    (command-queue consumer, section H)
      goto loop

The volume path (0x37e in init, repeated each tick) multiplies each channel's
programmed volume by a master volume at f0101e, shifts right 8, and stores the
result as the effective per-voice level -- a software volume stage on top of the
MultiPCM hardware envelope.

The sequencer step (0x424) walks a 36-entry event list at f01504 (stride 0x0e
bytes/entry) with a per-entry down-counter; entries carry key-on/off bits (tested
with masks 0x81 and 0x70 against threshold 0x50) and advance a playback cursor at
f01018.  This is the music/SE sequencer that turns queued commands into timed
note events.

## H. The host command protocol (000-6da consumer)

The queue consumer at 0x6da dequeues one byte at a time (0x1380: read (a1)+ with
wrap at f01000 -> f00000) and dispatches on the command's high nibble; the low
nibble selects a logical voice/channel (matched against the 13 channel blocks at
f01300):

    0x8n   note-off  on channel n      (handler 0xb30)
    0x9n   note-on   on channel n      (handler 0x7ae)
    0xCn   parameter command (1 data byte follows)
    0xDn   parameter command (1 data byte follows)
    0xEn / status (bit-7) commands carry a length (2 or 3 bytes) as recorded by
           the IRQ handler.

So the V60 drives the sound board with a compact MIDI-like byte stream: a status
byte (command nibble + channel nibble) optionally followed by parameter bytes,
delivered through the IRQ2 ring buffer and consumed in the foreground between
engine ticks.

## I. Recompilation notes

To recompile the sound subsystem natively:

- Replace the ring buffer + IRQ2 producer with a lock-free queue fed by the main
  CPU's sound-latch write; preserve the lead-byte length decoding (bit 7 ->
  2/3-byte commands; >= 0xf0 = control, skipped; 0xff = idle) so command framing
  matches.
- Model the engine tick off a timer at the YM3438 timer-A rate rather than
  polling a status bit; each tick runs the sequencer step and the per-voice
  volume refresh.
- The two MultiPCM chips and the YM3438 can be driven by existing emulation cores
  or native synthesis; the only board-specific behaviour to preserve is the
  channel-to-chip split (bit 3 of the voice control byte), the 0x100000-granular
  bank select, and the software master-volume multiply.
- The command protocol (0x8n/0x9n/0xCn/0xDn note and parameter bytes, low nibble =
  channel) is the contract between the recompiled main CPU and the recompiled
  sound engine; it can be honoured directly without emulating the 68000 at all.
