/*----------- defined in video/wc90.c -----------*/

extern uint8_t *wc90_fgvideoram,*wc90_bgvideoram,*wc90_txvideoram;

extern uint8_t *wc90_scroll0xlo, *wc90_scroll0xhi;
extern uint8_t *wc90_scroll1xlo, *wc90_scroll1xhi;
extern uint8_t *wc90_scroll2xlo, *wc90_scroll2xhi;

extern uint8_t *wc90_scroll0ylo, *wc90_scroll0yhi;
extern uint8_t *wc90_scroll1ylo, *wc90_scroll1yhi;
extern uint8_t *wc90_scroll2ylo, *wc90_scroll2yhi;

VIDEO_START( wc90 );
VIDEO_START( wc90t );
WRITE8_HANDLER( wc90_fgvideoram_w );
WRITE8_HANDLER( wc90_bgvideoram_w );
WRITE8_HANDLER( wc90_txvideoram_w );
VIDEO_UPDATE( wc90 );
