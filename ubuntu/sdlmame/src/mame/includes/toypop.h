class toypop_state : public driver_device
{
public:
	toypop_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_m68000_sharedram(*this, "m68k_shared"),
		m_bg_image(*this, "bg_image"){ }

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_m68000_sharedram;
	required_shared_ptr<UINT16> m_bg_image;
	tilemap_t *m_bg_tilemap;

	int m_bitmapflip;
	int m_palettebank;
	int m_interrupt_enable_68k;
	UINT8 m_main_irq_mask;
	UINT8 m_sound_irq_mask;
	DECLARE_READ16_MEMBER(toypop_m68000_sharedram_r);
	DECLARE_WRITE16_MEMBER(toypop_m68000_sharedram_w);
	DECLARE_READ8_MEMBER(toypop_main_interrupt_enable_r);
	DECLARE_WRITE8_MEMBER(toypop_main_interrupt_enable_w);
	DECLARE_WRITE8_MEMBER(toypop_main_interrupt_disable_w);
	DECLARE_WRITE8_MEMBER(toypop_sound_interrupt_enable_acknowledge_w);
	DECLARE_WRITE8_MEMBER(toypop_sound_interrupt_disable_w);
	DECLARE_WRITE8_MEMBER(toypop_sound_clear_w);
	DECLARE_WRITE8_MEMBER(toypop_sound_assert_w);
	DECLARE_WRITE8_MEMBER(toypop_m68000_clear_w);
	DECLARE_WRITE8_MEMBER(toypop_m68000_assert_w);
	DECLARE_WRITE16_MEMBER(toypop_m68000_interrupt_enable_w);
	DECLARE_WRITE16_MEMBER(toypop_m68000_interrupt_disable_w);
	DECLARE_WRITE8_MEMBER(toypop_videoram_w);
	DECLARE_WRITE8_MEMBER(toypop_palettebank_w);
	DECLARE_WRITE16_MEMBER(toypop_flipscreen_w);
	DECLARE_READ16_MEMBER(toypop_merged_background_r);
	DECLARE_WRITE16_MEMBER(toypop_merged_background_w);
	DECLARE_READ8_MEMBER(dipA_l);
	DECLARE_READ8_MEMBER(dipA_h);
	DECLARE_READ8_MEMBER(dipB_l);
	DECLARE_READ8_MEMBER(dipB_h);
	DECLARE_WRITE8_MEMBER(out_coin0);
	DECLARE_WRITE8_MEMBER(out_coin1);
	DECLARE_WRITE8_MEMBER(flip);
	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_toypop(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(toypop_main_vblank_irq);
	INTERRUPT_GEN_MEMBER(toypop_sound_timer_irq);
	INTERRUPT_GEN_MEMBER(toypop_m68000_interrupt);
	TIMER_CALLBACK_MEMBER(namcoio_run);
};
