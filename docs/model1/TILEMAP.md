# Model 1 2D Tilemap Layer (Sega System 24 tile hardware)

The Model 1 reuses the Sega System 24 tilemap chip (315-5292 family) for its 2D
overlay: the medium-resolution background/foreground tile planes drawn around the
3D scene (scores, life bars, attract text, backdrops).  This document describes
the tile format, the four-layer structure, scrolling, priority, and the palette
so the recompilation rasteriser can reproduce it without the tilemap chip.

## A. Address space (V60 side)

    700000   tile RAM window (sys24 tile/char/scroll/control RAM)
    780000   character (pattern) RAM
    900000   palette RAM (16-bit entries; loaded from ROM 0xFD0770 at boot)
    910000   colour translation / mixer control

Internally the tile RAM is addressed as 16-bit words:

    0x0000-0x3fff   four tilemap name tables, 0x1000 words each:
                    layer0 scroll page = 0x0000, layer0 window page = 0x1000,
                    layer1 scroll page = 0x2000, layer1 window page = 0x3000.
                    Each name table is 64x64 tiles.
    0x4000-0x43ff   per-line horizontal scroll table for layer 0 (0x200 words)
    0x4200-0x45ff   per-line horizontal scroll table for layer 1
    0x5000-0x5003   horizontal scroll value per layer pair
    0x5004-0x5007   vertical scroll value + control per layer pair
    0x6000-0x67ff   8-pixel layer-select mask for pair 0
    0x6800-0x6fff   8-pixel layer-select mask for pair 1

## B. Tile name-table entry (16 bits)

    bit 15      category / priority bit (1 = high-priority half of the layer)
    bits 14-7   colour (palette select), 8 bits -> (val >> 7) & 0xff
    bits  6-0   plus the low bits masked by tile_mask (0x3fff for Model 1) =
                tile number into character RAM

Tiles are 8x8 pixels, 4 bits per pixel (16 colours/tile), 32 bytes per tile.
Pen 0 is transparent.  The byte order inside a character word is big-endian
(WORD_XOR_BE) so the recomp must unswap when reading 4bpp nibbles.

## C. Four layers, two pairs, 8-pixel selection

The hardware exposes four tilemaps (layer 0..3) grouped into two pairs.  Within a
pair, an 8-pixel-granular mask (the 0x6000 / 0x6800 tables) selects, for each
8-pixel column span, which of the two layers in the pair is shown -- this is how
the hardware does per-column windowing (e.g. a status strip over a scrolling
background).  A "window" layer uses the inverted mask.

The category bit (name-table bit 15) splits each layer into a low and high
priority half drawn at different points in the layer stack.

## D. Scrolling

- Flat scroll: hscr = tile_ram[0x5000 + (layer>>1)], vscr = tile_ram[0x5004 +
  (layer>>1)].  Only the low 9 bits are used (& 0x1ff); the plane is 512 pixels.
- vscr bit 15 = layer disable (skip the layer entirely).
- Per-line horizontal scroll: when hscr bit 15 is set and the control word
  ctrl = tile_ram[0x5004 + ((layer>>1)&2)] has bits 13-14 set, the layer reads a
  per-scanline H value from the 0x4000 + 0x200*layer table.  Control modes 1, 2
  and 3 select different split/window behaviours (mode 1 swaps the two paired
  layers above a V threshold; modes 2/3 clip left/right of the per-line H).

## E. Draw order on Model 1

The frame is composited as: the four low-priority layer halves, then the 3D
scene, then the four high-priority halves.  Concretely the renderer draws layers
in the order 6, 4, 2, 0 (the low/even category) beneath the 3D output and 7, 5, 3,
1 (the high/odd category) above it -- i.e. (layer_index<<1 | category) packed into
the call argument, even = below, odd = above.

## F. Palette format (900000, 16-bit per entry)

Each palette word encodes 5-bit-per-channel colour with a shared LSB plus a
highlight flag:

    R = ((word & 0x000f) << 4) | (word & 0x1000 ? 8 : 0)
    G =  (word & 0x00f0)       | (word & 0x2000 ? 8 : 0)
    B = ((word & 0x0f00) >> 4) | (word & 0x4000 ? 8 : 0)
    then each channel |= channel >> 5   (expand to full 8-bit range)
    bit 15 = highlight: store a second "shadow/highlight" copy of the colour at
             palette_index + total_colors/2 (0.6x for shadow, 1-0.6*(255-c) for
             highlight).

So the recomp keeps two palette banks: the normal colour and a shadow/highlight
variant selected by bit 15, used when a tile or sprite pixel requests shadowing.

## G. Recompilation notes

- Tiles, scroll tables, masks and palette all live in RAM (no gfx ROMs), so the
  recomp rasteriser reads them straight from the emulated tile RAM image; no chip
  emulation is needed, only the layout above.
- Implement the layer stack as: for each of the four layers, for each category
  half, walk 64x64 8x8 tiles with the (flat or per-line) scroll applied, sampling
  the 8-pixel selection mask to choose the active layer of the pair, writing pen!=0
  pixels with the decoded palette colour (and the shadow/highlight bank when the
  palette word's bit 15 is set).  Composite below the 3D scene for even category,
  above it for odd.
