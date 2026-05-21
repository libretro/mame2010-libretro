/*************************************************************************

                      -= IGS Lord Of Gun =-

*************************************************************************/

/*----------- defined in video/lordgun.c -----------*/

extern uint16_t *lordgun_priority_ram, lordgun_priority;

extern uint16_t *lordgun_vram_0, *lordgun_scroll_x_0, *lordgun_scroll_y_0;
extern uint16_t *lordgun_vram_1, *lordgun_scroll_x_1, *lordgun_scroll_y_1;
extern uint16_t *lordgun_vram_2, *lordgun_scroll_x_2, *lordgun_scroll_y_2;
extern uint16_t *lordgun_vram_3, *lordgun_scroll_x_3, *lordgun_scroll_y_3;
extern uint16_t *lordgun_scrollram;
extern int lordgun_whitescreen;

typedef struct _lordgun_gun_data lordgun_gun_data;
struct _lordgun_gun_data
{
	int		scr_x,	scr_y;
	uint16_t	hw_x,	hw_y;
};

extern lordgun_gun_data lordgun_gun[2];

WRITE16_HANDLER( lordgun_paletteram_w );
WRITE16_HANDLER( lordgun_vram_0_w );
WRITE16_HANDLER( lordgun_vram_1_w );
WRITE16_HANDLER( lordgun_vram_2_w );
WRITE16_HANDLER( lordgun_vram_3_w );

float lordgun_crosshair_mapper(const input_field_config *field, float linear_value);
void lordgun_update_gun(running_machine *machine, int i);

VIDEO_START( lordgun );
VIDEO_UPDATE( lordgun );
