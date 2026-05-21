/*----------- defined in drivers/nmk16.c -----------*/

extern uint16_t* nmk16_mainram;


/*----------- defined in video/nmk16.c -----------*/

extern uint16_t *nmk_bgvideoram0;
extern uint16_t *nmk_bgvideoram1;
extern uint16_t *nmk_bgvideoram2;
extern uint16_t *nmk_bgvideoram3;

extern uint16_t *nmk_fgvideoram,*nmk_txvideoram;
extern uint16_t *gunnail_scrollram, *gunnail_scrollramy;
extern uint16_t *afega_scroll_0;
extern uint16_t *afega_scroll_1;


WRITE16_HANDLER( nmk_bgvideoram0_w );
WRITE16_HANDLER( nmk_bgvideoram1_w );
WRITE16_HANDLER( nmk_bgvideoram2_w );
WRITE16_HANDLER( nmk_bgvideoram3_w );

WRITE16_HANDLER( nmk_fgvideoram_w );
WRITE16_HANDLER( nmk_txvideoram_w );
WRITE16_HANDLER( nmk_scroll_w );
WRITE16_HANDLER( nmk_scroll_2_w );
WRITE16_HANDLER( nmk_flipscreen_w );
WRITE16_HANDLER( nmk_tilebank_w );
WRITE16_HANDLER( bioship_scroll_w );
WRITE16_HANDLER( bioship_bank_w );
WRITE16_HANDLER( mustang_scroll_w );
WRITE16_HANDLER( bioshipbg_scroll_w );
WRITE16_HANDLER( vandyke_scroll_w );
WRITE16_HANDLER( vandykeb_scroll_w );
WRITE16_HANDLER( manybloc_scroll_w );

VIDEO_START( macross );
VIDEO_UPDATE( manybloc );
VIDEO_START( gunnail );
VIDEO_START( macross2 );
VIDEO_START( raphero );
VIDEO_START( bjtwin );
VIDEO_START( bioship );
VIDEO_START( strahl );
VIDEO_UPDATE( bioship );
VIDEO_UPDATE( strahl );
VIDEO_UPDATE( macross );
VIDEO_UPDATE( gunnail );
VIDEO_UPDATE( bjtwin );
VIDEO_UPDATE( tharrier );
VIDEO_UPDATE( hachamf );
VIDEO_UPDATE( tdragon );
VIDEO_EOF( nmk );
VIDEO_EOF( strahl );

VIDEO_START( afega );
VIDEO_START( grdnstrm );
VIDEO_START( firehawk );
VIDEO_UPDATE( afega );
VIDEO_UPDATE( redhawkb );
VIDEO_UPDATE(redhawki );
VIDEO_UPDATE( bubl2000 );
VIDEO_UPDATE( firehawk );
