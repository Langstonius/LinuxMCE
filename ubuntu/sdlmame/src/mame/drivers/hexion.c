/***************************************************************************

Hexion (GX122) (c) 1992 Konami

driver by Nicola Salmoria

Notes:
- There are probably palette PROMs missing. Palette data doesn't seem to be
  written anywhere in RAM.
- The board has a 052591, which is used for protection in Thunder Cross and
  S.P.Y. In this game, however, the only thing it seems to do is clear the
  screen.
  This is the program for the 052591:
00: 5f 80 01 e0 08
01: df 80 00 e0 0c
02: df 90 02 e0 0c
03: df a0 03 e0 0c
04: df b0 0f e0 0c
05: df c0 ff bf 0c
06: 5c 02 00 33 0c
07: 5f 80 04 80 0c
08: 5c 0e 00 2b 0c
09: df 70 00 cb 08
0a: 5f 80 00 80 0c
0b: 5c 04 00 2b 0c
0c: df 60 00 cb 08
0d: 5c 0c 1f e9 0c
0e: 4c 0c 2d e9 08
0f: 5f 80 03 80 0c
10: 5c 04 00 2b 0c
11: 5f 00 00 cb 00
12: 5f 80 02 a0 0c
13: df d0 00 c0 04
14: 01 3a 00 f3 0a
15: 5c 08 00 b3 0c
16: 5c 0e 00 13 0c
17: 5f 80 00 a0 0c
18: 5c 00 00 13 0c
19: 5c 08 00 b3 0c
1a: 5c 00 00 13 0c
1b: 84 5a 00 b3 0c
1c: 48 0a 5b d1 0c
1d: 5f 80 00 e0 08
1e: 5f 00 1e fd 0c
1f: 5f 80 01 a0 0c
20: df 20 00 cb 08
21: 5c 08 00 b3 0c
22: 5f 80 03 00 0c
23: 5c 08 00 b3 0c
24: 5f 80 00 80 0c
25: 5c 00 00 33 0c
26: 5c 08 00 93 0c
27: 9f 91 ff cf 0e
28: 5c 84 00 20 0c
29: 84 00 00 b3 0c
2a: 49 10 69 d1 0c
2b: 5f 80 00 e0 08
2c: 5f 00 2c fd 0c
2d: 5f 80 01 a0 0c
2e: df 20 00 cb 08
2f: 5c 08 00 b3 0c
30: 5f 80 03 00 0c
31: 5c 00 00 b3 0c
32: 5f 80 01 00 0c
33: 5c 08 00 b3 0c
34: 5f 80 00 80 0c
35: 5c 00 00 33 0c
36: 5c 08 00 93 0c
37: 9f 91 ff cf 0e
38: 5c 84 00 20 0c
39: 84 00 00 b3 0c
3a: 49 10 79 d1 0c
3b: 5f 80 00 e0 08
3c: 5f 00 3c fd 0c
3d: ff ff ff ff ff
3e: ff ff ff ff ff
3f: ff ff ff ff ff

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/okim6295.h"
#include "sound/k051649.h"
#include "includes/konamipt.h"
#include "includes/hexion.h"
#include "machine/k053252.h"


WRITE8_MEMBER(hexion_state::coincntr_w)
{
//logerror("%04x: coincntr_w %02x\n",space.device().safe_pc(),data);

	/* bits 0/1 = coin counters */
	coin_counter_w(machine(), 0,data & 0x01);
	coin_counter_w(machine(), 1,data & 0x02);

	/* bit 5 = flip screen */
	flip_screen_set(data & 0x20);

	/* other bit unknown */
if ((data & 0xdc) != 0x10) popmessage("coincntr %02x",data);
}

WRITE_LINE_MEMBER(hexion_state::hexion_irq_ack_w)
{
	machine().device("maincpu")->execute().set_input_line(0, CLEAR_LINE);
}

