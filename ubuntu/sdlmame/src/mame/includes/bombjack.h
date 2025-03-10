/*************************************************************************

    Bomb Jack

*************************************************************************/

class bombjack_state : public driver_device
{
public:
	bombjack_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t   *m_fg_tilemap;
	tilemap_t   *m_bg_tilemap;
	UINT8       m_background_image;

	/* sound-related */
	UINT8       m_latch;

	UINT8       m_nmi_mask;
	DECLARE_WRITE8_MEMBER(bombjack_soundlatch_w);
	DECLARE_READ8_MEMBER(bombjack_soundlatch_r);
	DECLARE_WRITE8_MEMBER(irq_mask_w);
	DECLARE_WRITE8_MEMBER(bombjack_videoram_w);
	DECLARE_WRITE8_MEMBER(bombjack_colorram_w);
	DECLARE_WRITE8_MEMBER(bombjack_background_w);
	DECLARE_WRITE8_MEMBER(bombjack_flipscreen_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_bombjack(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	TIMER_CALLBACK_MEMBER(soundlatch_callback);
};
