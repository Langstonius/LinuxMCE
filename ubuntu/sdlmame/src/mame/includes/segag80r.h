/*************************************************************************

    Sega G-80 raster hardware

*************************************************************************/

#include "machine/segag80.h"
#include "sound/sn76496.h"

class segag80r_state : public driver_device
{
public:
	segag80r_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_mainram(*this, "mainram"),
		m_videoram(*this, "videoram"),
		m_sn1(*this, "sn1"),
		m_sn2(*this, "sn2"){ }

	required_shared_ptr<UINT8> m_mainram;
	required_shared_ptr<UINT8> m_videoram;

	optional_device<sn76496_device> m_sn1;
	optional_device<sn76496_device> m_sn2;

	UINT8 m_sound_state[2];
	UINT8 m_sound_rate;
	UINT16 m_sound_addr;
	UINT8 m_sound_data;
	UINT8 m_square_state;
	UINT8 m_square_count;
	emu_timer *m_sega005_sound_timer;
	sound_stream *m_sega005_stream;
	UINT8 m_n7751_command;
	UINT8 m_n7751_busy;
	segag80_decrypt_func m_decrypt;
	UINT8 m_background_pcb;
	double m_rweights[3];
	double m_gweights[3];
	double m_bweights[2];
	UINT8 m_video_control;
	UINT8 m_video_flip;
	UINT8 m_vblank_latch;
	tilemap_t *m_spaceod_bg_htilemap;
	tilemap_t *m_spaceod_bg_vtilemap;
	UINT16 m_spaceod_hcounter;
	UINT16 m_spaceod_vcounter;
	UINT8 m_spaceod_fixed_color;
	UINT8 m_spaceod_bg_control;
	UINT8 m_spaceod_bg_detect;
	tilemap_t *m_bg_tilemap;
	UINT8 m_bg_enable;
	UINT8 m_bg_char_bank;
	UINT16 m_bg_scrollx;
	UINT16 m_bg_scrolly;
	UINT8 m_pignewt_bg_color_offset;
	DECLARE_WRITE8_MEMBER(mainram_w);
	DECLARE_WRITE8_MEMBER(vidram_w);
	DECLARE_WRITE8_MEMBER(monsterb_vidram_w);
	DECLARE_WRITE8_MEMBER(pignewt_vidram_w);
	DECLARE_WRITE8_MEMBER(sindbadm_vidram_w);
	DECLARE_READ8_MEMBER(mangled_ports_r);
	DECLARE_READ8_MEMBER(spaceod_mangled_ports_r);
	DECLARE_READ8_MEMBER(spaceod_port_fc_r);
	DECLARE_WRITE8_MEMBER(coin_count_w);
	DECLARE_WRITE8_MEMBER(segag80r_videoram_w);
	DECLARE_READ8_MEMBER(segag80r_video_port_r);
	DECLARE_WRITE8_MEMBER(segag80r_video_port_w);
	DECLARE_READ8_MEMBER(spaceod_back_port_r);
	DECLARE_WRITE8_MEMBER(spaceod_back_port_w);
	DECLARE_WRITE8_MEMBER(monsterb_videoram_w);
	DECLARE_WRITE8_MEMBER(monsterb_back_port_w);
	DECLARE_WRITE8_MEMBER(pignewt_videoram_w);
	DECLARE_WRITE8_MEMBER(pignewt_back_color_w);
	DECLARE_WRITE8_MEMBER(pignewt_back_port_w);
	DECLARE_WRITE8_MEMBER(sindbadm_videoram_w);
	DECLARE_WRITE8_MEMBER(sindbadm_back_port_w);
	DECLARE_WRITE8_MEMBER(astrob_sound_w);
	DECLARE_WRITE8_MEMBER(spaceod_sound_w);
	DECLARE_READ8_MEMBER(n7751_rom_r);
	DECLARE_READ8_MEMBER(n7751_command_r);
	DECLARE_READ8_MEMBER(n7751_t1_r);
	DECLARE_INPUT_CHANGED_MEMBER(service_switch);
	DECLARE_WRITE8_MEMBER(usb_ram_w);
	DECLARE_WRITE8_MEMBER(sindbadm_soundport_w);
	DECLARE_WRITE8_MEMBER(sindbadm_misc_w);
	DECLARE_WRITE8_MEMBER(sindbadm_sn1_SN76496_w);
	DECLARE_WRITE8_MEMBER(sindbadm_sn2_SN76496_w);
	DECLARE_DRIVER_INIT(spaceod);
	DECLARE_DRIVER_INIT(sindbadm);
	DECLARE_DRIVER_INIT(pignewt);
	DECLARE_DRIVER_INIT(monsterb);
	DECLARE_DRIVER_INIT(005);
	DECLARE_DRIVER_INIT(monster2);
	DECLARE_DRIVER_INIT(astrob);
	TILE_GET_INFO_MEMBER(spaceod_get_tile_info);
	TILEMAP_MAPPER_MEMBER(spaceod_scan_rows);
	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	virtual void machine_start();
	virtual void video_start();
	UINT32 screen_update_segag80r(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(segag80r_vblank_start);
	INTERRUPT_GEN_MEMBER(sindbadm_vblank_start);
	TIMER_CALLBACK_MEMBER(vblank_latch_clear);
	DECLARE_WRITE8_MEMBER(sega005_sound_a_w);
	DECLARE_WRITE8_MEMBER(sega005_sound_b_w);
	DECLARE_WRITE8_MEMBER(monsterb_sound_a_w);
	DECLARE_WRITE8_MEMBER(monsterb_sound_b_w);
	DECLARE_READ8_MEMBER(n7751_status_r);
	DECLARE_WRITE8_MEMBER(n7751_command_w);
	DECLARE_WRITE8_MEMBER(n7751_rom_control_w);
	DECLARE_WRITE8_MEMBER(n7751_p2_w);
};


/*----------- defined in audio/segag80r.c -----------*/

MACHINE_CONFIG_EXTERN( astrob_sound_board );
MACHINE_CONFIG_EXTERN( 005_sound_board );
MACHINE_CONFIG_EXTERN( spaceod_sound_board );
MACHINE_CONFIG_EXTERN( monsterb_sound_board );

/*----------- defined in video/segag80r.c -----------*/

#define G80_BACKGROUND_NONE         0
#define G80_BACKGROUND_SPACEOD      1
#define G80_BACKGROUND_MONSTERB     2
#define G80_BACKGROUND_PIGNEWT      3
#define G80_BACKGROUND_SINDBADM     4
