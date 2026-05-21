/*----------- defined in video/sei_crtc.c -----------*/

extern uint16_t *seibucrtc_sc0vram,*seibucrtc_sc1vram,*seibucrtc_sc2vram,*seibucrtc_sc3vram;
extern uint16_t *seibucrtc_vregs;
extern uint16_t seibucrtc_sc0bank;

WRITE16_HANDLER( seibucrtc_sc0vram_w );
WRITE16_HANDLER( seibucrtc_sc1vram_w );
WRITE16_HANDLER( seibucrtc_sc2vram_w );
WRITE16_HANDLER( seibucrtc_sc3vram_w );
WRITE16_HANDLER( seibucrtc_vregs_w );
void seibucrtc_sc0bank_w(uint16_t data);
VIDEO_START( seibu_crtc );
VIDEO_UPDATE( seibu_crtc );