WRITE_LINE_MEMBER(hexion_state::hexion_nmi_ack_w)
{
	machine().device("maincpu")->execute().set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

static ADDRESS_MAP_START( hexion_map, AS_PROGRAM, 8, hexion_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_ROMBANK("bank1")
	AM_RANGE(0xa000, 0xbfff) AM_RAM
	AM_RANGE(0xc000, 0xdffe) AM_READWRITE(hexion_bankedram_r, hexion_bankedram_w)
	AM_RANGE(0xdfff, 0xdfff) AM_WRITE(hexion_bankctrl_w)
	AM_RANGE(0xe800, 0xe87f) AM_DEVREADWRITE_LEGACY("konami", k051649_waveform_r, k051649_waveform_w)
	AM_RANGE(0xe880, 0xe889) AM_DEVWRITE_LEGACY("konami", k051649_frequency_w)
	AM_RANGE(0xe88a, 0xe88e) AM_DEVWRITE_LEGACY("konami", k051649_volume_w)
	AM_RANGE(0xe88f, 0xe88f) AM_DEVWRITE_LEGACY("konami", k051649_keyonoff_w)
	AM_RANGE(0xe8e0, 0xe8ff) AM_DEVREADWRITE_LEGACY("konami", k051649_test_r, k051649_test_w)
	AM_RANGE(0xf000, 0xf00f) AM_DEVREADWRITE_LEGACY("k053252",k053252_r,k053252_w)
	AM_RANGE(0xf200, 0xf200) AM_DEVWRITE("oki", okim6295_device, write)
	AM_RANGE(0xf400, 0xf400) AM_READ_PORT("DSW1")
	AM_RANGE(0xf401, 0xf401) AM_READ_PORT("DSW2")
	AM_RANGE(0xf402, 0xf402) AM_READ_PORT("P1")
	AM_RANGE(0xf403, 0xf403) AM_READ_PORT("P2")
	AM_RANGE(0xf440, 0xf440) AM_READ_PORT("DSW3")
	AM_RANGE(0xf441, 0xf441) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xf480, 0xf480) AM_WRITE(hexion_bankswitch_w)
	AM_RANGE(0xf4c0, 0xf4c0) AM_WRITE(coincntr_w)
	AM_RANGE(0xf500, 0xf500) AM_WRITE(hexion_gfxrom_select_w)
	AM_RANGE(0xf540, 0xf540) AM_READ(watchdog_reset_r)
ADDRESS_MAP_END



static INPUT_PORTS_START( hexion )
	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), DEF_STR( Free_Play ), SW1)

	PORT_START("DSW2")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x70, 0x40, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:5,6,7")
	PORT_DIPSETTING(    0x70, DEF_STR( Easiest ) )          // "1"
	PORT_DIPSETTING(    0x60, DEF_STR( Very_Easy) )         // "2"
	PORT_DIPSETTING(    0x50, DEF_STR( Easy ) )             // "3"
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )           // "4"
	PORT_DIPSETTING(    0x30, DEF_STR( Medium_Hard ) )      // "5"
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )             // "6"
	PORT_DIPSETTING(    0x10, DEF_STR( Very_Hard ) )        // "7"
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )          // "8"
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xfa, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )

	PORT_START("P1")
	KONAMI8_B12_START(1)

	PORT_START("P2")
	KONAMI8_B12_START(2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL )   /* 052591? game waits for it to be 0 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ 0, 1, 2, 3 },
	{ RGN_FRAC(1,2)+0*4, RGN_FRAC(1,2)+1*4, 0*4, 1*4, RGN_FRAC(1,2)+2*4, RGN_FRAC(1,2)+3*4, 2*4, 3*4 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static GFXDECODE_START( hexion )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 16 )
GFXDECODE_END

TIMER_DEVICE_CALLBACK_MEMBER(hexion_state::hexion_scanline)
{
	int scanline = param;

	if(scanline == 256)
		machine().device("maincpu")->execute().set_input_line(0, ASSERT_LINE);
	else if ((scanline == 85) || (scanline == 170)) //TODO
		machine().device("maincpu")->execute().set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

static const k053252_interface hexion_k053252_intf =
{
	"screen",
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(hexion_state,hexion_irq_ack_w),
	DEVCB_DRIVER_LINE_MEMBER(hexion_state,hexion_nmi_ack_w),
	0, 0
};

static MACHINE_CONFIG_START( hexion, hexion_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,24000000/4) /* Z80B 6 MHz */
	MCFG_CPU_PROGRAM_MAP(hexion_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", hexion_state, hexion_scanline, "screen", 0, 1)

	MCFG_K053252_ADD("k053252", 24000000/2, hexion_k053252_intf)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 36*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(hexion_state, screen_update_hexion)

	MCFG_GFXDECODE(hexion)
	MCFG_PALETTE_LENGTH(256)

	MCFG_PALETTE_INIT(RRRR_GGGG_BBBB)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MCFG_SOUND_ADD("konami", K051649, 24000000/16)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( hexion )
	ROM_REGION( 0x34800, "maincpu", 0 ) /* ROMs + space for additional RAM */
	ROM_LOAD( "122jab01.bin", 0x00000, 0x20000, CRC(eabc6dd1) SHA1(e74c1f1f2fcf8973f0741a2d544f25c8639448bf) )
	ROM_RELOAD(               0x10000, 0x20000 )    /* banked at 8000-9fff */

	ROM_REGION( 0x80000, "gfx1", 0 )    /* addressable by the main CPU */
	ROM_LOAD( "122a07.bin",   0x00000, 0x40000, CRC(22ae55e3) SHA1(41bdc990f69416b639542e2186a3610c16389063) )
	ROM_LOAD( "122a06.bin",   0x40000, 0x40000, CRC(438f4388) SHA1(9e23805c9642a237daeaf106187d1e1e0692434d) )

	ROM_REGION( 0x40000, "oki", 0 ) /* OKIM6295 samples */
	ROM_LOAD( "122a05.bin",   0x0000, 0x40000, CRC(bcc831bf) SHA1(c3382065dd0069a4dc0bde2d9931ec85b0bffc73) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "122a04.10b",   0x0000, 0x0100, CRC(506eb8c6) SHA1(3bff7cf286942d8bdbc3998245c3de20981fbecb) )
	ROM_LOAD( "122a03.11b",   0x0100, 0x0100, CRC(590c4f64) SHA1(db4b34f8c5fdfea034a94d65873f6fb842f123e9) )
	ROM_LOAD( "122a02.13b",   0x0200, 0x0100, CRC(5734305c) SHA1(c72e59acf79a4db1a5a9d827eef899c0675336f2) )
ROM_END


GAME( 1992, hexion, 0, hexion, hexion, driver_device, 0, ROT0, "Konami", "Hexion (Japan ver JAB)", 0 )
