/***************************************************************************

    Flak Attack / MX5000 (Konami GX669)

    Driver by:
        Manuel Abadia <manu@teleline.es>

    TO DO:
        -What does 0x900X do? (Z80)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/hd6309/hd6309.h"
#include "sound/2151intf.h"
#include "sound/k007232.h"
#include "video/konicdev.h"
#include "includes/konamipt.h"
#include "includes/flkatck.h"


INTERRUPT_GEN_MEMBER(flkatck_state::flkatck_interrupt)
{

	if (m_irq_enabled)
		device.execute().set_input_line(HD6309_IRQ_LINE, HOLD_LINE);
}

WRITE8_MEMBER(flkatck_state::flkatck_bankswitch_w)
{
	/* bits 3-4: coin counters */
	coin_counter_w(machine(), 0, data & 0x08);
	coin_counter_w(machine(), 1, data & 0x10);

	/* bits 0-1: bank # */
	if ((data & 0x03) != 0x03)  /* for safety */
		membank("bank1")->set_entry(data & 0x03);
}

READ8_MEMBER(flkatck_state::flkatck_ls138_r)
{
	int data = 0;

	switch ((offset & 0x1c) >> 2)
	{
		case 0x00:
			if (offset & 0x02)
				data = ioport((offset & 0x01) ? "COIN" : "DSW3")->read();
			else
				data = ioport((offset & 0x01) ? "P2" : "P1")->read();
			break;
		case 0x01:
			if (offset & 0x02)
				data = ioport((offset & 0x01) ? "DSW1" : "DSW2")->read();
			break;
	}

	return data;
}

WRITE8_MEMBER(flkatck_state::flkatck_ls138_w)
{

	switch ((offset & 0x1c) >> 2)
	{
		case 0x04:  /* bankswitch */
			flkatck_bankswitch_w(space, 0, data);
			break;
		case 0x05:  /* sound code number */
			soundlatch_byte_w(space, 0, data);
			break;
		case 0x06:  /* Cause interrupt on audio CPU */
			m_audiocpu->set_input_line(0, HOLD_LINE);
			break;
		case 0x07:  /* watchdog reset */
			watchdog_reset_w(space, 0, data);
			break;
	}
}

/* Protection - an external multiplyer connected to the sound CPU */
READ8_MEMBER(flkatck_state::multiply_r)
{
	return (m_multiply_reg[0] * m_multiply_reg[1]) & 0xff;
}

WRITE8_MEMBER(flkatck_state::multiply_w)
{
	m_multiply_reg[offset] = data;
}


static ADDRESS_MAP_START( flkatck_map, AS_PROGRAM, 8, flkatck_state )
	AM_RANGE(0x0000, 0x0007) AM_RAM_WRITE(flkatck_k007121_regs_w)                                   /* 007121 registers */
	AM_RANGE(0x0008, 0x03ff) AM_RAM                                                                 /* RAM */
	AM_RANGE(0x0400, 0x041f) AM_READWRITE(flkatck_ls138_r, flkatck_ls138_w)                         /* inputs, DIPS, bankswitch, counters, sound command */
	AM_RANGE(0x0800, 0x0bff) AM_RAM_WRITE(paletteram_xBBBBBGGGGGRRRRR_byte_le_w) AM_SHARE("paletteram") /* palette */
	AM_RANGE(0x1000, 0x1fff) AM_RAM                                                                 /* RAM */
	AM_RANGE(0x2000, 0x3fff) AM_RAM_WRITE(flkatck_k007121_w) AM_SHARE("k007121_ram")                    /* Video RAM (007121) */
	AM_RANGE(0x4000, 0x5fff) AM_ROMBANK("bank1")                                                            /* banked ROM */
	AM_RANGE(0x6000, 0xffff) AM_ROM                                                                 /* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( flkatck_sound_map, AS_PROGRAM, 8, flkatck_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM                                             /* ROM */
	AM_RANGE(0x8000, 0x87ff) AM_RAM                                             /* RAM */
	AM_RANGE(0x9000, 0x9000) AM_READWRITE(multiply_r, multiply_w)               /* ??? */
//  AM_RANGE(0x9001, 0x9001) AM_RAM                                             /* ??? */
	AM_RANGE(0x9004, 0x9004) AM_READNOP                                         /* ??? */
	AM_RANGE(0x9006, 0x9006) AM_WRITENOP                                        /* ??? */
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_byte_r)                             /* soundlatch_byte_r */
	AM_RANGE(0xb000, 0xb00d) AM_DEVREADWRITE_LEGACY("konami", k007232_r, k007232_w) /* 007232 registers */
	AM_RANGE(0xc000, 0xc001) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)           /* YM2151 */
ADDRESS_MAP_END


static INPUT_PORTS_START( flkatck )
	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "Invalid", SW1)
	/* "Invalid" = both coin slots disabled */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "30K, Every 70K" )
	PORT_DIPSETTING(    0x10, "40K, Every 80K" )
	PORT_DIPSETTING(    0x08, "30K Only" )
	PORT_DIPSETTING(    0x00, "40K Only" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Upright Controls" )      PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_SERVICE_DIPLOC(   0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW3:4" )        /* Listed as "Unused" */
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	KONAMI8_SYSTEM_UNK

	PORT_START("P1")
	KONAMI8_B12_UNK(1)

	PORT_START("P2")
	KONAMI8_B12_UNK(2)
