#include "video/poly.h"

class taitojc_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, taitojc_state(machine)); }

	taitojc_state(running_machine &machine) { }

	int texture_x;
	int texture_y;

	uint32_t dsp_rom_pos;
	uint16_t dsp_tex_address;
	uint16_t dsp_tex_offset;


	int first_dsp_reset;
	int viewport_data[3];

	int32_t projected_point_x;
	int32_t projected_point_y;
	int32_t projection_data[3];

	int32_t intersection_data[3];

	uint8_t *texture;
	bitmap_t *framebuffer;
	bitmap_t *zbuffer;

	uint32_t *vram;
	uint32_t *objlist;

	//int debug_tex_pal;

	int gfx_index;

	uint32_t *char_ram;
	uint32_t *tile_ram;
	tilemap_t *tilemap;

	poly_manager *poly;

	uint32_t *main_ram;
	uint16_t *dsp_shared_ram;
	uint32_t *palette_ram;

	uint16_t *polygon_fifo;
	int polygon_fifo_ptr;

	uint8_t mcu_comm_main;
	uint8_t mcu_comm_hc11;
	uint8_t mcu_data_main;
	uint8_t mcu_data_hc11;
};


/*----------- defined in video/taitojc.c -----------*/

READ32_HANDLER(taitojc_tile_r);
WRITE32_HANDLER(taitojc_tile_w);
READ32_HANDLER(taitojc_char_r);
WRITE32_HANDLER(taitojc_char_w);
void taitojc_clear_frame(running_machine *machine);
void taitojc_render_polygons(running_machine *machine, uint16_t *polygon_fifo, int length);

VIDEO_START(taitojc);
VIDEO_UPDATE(taitojc);
