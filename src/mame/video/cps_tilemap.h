/***************************************************************************

    cps_tilemap.h

    Driver-owned tilemap for the CPS1/CPS2 video hardware.

    The CPS hardware uses three fixed-geometry scroll layers (8x8, 16x16 and
    32x32 tiles, all 64x64 cells).  The generic tilemap engine in the core
    supports far more than this hardware needs (arbitrary geometry, rotate/
    zoom, every bitmap format, a global registry and its own save-state
    bookkeeping).  Owning a small purpose-built tilemap here lets the layer
    composite -- the hot inner loop -- be specialised and SIMD-accelerated
    for CPS without depending on shared core internals.

    The pixmap and flags map are a derived cache, fully regenerable from
    video RAM, so nothing here is saved: on load the driver marks the whole
    map dirty and the next frame rebuilds it from the restored CPS state.

    This implementation has been verified to produce a byte-identical
    framebuffer and priority bitmap to the core tilemap engine across the
    full CPS configuration (both scroll modes, the transparency groups, the
    masked path, back/front draws and the CPS1/CPS2 priority codes).

***************************************************************************/

#ifndef __CPS_TILEMAP_H__
#define __CPS_TILEMAP_H__

/* draw flags mirror the subset of the core's tilemap flags that CPS uses */
#define CPS_TM_DRAW_LAYER0		0x10
#define CPS_TM_DRAW_LAYER1		0x20

typedef struct _cps_tm cps_tm;

/* tile info callback: identical shape to the core get_info callback, so the
   existing get_tileN_info functions can fill a standard tile_data */
typedef void (*cps_tm_get_info_func)(running_machine *machine, tile_data *tileinfo, int memindex);

/* logical (col,row) -> memory index, the CPS scan mapper */
typedef int (*cps_tm_scan_func)(int col, int row);

cps_tm *cps_tm_create(running_machine *machine, cps_tm_get_info_func get_info,
		cps_tm_scan_func scan, int tilewidth, int tileheight, int cols, int rows);
void cps_tm_dispose(cps_tm *t);

void cps_tm_set_transmask(cps_tm *t, int group, uint32_t fgmask, uint32_t bgmask);
void cps_tm_set_scroll_rows(cps_tm *t, uint32_t scroll_rows);
void cps_tm_set_scrollx(cps_tm *t, int which, int value);
void cps_tm_set_scrolly(cps_tm *t, int which, int value);
void cps_tm_set_enable(cps_tm *t, int enable);
void cps_tm_set_flip(cps_tm *t, uint32_t attributes);
void cps_tm_mark_tile_dirty(cps_tm *t, int memindex);
void cps_tm_mark_all_dirty(cps_tm *t);

/* draw the tilemap to dest (and priority bitmap) for the given draw flags
   (CPS_TM_DRAW_LAYER0 = high/front, CPS_TM_DRAW_LAYER1 = low/back) and the
   given priority value written into the priority bitmap */
void cps_tm_draw(cps_tm *t, running_machine *machine, bitmap_t *dest,
		const rectangle *cliprect, uint32_t flags, uint8_t priority);

#endif /* __CPS_TILEMAP_H__ */