INPUT_PORTS_END

static const gfx_layout gfxlayout =
{
	8,8,
	0x80000/32,
	4,
	{ 0, 1, 2, 3 },
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static GFXDECODE_START( flkatck )
	GFXDECODE_ENTRY( "gfx1", 0, gfxlayout, 0, 32 )
GFXDECODE_END

static void volume_callback0(device_t *device, int v)
{
	k007232_set_volume(device, 0, (v >> 4) * 0x11, 0);
	k007232_set_volume(device, 1, 0, (v & 0x0f) * 0x11);
}

static const k007232_interface k007232_config =
{
	volume_callback0    /* external port callback */
};


void flkatck_state::machine_start()
{
	UINT8 *ROM = memregion("maincpu")->base();

	membank("bank1")->configure_entries(0, 3, &ROM[0x10000], 0x2000);

	m_audiocpu = machine().device<cpu_device>("audiocpu");
	m_k007121 = machine().device("k007121");

	save_item(NAME(m_irq_enabled));
	save_item(NAME(m_multiply_reg));
	save_item(NAME(m_flipscreen));
}

void flkatck_state::machine_reset()
{

	k007232_set_bank(machine().device("konami"), 0, 1);

	m_irq_enabled = 0;
	m_multiply_reg[0] = 0;
	m_multiply_reg[1] = 0;
	m_flipscreen = 0;
}

static MACHINE_CONFIG_START( flkatck, flkatck_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD6309,3000000*4) /* HD63C09EP, 24/8 MHz */
	MCFG_CPU_PROGRAM_MAP(flkatck_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", flkatck_state,  flkatck_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80,3579545)   /* NEC D780C-1, 3.579545 MHz */
	MCFG_CPU_PROGRAM_MAP(flkatck_sound_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(37*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 35*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(flkatck_state, screen_update_flkatck)

	MCFG_GFXDECODE(flkatck)
	MCFG_PALETTE_LENGTH(512)


	MCFG_K007121_ADD("k007121")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", 3579545)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_SOUND_ADD("konami", K007232, 3579545)
	MCFG_SOUND_CONFIG(k007232_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)
MACHINE_CONFIG_END



ROM_START( mx5000 )
	ROM_REGION( 0x18000, "maincpu", 0 )     /* 6309 code */
	ROM_LOAD( "669_r01.16c", 0x010000, 0x006000, CRC(79b226fc) SHA1(3bc4d93717230fecd54bd08a0c3eeedc1c8f571d) ) /* banked ROM */
	ROM_CONTINUE(            0x006000, 0x00a000 )           /* fixed ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* 64k for the SOUND CPU */
	ROM_LOAD( "669_m02.16b", 0x000000, 0x008000, CRC(7e11e6b9) SHA1(7a7d65a458b15842a6345388007c8f682aec20a7) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "mask4m.5e",   0x000000, 0x080000, CRC(ff1d718b) SHA1(d44fe3ed5a3ba1b3036264e37f9cd3500b706635) ) /* tiles + sprites */

	ROM_REGION( 0x040000, "konami", 0 ) /* 007232 data (chip 1) */
	ROM_LOAD( "mask2m.11a",  0x000000, 0x040000, CRC(6d1ea61c) SHA1(9e6eb9ac61838df6e1f74e74bb72f3edf1274aed) )
ROM_END

ROM_START( flkatck )
	ROM_REGION( 0x18000, "maincpu", 0 )     /* 6309 code */
	ROM_LOAD( "669_p01.16c", 0x010000, 0x006000, CRC(c5cd2807) SHA1(22ddd911a23954ff2d52552e07323f5f0ddaeead) ) /* banked ROM */
	ROM_CONTINUE(            0x006000, 0x00a000 )           /* fixed ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* 64k for the SOUND CPU */
	ROM_LOAD( "669_m02.16b", 0x000000, 0x008000, CRC(7e11e6b9) SHA1(7a7d65a458b15842a6345388007c8f682aec20a7) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "mask4m.5e",   0x000000, 0x080000, CRC(ff1d718b) SHA1(d44fe3ed5a3ba1b3036264e37f9cd3500b706635) ) /* tiles + sprites */

	ROM_REGION( 0x040000, "konami", 0 ) /* 007232 data (chip 1) */
	ROM_LOAD( "mask2m.11a",  0x000000, 0x040000, CRC(6d1ea61c) SHA1(9e6eb9ac61838df6e1f74e74bb72f3edf1274aed) )
ROM_END



GAME( 1987, mx5000,  0,      flkatck, flkatck, driver_device, 0, ROT90, "Konami", "MX5000", GAME_SUPPORTS_SAVE )
GAME( 1987, flkatck, mx5000, flkatck, flkatck, driver_device, 0, ROT90, "Konami", "Flak Attack (Japan)", GAME_SUPPORTS_SAVE )
