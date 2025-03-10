class gatron_state : public driver_device
{
public:
	gatron_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"){ }

	required_shared_ptr<UINT8> m_videoram;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(output_port_0_w);
	DECLARE_WRITE8_MEMBER(gat_videoram_w);
	DECLARE_WRITE8_MEMBER(output_port_1_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_gat(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
