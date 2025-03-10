/*************************************************************************

    Cross Pang

*************************************************************************/

class crospang_state : public driver_device
{
public:
	crospang_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_fg_videoram(*this, "fg_videoram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT16> m_fg_videoram;
	required_shared_ptr<UINT16> m_bg_videoram;
	required_shared_ptr<UINT16> m_spriteram;
//  UINT16 *  m_paletteram;       // currently this uses generic palette handling

	/* video-related */
	tilemap_t   *m_bg_layer;
	tilemap_t   *m_fg_layer;
	int       m_bestri_tilebank;

	/* devices */
	cpu_device *m_audiocpu;
	DECLARE_WRITE16_MEMBER(crospang_soundlatch_w);
	DECLARE_WRITE16_MEMBER(bestri_tilebank_w);
	DECLARE_WRITE16_MEMBER(bestri_bg_scrolly_w);
	DECLARE_WRITE16_MEMBER(bestri_fg_scrolly_w);
	DECLARE_WRITE16_MEMBER(bestri_fg_scrollx_w);
	DECLARE_WRITE16_MEMBER(bestri_bg_scrollx_w);
	DECLARE_WRITE16_MEMBER(crospang_fg_scrolly_w);
	DECLARE_WRITE16_MEMBER(crospang_bg_scrolly_w);
	DECLARE_WRITE16_MEMBER(crospang_fg_scrollx_w);
	DECLARE_WRITE16_MEMBER(crospang_bg_scrollx_w);
	DECLARE_WRITE16_MEMBER(crospang_fg_videoram_w);
	DECLARE_WRITE16_MEMBER(crospang_bg_videoram_w);
	DECLARE_DRIVER_INIT(crospang);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_crospang(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
