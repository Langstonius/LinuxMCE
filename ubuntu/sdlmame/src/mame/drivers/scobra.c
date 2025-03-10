/***************************************************************************

 Super Cobra hardware


TODO:
----

- Need correct color PROMs for Super Bond

- Dark Planet background graphics

- Explosion sound in Scramble/Super Cobra repeats

- Armored Car probably has some other effect(s) during an explosion.
  It uses both POUT1 and POUT2.


Notes/Tidbits:
-------------

- Armored Car sets Port C as well, but it's input only and the games uses other
  bits for the 2nd player controls.  Maybe the games was meant to use 2 joysticks
  at one time.

- Video Hustler and its two bootlegs all have identical code, the only
  differences are the title, copyright removed, different encryptions or
  no encryption, plus hustlerb has a different memory map.

- In Tazmania, when set to Upright mode, player 2 left skips the current
  level

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/7474.h"
#include "sound/ay8910.h"
#include "includes/scramble.h"


class scobra_state : public scramble_state
{
public:
	scobra_state(const machine_config &mconfig, device_type type, const char *tag)
		: scramble_state(mconfig, type, tag),
			m_soundram(*this, "soundram") { }

	optional_shared_ptr<UINT8> m_soundram;
	DECLARE_READ8_MEMBER(scobra_soundram_r);
	DECLARE_WRITE8_MEMBER(scobra_soundram_w);
	DECLARE_READ8_MEMBER(scobra_type2_ppi8255_0_r);
	DECLARE_READ8_MEMBER(scobra_type2_ppi8255_1_r);
	DECLARE_READ8_MEMBER(hustler_ppi8255_0_r);
	DECLARE_READ8_MEMBER(hustler_ppi8255_1_r);
	DECLARE_WRITE8_MEMBER(scobra_type2_ppi8255_0_w);
	DECLARE_WRITE8_MEMBER(scobra_type2_ppi8255_1_w);
	DECLARE_WRITE8_MEMBER(hustler_ppi8255_0_w);
	DECLARE_WRITE8_MEMBER(hustler_ppi8255_1_w);
	DECLARE_CUSTOM_INPUT_MEMBER(stratgyx_coinage_r);
};


static const gfx_layout scobra_charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout scobra_spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};


static GFXDECODE_START( scobra )
	GFXDECODE_ENTRY( "gfx1", 0x0000, scobra_charlayout,   0, 8 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, scobra_spritelayout, 0, 8 )
GFXDECODE_END



static const ay8910_interface hustler_ay8910_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_DRIVER_MEMBER(driver_device, soundlatch_byte_r),
	DEVCB_HANDLER(frogger_portB_r),
	DEVCB_NULL,
	DEVCB_NULL
};

READ8_MEMBER(scobra_state::scobra_type2_ppi8255_0_r){ return m_ppi8255_0->read(space, offset >> 2); }
READ8_MEMBER(scobra_state::scobra_type2_ppi8255_1_r){ return m_ppi8255_1->read(space, offset >> 2); }
WRITE8_MEMBER(scobra_state::scobra_type2_ppi8255_0_w){ m_ppi8255_0->write(space, offset >> 2, data); }
WRITE8_MEMBER(scobra_state::scobra_type2_ppi8255_1_w){ m_ppi8255_1->write(space, offset >> 2, data); }

READ8_MEMBER(scobra_state::hustler_ppi8255_0_r){ return m_ppi8255_0->read(space, offset >> 3); }
READ8_MEMBER(scobra_state::hustler_ppi8255_1_r){ return m_ppi8255_1->read(space, offset >> 3); }
WRITE8_MEMBER(scobra_state::hustler_ppi8255_0_w){ m_ppi8255_0->write(space, offset >> 3, data); }
WRITE8_MEMBER(scobra_state::hustler_ppi8255_1_w){ m_ppi8255_1->write(space, offset >> 3, data); }

static ADDRESS_MAP_START( type1_map, AS_PROGRAM, 8, scobra_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x8bff) AM_RAM_WRITE(galaxold_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x8c00, 0x8fff) AM_RAM_WRITE(galaxold_videoram_w)  /* mirror */
	AM_RANGE(0x8c00, 0x8fff) AM_READ(galaxold_videoram_r)   /* mirror */
	AM_RANGE(0x9000, 0x903f) AM_RAM_WRITE(galaxold_attributesram_w) AM_SHARE("attributesram")
	AM_RANGE(0x9040, 0x905f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x9060, 0x907f) AM_RAM AM_SHARE("bulletsram")
	AM_RANGE(0x9080, 0x90ff) AM_RAM
	AM_RANGE(0x9800, 0x9803) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0xa000, 0xa003) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0xa801, 0xa801) AM_WRITE(galaxold_nmi_enable_w)
	AM_RANGE(0xa802, 0xa802) AM_WRITE(galaxold_coin_counter_w)
	AM_RANGE(0xa804, 0xa804) AM_WRITE(galaxold_stars_enable_w)
	AM_RANGE(0xa806, 0xa806) AM_WRITE(galaxold_flip_screen_x_w)
	AM_RANGE(0xa807, 0xa807) AM_WRITE(galaxold_flip_screen_y_w)
	AM_RANGE(0xb000, 0xb000) AM_READ(watchdog_reset_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( type2_map, AS_PROGRAM, 8, scobra_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x883f) AM_RAM_WRITE(galaxold_attributesram_w) AM_SHARE("attributesram")
	AM_RANGE(0x8840, 0x885f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x8860, 0x887f) AM_RAM AM_SHARE("bulletsram")
	AM_RANGE(0x8880, 0x88ff) AM_RAM
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(galaxold_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x9400, 0x97ff) AM_READWRITE(galaxold_videoram_r, galaxold_videoram_w) /* mirror */
	AM_RANGE(0x9800, 0x9800) AM_READ(watchdog_reset_r)
	AM_RANGE(0xa000, 0xa00f) AM_READWRITE(scobra_type2_ppi8255_0_r, scobra_type2_ppi8255_0_w)
	AM_RANGE(0xa800, 0xa80f) AM_READWRITE(scobra_type2_ppi8255_1_r, scobra_type2_ppi8255_1_w)
	AM_RANGE(0xb000, 0xb000) AM_WRITE(galaxold_stars_enable_w)
	AM_RANGE(0xb004, 0xb004) AM_WRITE(galaxold_nmi_enable_w)
	AM_RANGE(0xb006, 0xb006) AM_WRITE(galaxold_coin_counter_0_w)
	AM_RANGE(0xb008, 0xb008) AM_WRITE(galaxold_coin_counter_1_w)
	AM_RANGE(0xb00c, 0xb00c) AM_WRITE(galaxold_flip_screen_y_w)
	AM_RANGE(0xb00e, 0xb00e) AM_WRITE(galaxold_flip_screen_x_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( hustler_map, AS_PROGRAM, 8, scobra_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x8bff) AM_RAM_WRITE(galaxold_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x9000, 0x903f) AM_RAM_WRITE(galaxold_attributesram_w) AM_SHARE("attributesram")
	AM_RANGE(0x9040, 0x905f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x9060, 0x907f) AM_RAM AM_SHARE("bulletsram")
	AM_RANGE(0x9080, 0x90ff) AM_RAM
	AM_RANGE(0xa802, 0xa802) AM_WRITE(galaxold_flip_screen_x_w)
	AM_RANGE(0xa804, 0xa804) AM_WRITE(galaxold_nmi_enable_w)
	AM_RANGE(0xa806, 0xa806) AM_WRITE(galaxold_flip_screen_y_w)
	AM_RANGE(0xa80e, 0xa80e) AM_WRITENOP    /* coin counters */
	AM_RANGE(0xb800, 0xb800) AM_READ(watchdog_reset_r)
	AM_RANGE(0xd000, 0xd01f) AM_READWRITE(hustler_ppi8255_0_r, hustler_ppi8255_0_w)
	AM_RANGE(0xe000, 0xe01f) AM_READWRITE(hustler_ppi8255_1_r, hustler_ppi8255_1_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( hustlerb_map, AS_PROGRAM, 8, scobra_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x8bff) AM_RAM_WRITE(galaxold_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x9000, 0x903f) AM_RAM_WRITE(galaxold_attributesram_w) AM_SHARE("attributesram")
	AM_RANGE(0x9040, 0x905f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x9060, 0x907f) AM_RAM AM_SHARE("bulletsram")
	AM_RANGE(0x9080, 0x90ff) AM_RAM
	AM_RANGE(0xa801, 0xa801) AM_WRITE(galaxold_nmi_enable_w)
	AM_RANGE(0xa802, 0xa802) AM_WRITENOP    /* coin counters */
	AM_RANGE(0xa806, 0xa806) AM_WRITE(galaxold_flip_screen_y_w)
	AM_RANGE(0xa807, 0xa807) AM_WRITE(galaxold_flip_screen_x_w)
	AM_RANGE(0xb000, 0xb000) AM_READ(watchdog_reset_r)
	AM_RANGE(0xc100, 0xc103) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0xc200, 0xc203) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( hustlerb3_map, AS_PROGRAM, 8, scobra_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(galaxold_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x9800, 0x983f) AM_RAM_WRITE(galaxold_attributesram_w) AM_SHARE("attributesram")
	AM_RANGE(0x9840, 0x985f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x9860, 0x987f) AM_RAM AM_SHARE("bulletsram")
	AM_RANGE(0x9880, 0x98ff) AM_RAM
	AM_RANGE(0xb001, 0xb001) AM_WRITE(galaxold_nmi_enable_w)
	AM_RANGE(0xb006, 0xb006) AM_WRITE(galaxold_flip_screen_y_w)
	AM_RANGE(0xb007, 0xb007) AM_WRITE(galaxold_flip_screen_x_w)
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("IN0")
	AM_RANGE(0xa800, 0xa800) AM_READ_PORT("IN1")
	AM_RANGE(0xb000, 0xb000) AM_READ_PORT("IN2")
	AM_RANGE(0xb800, 0xb800) AM_READ(watchdog_reset_r)

	// NOTE: hardware does not have any 8255 chip. Is this handled through TTL, or a leftover?
	// Is b800 the actual sound comms? The soundroms are not dumped, so we can't know...
//  AM_RANGE(0xb800, 0xb800) AM_WRITENOP
	AM_RANGE(0xc200, 0xc203) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mimonkey_map, AS_PROGRAM, 8, scobra_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x8bff) AM_RAM_WRITE(galaxold_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x8c00, 0x8fff) AM_READWRITE(galaxold_videoram_r, galaxold_videoram_w) /* mirror */
	AM_RANGE(0x9000, 0x903f) AM_RAM_WRITE(galaxold_attributesram_w) AM_SHARE("attributesram")
	AM_RANGE(0x9040, 0x905f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x9060, 0x907f) AM_RAM AM_SHARE("bulletsram")
	AM_RANGE(0x9080, 0x90ff) AM_RAM
	AM_RANGE(0x9800, 0x9803) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0xa000, 0xa003) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0xa801, 0xa801) AM_WRITE(galaxold_nmi_enable_w)
	AM_RANGE(0xa800, 0xa802) AM_WRITE(galaxold_gfxbank_w)
	AM_RANGE(0xa806, 0xa806) AM_WRITE(galaxold_flip_screen_x_w)
	AM_RANGE(0xa807, 0xa807) AM_WRITE(galaxold_flip_screen_y_w)
	AM_RANGE(0xb000, 0xb000) AM_READ(watchdog_reset_r)
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END


READ8_MEMBER(scobra_state::scobra_soundram_r)
{
	return m_soundram[offset & 0x03ff];
}

WRITE8_MEMBER(scobra_state::scobra_soundram_w)
{
	m_soundram[offset & 0x03ff] = data;
}

static ADDRESS_MAP_START( scobra_sound_map, AS_PROGRAM, 8, scobra_state )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_READWRITE(scobra_soundram_r, scobra_soundram_w)
	AM_RANGE(0x8000, 0x83ff) AM_WRITENOP AM_SHARE("soundram")  /* only here to initialize pointer */
	AM_RANGE(0x9000, 0x9fff) AM_WRITE(scramble_filter_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( scobra_sound_io_map, AS_IO, 8, scobra_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x10) AM_DEVWRITE_LEGACY("ay1", ay8910_address_w)
	AM_RANGE(0x20, 0x20) AM_DEVREADWRITE_LEGACY("ay1", ay8910_r, ay8910_data_w)
	AM_RANGE(0x40, 0x40) AM_DEVWRITE_LEGACY("ay2", ay8910_address_w)
	AM_RANGE(0x80, 0x80) AM_DEVREADWRITE_LEGACY("ay2", ay8910_r, ay8910_data_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( hustler_sound_map, AS_PROGRAM, 8, scobra_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM
	AM_RANGE(0x6000, 0x6fff) AM_WRITE(frogger_filter_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( hustler_sound_io_map, AS_IO, 8, scobra_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x40, 0x40) AM_DEVREADWRITE_LEGACY("aysnd", ay8910_r, ay8910_data_w)
	AM_RANGE(0x80, 0x80) AM_DEVWRITE_LEGACY("aysnd", ay8910_address_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( hustlerb_sound_map, AS_PROGRAM, 8, scobra_state )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x6000, 0x6fff) AM_WRITE(frogger_filter_w)
	AM_RANGE(0x8000, 0x8fff) AM_RAM_READ(scobra_soundram_r) AM_SHARE("soundram")  /* only here to initialize pointer */
ADDRESS_MAP_END

static ADDRESS_MAP_START( hustlerb_sound_io_map, AS_IO, 8, scobra_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x40, 0x40) AM_DEVWRITE_LEGACY("aysnd", ay8910_address_w)
	AM_RANGE(0x80, 0x80) AM_DEVREADWRITE_LEGACY("aysnd", ay8910_r, ay8910_data_w)
ADDRESS_MAP_END


/* stratgyx coinage DIPs are spread across two input ports */
CUSTOM_INPUT_MEMBER(scobra_state::stratgyx_coinage_r)
{
	int bit_mask = (FPTR)param;
	return (ioport("IN4")->read() & bit_mask) ? 0x01 : 0x00;
}


static INPUT_PORTS_START( stratgyx )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "255 (Cheat)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, scobra_state,stratgyx_coinage_r, (void *)0x01) /* lower 2 coinage DIPs */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, scobra_state,stratgyx_coinage_r, (void *)0x02) /* lower 2 coinage DIPs */
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, scobra_state,stratgyx_coinage_r, (void *)0x04) /* upper 2 coinage DIPs */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, scobra_state,stratgyx_coinage_r, (void *)0x08) /* upper 2 coinage DIPs */
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  /* none of these appear to be used */
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN4")      /* fake coinage DIPs.  read via IN2 and IN3 */
	PORT_DIPNAME( 0x0f, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x0a, "A 2/1  B 1/3" )
	/*PORT_DIPSETTING(    0x02, "A 2/1  B 1/3" )*/
	PORT_DIPSETTING(    0x09, "A 2/1  B 1/6" )
	PORT_DIPSETTING(    0x03, "A 4/3  B 1/3" )
	PORT_DIPSETTING(    0x0c, "A 1/1  B 2/1" )
	PORT_DIPSETTING(    0x07, "A 1/1  B 1/2" )
	PORT_DIPSETTING(    0x01, "A 1/1  B 1/3" )
	PORT_DIPSETTING(    0x06, "A 1/1  B 1/5" )
	/*PORT_DIPSETTING(    0x0b, "A 1/1  B 1/5" )*/
	PORT_DIPSETTING(    0x05, "A 1/1  B 1/7" )
	PORT_DIPSETTING(    0x0d, "A 1/2  B 1/1" )
	PORT_DIPSETTING(    0x0e, "A 1/3  B 3/1" )
	PORT_DIPSETTING(    0x0f, "A 1/4  B 4/1" )
	PORT_DIPSETTING(    0x04, "A 1/99  B 2/1" )
	/*PORT_DIPSETTING(    0x08, "A 1/99  B 2/1" )*/
	PORT_DIPSETTING(    0x00, "A 1/99  B 1/3" )
INPUT_PORTS_END

/* cocktail mode is N/A */
static INPUT_PORTS_START( darkplnt )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )    // and START1
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )    // and START2
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "Bonus Occurrence" )
	PORT_DIPSETTING(    0x01, "Once" )
	PORT_DIPSETTING(    0x00, "Every" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_BIT( 0xfc, 0x00, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF,scobra_state, darkplnt_custom_r, "DIAL") /* scrambled dial */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x06, 0x02, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, "Coin A 1/2 Coin B 2/1" )
	PORT_DIPSETTING(    0x04, "Coin A 1/3 Coin B 3/1" )
	PORT_DIPSETTING(    0x06, "Coin A 1/4 Coin B 4/1" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "100k" )
	PORT_DIPSETTING(    0x08, "200k" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DIAL")
	PORT_BIT( 0xfc, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) /* scrambled dial */
INPUT_PORTS_END

static INPUT_PORTS_START( tazmania )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x06, 0x02, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, "Coin A 1/2 Coin B 2/1" )
	PORT_DIPSETTING(    0x04, "Coin A 1/3 Coin B 3/1" )
	PORT_DIPSETTING(    0x06, "Coin A 1/4 Coin B 4/1" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/* cocktail mode is N/A */
static INPUT_PORTS_START( rescue )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_DIPNAME( 0x02, 0x02, "Starting Level" )
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x06, 0x02, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, "Coin A 1/2 Coin B 2/1" )
	PORT_DIPSETTING(    0x04, "Coin A 1/3 Coin B 3/1" )
	PORT_DIPSETTING(    0x06, "Coin A 1/4 Coin B 4/1" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/* cocktail mode is N/A */
static INPUT_PORTS_START( minefld )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_DIPNAME( 0x02, 0x02, "Starting Level" )
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, "Coin A 1/2 Coin B 2/1" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( hustler )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "Infinite (Cheat)" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( hustlerb3 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	// 6-pos dipswitch on mainboard K4
	PORT_DIPNAME( 0x40, 0x00, "Half Coinage" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )    PORT_CONDITION("IN1", 0x40, EQUALS, 0x40)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )    PORT_CONDITION("IN1", 0x40, EQUALS, 0x00)
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )    PORT_CONDITION("IN1", 0x40, EQUALS, 0x40)
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_6C ) )    PORT_CONDITION("IN1", 0x40, EQUALS, 0x00)
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )    PORT_CONDITION("IN1", 0x40, EQUALS, 0x40)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )    PORT_CONDITION("IN1", 0x40, EQUALS, 0x00)
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )    PORT_CONDITION("IN1", 0x40, EQUALS, 0x40)
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_6C ) )    PORT_CONDITION("IN1", 0x40, EQUALS, 0x00)
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "Infinite (Cheat)" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( mimonkey )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_DIPNAME( 0x20, 0x00, "Infinite Lives (Cheat)")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )   /* used, something to do with the bullets */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/* Same as 'mimonkey' but different "Lives" Dip Switch */
static INPUT_PORTS_START( mimonsco )
	PORT_INCLUDE( mimonkey )

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "4" )
INPUT_PORTS_END


static const ay8910_interface scobra_ay8910_interface_2 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_DRIVER_MEMBER(driver_device, soundlatch_byte_r),
	DEVCB_HANDLER(scramble_portB_r),
	DEVCB_NULL,
	DEVCB_NULL
};


static MACHINE_CONFIG_START( type1, scobra_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 18432000/6)    /* 3.072 MHz */
	MCFG_CPU_PROGRAM_MAP(type1_map)

	MCFG_CPU_ADD("audiocpu", Z80,14318000/8)    /* 1.78975 MHz */
	MCFG_CPU_PROGRAM_MAP(scobra_sound_map)
	MCFG_CPU_IO_MAP(scobra_sound_io_map)

	MCFG_7474_ADD("konami_7474", NOOP, WRITELINE(scobra_state,scramble_sh_7474_q_callback))

	MCFG_MACHINE_RESET_OVERRIDE(scobra_state,scramble)

	MCFG_I8255A_ADD( "ppi8255_0", scramble_ppi_0_intf )
	MCFG_I8255A_ADD( "ppi8255_1", scramble_ppi_1_intf )

	MCFG_7474_ADD("7474_9m_1", WRITELINE(scobra_state,galaxold_7474_9m_1_callback), NOOP)
	MCFG_7474_ADD("7474_9m_2", NOOP, WRITELINE(scobra_state,galaxold_7474_9m_2_q_callback))

	MCFG_TIMER_DRIVER_ADD("int_timer", scobra_state, galaxold_interrupt_timer)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(16000.0/132/2)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(scobra_state, screen_update_galaxold)

	MCFG_GFXDECODE(scobra)
	MCFG_PALETTE_LENGTH(32+64+2+1)  /* 32 for characters, 64 for stars, 2 for bullets, 1 for background */

	MCFG_PALETTE_INIT_OVERRIDE(scobra_state,scrambold)
	MCFG_VIDEO_START_OVERRIDE(scobra_state,scrambold)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay1", AY8910, 14318000/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.16)

	MCFG_SOUND_ADD("ay2", AY8910, 14318000/8)
	MCFG_SOUND_CONFIG(scobra_ay8910_interface_2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.16)
MACHINE_CONFIG_END



/* Rescue, Minefield and Strategy X have extra colors, and custom video initialise */
/* routines to set up the graduated color backgound they use */
static MACHINE_CONFIG_DERIVED( rescue, type1 )

	/* basic machine hardware */

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_PALETTE_LENGTH(32+64+2+128)    /* 32 for characters, 64 for stars, 2 for bullets, 128 for background */

	MCFG_PALETTE_INIT_OVERRIDE(scobra_state,rescue)
	MCFG_VIDEO_START_OVERRIDE(scobra_state,rescue)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( minefld, type1 )

	/* basic machine hardware */

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_PALETTE_LENGTH(32+64+2+256)    /* 32 for characters, 64 for stars, 2 for bullets, 256 for background */

	MCFG_PALETTE_INIT_OVERRIDE(scobra_state,minefld)
	MCFG_VIDEO_START_OVERRIDE(scobra_state,minefld)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( mimonkey, type1 )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(mimonkey_map)

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(scobra_state,mimonkey)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( type2, type1 )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(type2_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( stratgyx, type2 )

	/* basic machine hardware */

	MCFG_DEVICE_REMOVE("ppi8255_1")
	MCFG_I8255A_ADD( "ppi8255_1", stratgyx_ppi_1_intf )

	/* video hardware */
	MCFG_PALETTE_LENGTH(32+64+2+8)  /* 32 for characters, 64 for stars, 2 for bullets, 8 for background */

	MCFG_PALETTE_INIT_OVERRIDE(scobra_state,stratgyx)
	MCFG_VIDEO_START_OVERRIDE(scobra_state,stratgyx)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( darkplnt, type2 )

	/* basic machine hardware */

	/* video hardware */
	MCFG_PALETTE_LENGTH(32+64+2) /* 32 for characters, 64 (buffer) for stars, 2 for bullets */

	MCFG_PALETTE_INIT_OVERRIDE(scobra_state,darkplnt)
	MCFG_VIDEO_START_OVERRIDE(scobra_state,darkplnt)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( hustler, scobra_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 18432000/6)    /* 3.072 MHz */
	MCFG_CPU_PROGRAM_MAP(hustler_map)

	MCFG_CPU_ADD("audiocpu",Z80,14318000/8) /* 1.78975 MHz */
	MCFG_CPU_PROGRAM_MAP(hustler_sound_map)
	MCFG_CPU_IO_MAP(hustler_sound_io_map)

	MCFG_7474_ADD("konami_7474", NOOP, WRITELINE(scobra_state,scramble_sh_7474_q_callback))

	MCFG_MACHINE_RESET_OVERRIDE(scobra_state,scramble)

	MCFG_7474_ADD("7474_9m_1", WRITELINE(scobra_state,galaxold_7474_9m_1_callback), NOOP)
	MCFG_7474_ADD("7474_9m_2", NOOP, WRITELINE(scobra_state,galaxold_7474_9m_2_q_callback))

	MCFG_TIMER_DRIVER_ADD("int_timer", scobra_state, galaxold_interrupt_timer)

	/* device config overrides */
	MCFG_I8255A_ADD( "ppi8255_0", scramble_ppi_0_intf )
	MCFG_I8255A_ADD( "ppi8255_1", scramble_ppi_1_intf )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(16000.0/132/2)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(scobra_state, screen_update_galaxold)

	MCFG_GFXDECODE(scobra)
	MCFG_PALETTE_LENGTH(32+64+2)    /* 32 for characters, 64 for stars, 2 for bullets */

	MCFG_PALETTE_INIT_OVERRIDE(scobra_state,galaxold)
	MCFG_VIDEO_START_OVERRIDE(scobra_state,scrambold)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, 14318000/8)
	MCFG_SOUND_CONFIG(hustler_ay8910_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( hustlerb, hustler )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(hustlerb_map)

	MCFG_CPU_MODIFY("audiocpu")
	MCFG_CPU_PROGRAM_MAP(hustlerb_sound_map)
	MCFG_CPU_IO_MAP(hustlerb_sound_io_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( hustlerb3, hustler )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(hustlerb3_map)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( stratgyx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2c_1.bin",     0x0000, 0x1000, CRC(eec01237) SHA1(619aaf6379064395a1166575f207d61c90731bb9) )
	ROM_LOAD( "2e_2.bin",     0x1000, 0x1000, CRC(926cb2d5) SHA1(241e7b0f0d7e20a79299696be28963fd01269e86) )
	ROM_LOAD( "2f_3.bin",     0x2000, 0x1000, CRC(849e2504) SHA1(0ec00a5c77e9d81d69f8b847a50f36af2dacc8d0) )
	ROM_LOAD( "2h_4.bin",     0x3000, 0x1000, CRC(8a64069b) SHA1(ccc0e0441360e3ed93f2a0dab70c3a66edad969f) )
	ROM_LOAD( "2j_5.bin",     0x4000, 0x1000, CRC(78b9b898) SHA1(158e11352d68ed9e4277efbcb7e927e9bdc662f8) )
	ROM_LOAD( "2l_6.bin",     0x5000, 0x1000, CRC(20bae414) SHA1(cd3c03d3b6a3abb64cb86cffa733d5494cb226f7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound code */
	ROM_LOAD( "s1.bin",       0x0000, 0x1000, CRC(713a5db8) SHA1(8ec41cb93cfd856dc5aecace6238240a5d114ce1) )
	ROM_LOAD( "s2.bin",       0x1000, 0x1000, CRC(46079411) SHA1(72bfc39979818309ac5a49654a825f9e4bd0236c) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5f_c2.bin",    0x0000, 0x0800, CRC(7121b679) SHA1(bf76c5b777d1f1468b31524bfe250a1bc4911b09) )
	ROM_LOAD( "5h_c1.bin",    0x0800, 0x0800, CRC(d105ad91) SHA1(fd30d001764cf37ae52bab010db27acdc35fece6) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "strategy.6e",  0x0000, 0x0020, CRC(51a629e1) SHA1(f9826202c91a4a3deb8d815cbaa107b29dce5835) )

	ROM_REGION( 0x0020, "user1", 0 )
	ROM_LOAD( "strategy.10k", 0x0000, 0x0020, CRC(d95c0318) SHA1(83e5355fdca7b4e1fb5a0e6eeaacfbf9561e2c36) )    /* background color map */
ROM_END

ROM_START( stratgys )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2c.cpu",       0x0000, 0x1000, CRC(f2aaaf2b) SHA1(1aa5d0696961bcd43a4659db0a53f443cd3d3985) )
	ROM_LOAD( "2e.cpu",       0x1000, 0x1000, CRC(5873fdc8) SHA1(9707c6f140cd7d51f8bd1e53f3c138491aa302d1) )
	ROM_LOAD( "2f.cpu",       0x2000, 0x1000, CRC(532d604f) SHA1(eaf314d98005caacde8d2ea6240043ebe10650a5) )
	ROM_LOAD( "2h.cpu",       0x3000, 0x1000, CRC(82b1d95e) SHA1(dc5e79e159990e76353dea7b54e52fadfc9f59ab) )
	ROM_LOAD( "2j.cpu",       0x4000, 0x1000, CRC(66e84cde) SHA1(c1e6b9fe0052fe1f521c943a69d60f65fa392147) )
	ROM_LOAD( "2l.cpu",       0x5000, 0x1000, CRC(62b032d0) SHA1(592866f801839550e60a515978a2d97dd414c87a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound code */
	ROM_LOAD( "s1.bin",       0x0000, 0x1000, CRC(713a5db8) SHA1(8ec41cb93cfd856dc5aecace6238240a5d114ce1) )
	ROM_LOAD( "s2.bin",       0x1000, 0x1000, CRC(46079411) SHA1(72bfc39979818309ac5a49654a825f9e4bd0236c) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5f.cpu",       0x0000, 0x0800, CRC(f4aa5ddd) SHA1(2be9c8d8b94c599d4ff12864cbbddd7301be9fb2) )
	ROM_LOAD( "5h.cpu",       0x0800, 0x0800, CRC(548e4635) SHA1(fb8d4d498a98b9d19504f1b6c86dfa0110dcc5a7) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "strategy.6e",  0x0000, 0x0020, CRC(51a629e1) SHA1(f9826202c91a4a3deb8d815cbaa107b29dce5835) )

	ROM_REGION( 0x0020, "user1", 0 )
	ROM_LOAD( "strategy.10k", 0x0000, 0x0020, CRC(d95c0318) SHA1(83e5355fdca7b4e1fb5a0e6eeaacfbf9561e2c36) )
ROM_END

ROM_START( strongx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "strongx.2d",   0x0000, 0x1000, CRC(c2f7268c) SHA1(2943198d82d4bc7990b327c44bbce7e7eb011886) )
	ROM_LOAD( "strongx.2e",   0x1000, 0x1000, CRC(91059422) SHA1(c73b50bc4035bd7268628cf92b21acbafef0456e) )
	ROM_LOAD( "2f_3.bin",     0x2000, 0x1000, CRC(849e2504) SHA1(0ec00a5c77e9d81d69f8b847a50f36af2dacc8d0) )
	ROM_LOAD( "2h_4.bin",     0x3000, 0x1000, CRC(8a64069b) SHA1(ccc0e0441360e3ed93f2a0dab70c3a66edad969f) )
	ROM_LOAD( "2j_5.bin",     0x4000, 0x1000, CRC(78b9b898) SHA1(158e11352d68ed9e4277efbcb7e927e9bdc662f8) )
	ROM_LOAD( "2l_6.bin",     0x5000, 0x1000, CRC(20bae414) SHA1(cd3c03d3b6a3abb64cb86cffa733d5494cb226f7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound code */
	ROM_LOAD( "s1.bin",       0x0000, 0x1000, CRC(713a5db8) SHA1(8ec41cb93cfd856dc5aecace6238240a5d114ce1) )
	ROM_LOAD( "s2.bin",       0x1000, 0x1000, CRC(46079411) SHA1(72bfc39979818309ac5a49654a825f9e4bd0236c) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5f_c2.bin",    0x0000, 0x0800, CRC(7121b679) SHA1(bf76c5b777d1f1468b31524bfe250a1bc4911b09) )
	ROM_LOAD( "5h_c1.bin",    0x0800, 0x0800, CRC(d105ad91) SHA1(fd30d001764cf37ae52bab010db27acdc35fece6) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "strategy.6e",  0x0000, 0x0020, CRC(51a629e1) SHA1(f9826202c91a4a3deb8d815cbaa107b29dce5835) )

	ROM_REGION( 0x0020, "user1", 0 )
	ROM_LOAD( "strategy.10k", 0x0000, 0x0020, CRC(d95c0318) SHA1(83e5355fdca7b4e1fb5a0e6eeaacfbf9561e2c36) )    /* background color map */
ROM_END

ROM_START( darkplnt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "drkplt2c.dat", 0x0000, 0x1000, CRC(5a0ca559) SHA1(cf77bda30bb08633d274c22734a8f186520ddf7d) )
	ROM_LOAD( "drkplt2e.dat", 0x1000, 0x1000, CRC(52e2117d) SHA1(774e8ee84316135313c4b66c51d21c1661f345b3) )
	ROM_LOAD( "drkplt2g.dat", 0x2000, 0x1000, CRC(4093219c) SHA1(cc92ec4ec8661993173daedfa521252a6e34073d) )
	ROM_LOAD( "drkplt2j.dat", 0x3000, 0x1000, CRC(b974c78d) SHA1(af4c1b21ca8651bfc343a24a106db4c82f0052e7) )
	ROM_LOAD( "drkplt2k.dat", 0x4000, 0x1000, CRC(71a37385) SHA1(a6a358b2f7daa2eaa5bdf96aa903619376473b20) )
	ROM_LOAD( "drkplt2l.dat", 0x5000, 0x1000, CRC(5ad25154) SHA1(20b1347ed5fca108303132d28ae73fc4d71b4e5a) )
	ROM_LOAD( "drkplt2m.dat", 0x6000, 0x1000, CRC(8d2f0122) SHA1(59280abca22fd3d7af53aebf56a9e37eeb7cb11c) )
	ROM_LOAD( "drkplt2p.dat", 0x7000, 0x1000, CRC(2d66253b) SHA1(445c00a74551732fd02df2d5879688f9984931b5) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "5c.snd",       0x0000, 0x1000, CRC(672b9454) SHA1(2b2ec06d62ff5f19a1c802152c798940631fb1d6) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "drkplt5f.dat", 0x0000, 0x0800, CRC(2af0ee66) SHA1(a4f32c256c393a18afde5e3f3c67d7df1223999a) )
	ROM_LOAD( "drkplt5h.dat", 0x0800, 0x0800, CRC(66ef3225) SHA1(87f36a853c4a8ae40d1e3f0482b6e31c86f834f3) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6e.cpu",       0x0000, 0x0020, CRC(86b6e124) SHA1(76cc422aeb53a4970f91bd5eb00a3e24a76c09d1) )
ROM_END

ROM_START( tazmani2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2ck.cpu",      0x0000, 0x1000, CRC(bf0492bf) SHA1(e665be8b36eb00388daca1bc3a4e4b93b943e2bd) )
	ROM_LOAD( "2ek.cpu",      0x1000, 0x1000, CRC(6636c4d0) SHA1(a0470950111fd9ef3ec71ee9f4b337276db35887) )
	ROM_LOAD( "2fk.cpu",      0x2000, 0x1000, CRC(ce59a57b) SHA1(1ee9a1535f9e06b08db3133a29cc700d1b487e82) )
	ROM_LOAD( "2hk.cpu",      0x3000, 0x1000, CRC(8bda3380) SHA1(eb31dd180da5cb8f8dcdaa777a27ed5514cf38cf) )
	ROM_LOAD( "2jk.cpu",      0x4000, 0x1000, CRC(a4095e35) SHA1(5ccab4cb9c44870852b22511daeda10d54b60822) )
	ROM_LOAD( "2kk.cpu",      0x5000, 0x1000, CRC(f308ca36) SHA1(af360c4ba88b3a3365a2d81575886fcec25f19bf) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "rom0.snd",     0x0000, 0x0800, CRC(b8d741f1) SHA1(a1bb8a1e0d6b34111f05c539c8e92fffacf5aa5c) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5f.cpu",       0x0000, 0x0800, CRC(2c5b612b) SHA1(32e3a41a9a4a8b1285b6a195213ff0d98012360a) )
	ROM_LOAD( "5h.cpu",       0x0800, 0x0800, CRC(3f5ff3ac) SHA1(bc70eef54a45b52c14e35464e5f06b5eec554eb6) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "colr6f.cpu",   0x0000, 0x0020, CRC(fce333c7) SHA1(f63a214dc47c5e7c80db000b0b6a261ca8da6629) )
ROM_END

/*
    Rescue

    CPU/Video Board: A969 (Has various wire mods)
    Sound Board:     ?
*/

ROM_START( rescue )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rb15acpu.bin", 0x0000, 0x1000, CRC(d7e654ba) SHA1(6bcf7a6d86bc2de9a304f0aa9542fdcb58ca73dd) )
	ROM_LOAD( "rb15bcpu.bin", 0x1000, 0x1000, CRC(a93ea158) SHA1(c5550881d2a44f82eeb56d90487137d959c8c6ae) )
	ROM_LOAD( "rb15ccpu.bin", 0x2000, 0x1000, CRC(058cd3d0) SHA1(3f8962be982952adab23c1884cd3a9be5ddd4108) )
	ROM_LOAD( "rb15dcpu.bin", 0x3000, 0x1000, CRC(d6505742) SHA1(804df84ea507bf41e9376a7ce63a278c3701f0cd) )
	ROM_LOAD( "rb15ecpu.bin", 0x4000, 0x1000, CRC(604df3a4) SHA1(15790fa442538632f232280c096ac788d9bf8117) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "rb15csnd.bin", 0x0000, 0x0800, CRC(8b24bf17) SHA1(cc49fef3c629c12f1a7eb9886fdc2df4b08f4b37) )
	ROM_LOAD( "rb15dsnd.bin", 0x0800, 0x0800, CRC(d96e4fb3) SHA1(8bb023c7c668f93d2333d648fc3cefdbd66f92db) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "rb15fcpu.bin", 0x0000, 0x0800, CRC(4489d20c) SHA1(c7a2afbd2e5645a1a25dec6147d61a38ba12380f) )
	ROM_LOAD( "rb15hcpu.bin", 0x0800, 0x0800, CRC(5512c547) SHA1(e0f1c994daaa8933230cbc4bb88d459a698e0d8e) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "rescue.clr",   0x0000, 0x0020, CRC(40c6bcbd) SHA1(cb0c058eadc37eba4b1a99be095da81a14099d8d) )
ROM_END

ROM_START( rescueb )  /* VIDEL GAMES @ ($079A-$07A4) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p1.2c",        0x0000, 0x1000, CRC(d3661258) SHA1(41782097f6acb1d76067e7d727837a5a5297e43f) )
	ROM_LOAD( "p2.2e",        0x1000, 0x1000, CRC(47464506) SHA1(53a2a3ff307ad49ead8a7fcf897c8650f56eeb19) )
	ROM_LOAD( "p3.2f",        0x2000, 0x1000, CRC(7dc03ec3) SHA1(9aa5fda48a40c977d15e51bbcfe9b155705a6941) )
	ROM_LOAD( "p4.2h",        0x3000, 0x1000, CRC(a11eaf78) SHA1(dfbc6ea5894493fd74fe3f75cd1793bd31673319) )
	ROM_LOAD( "rb15ecpu.bin", 0x4000, 0x1000, CRC(604df3a4) SHA1(15790fa442538632f232280c096ac788d9bf8117) ) // p5.2j

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "rb15csnd.bin", 0x0000, 0x0800, CRC(8b24bf17) SHA1(cc49fef3c629c12f1a7eb9886fdc2df4b08f4b37) ) // s1.5c
	ROM_LOAD( "rb15dsnd.bin", 0x0800, 0x0800, CRC(d96e4fb3) SHA1(8bb023c7c668f93d2333d648fc3cefdbd66f92db) ) // s1.5d

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "rb15fcpu.bin", 0x0000, 0x0800, CRC(4489d20c) SHA1(c7a2afbd2e5645a1a25dec6147d61a38ba12380f) ) // c1.5f
	ROM_LOAD( "rb15hcpu.bin", 0x0800, 0x0800, CRC(5512c547) SHA1(e0f1c994daaa8933230cbc4bb88d459a698e0d8e) ) // c2.5h

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "rescue.clr",   0x0000, 0x0020, CRC(40c6bcbd) SHA1(cb0c058eadc37eba4b1a99be095da81a14099d8d) )
ROM_END

ROM_START( aponow )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "aponow01.rom", 0x0000, 0x1000, CRC(33011579) SHA1(62bd821f6e0968a5ed9cbab26a8bff31e707f07a) )
	ROM_LOAD( "aponow02.rom", 0x1000, 0x1000, CRC(d477573e) SHA1(e5deb62f2763ff81e9ca24df67b0539cca8ba714) )
	ROM_LOAD( "aponow03.rom", 0x2000, 0x1000, CRC(46c41898) SHA1(c501f1d75762cfeda68e178e70cdf7eb423005fe) )
	ROM_LOAD( "rb15dcpu.bin", 0x3000, 0x1000, CRC(d6505742) SHA1(804df84ea507bf41e9376a7ce63a278c3701f0cd) ) // aponow04.rom
	ROM_LOAD( "rb15ecpu.bin", 0x4000, 0x1000, CRC(604df3a4) SHA1(15790fa442538632f232280c096ac788d9bf8117) ) // aponow05.rom

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "rb15csnd.bin", 0x0000, 0x0800, CRC(8b24bf17) SHA1(cc49fef3c629c12f1a7eb9886fdc2df4b08f4b37) )
	ROM_LOAD( "rb15dsnd.bin", 0x0800, 0x0800, CRC(d96e4fb3) SHA1(8bb023c7c668f93d2333d648fc3cefdbd66f92db) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "rb15fcpu.bin", 0x0000, 0x0800, CRC(4489d20c) SHA1(c7a2afbd2e5645a1a25dec6147d61a38ba12380f) )
	ROM_LOAD( "rb15hcpu.bin", 0x0800, 0x0800, CRC(5512c547) SHA1(e0f1c994daaa8933230cbc4bb88d459a698e0d8e) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "rescue.clr",   0x0000, 0x0020, CRC(40c6bcbd) SHA1(cb0c058eadc37eba4b1a99be095da81a14099d8d) )
ROM_END

ROM_START( minefld )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ma22c",        0x0000, 0x1000, CRC(1367a035) SHA1(25e415743a4cbe573a00b81155f6292592e32cdd) )
	ROM_LOAD( "ma22e",        0x1000, 0x1000, CRC(68946d21) SHA1(62f4501c6d2e8615b1f176e6cea9a75c912de23e) )
	ROM_LOAD( "ma22f",        0x2000, 0x1000, CRC(7663aee5) SHA1(eacb2283e0555176629a96e9b607f0477927d84c) )
	ROM_LOAD( "ma22h",        0x3000, 0x1000, CRC(9787475d) SHA1(161c6f35cc2ce2a88bcc2e9626ccfe1f5e099f28) )
	ROM_LOAD( "ma22j",        0x4000, 0x1000, CRC(2ceceb54) SHA1(c7385ce876582515f6b9a529e0101032056e14e7) )
	ROM_LOAD( "ma22l",        0x5000, 0x1000, CRC(85138fc9) SHA1(b3f2e483150583d2b4daf6a8a308a7f47a0e4f1b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ma15c",        0x0000, 0x0800, CRC(8bef736b) SHA1(c5c7ce9c40e6fe60a4914b6bbd646ba6853c9043) )
	ROM_LOAD( "ma15d",        0x0800, 0x0800, CRC(f67b3f97) SHA1(bb15f95eab6594508b5c02af60ed9fff3abd23ee) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "ma15f",        0x0000, 0x0800, CRC(9f703006) SHA1(95c1368b6a3e0cea6fa9406155d3b8d1eb9477f4) )
	ROM_LOAD( "ma15h",        0x0800, 0x0800, CRC(ed0dccb1) SHA1(f1554aa5a4d3a70936645e784a94305f642670df) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "minefld.clr",  0x0000, 0x0020, CRC(1877368e) SHA1(5850e03debe572f72abd91c756c0f8613018a962) )
ROM_END

ROM_START( hustler )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hustler.1",    0x0000, 0x1000, CRC(94479a3e) SHA1(ea3a1a3f6bee3d35a6a0fb0ba689a25f6b919e5d) )
	ROM_LOAD( "hustler.2",    0x1000, 0x1000, CRC(3cc67bcc) SHA1(bc36eaab68f48c837d0d3d3bc1e7de961fa24a21) )
	ROM_LOAD( "hustler.3",    0x2000, 0x1000, CRC(9422226a) SHA1(445daa94f7236f0776f4692de6cb213f67684f70) )
	/* 3000-3fff space for diagnostics ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "hustler.6",    0x0000, 0x0800, CRC(7a946544) SHA1(7ee2ad3fdf996f08534fb87fc02b619c168f420c) )
	ROM_LOAD( "hustler.7",    0x0800, 0x0800, CRC(3db57351) SHA1(e5075a7130a80d2bf24f0556c2589dff0625ee60) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "hustler.5f",   0x0000, 0x0800, CRC(0bdfad0e) SHA1(8e6f1737604f3801c03fa2e9a5e6a2778b54bae8) )
	ROM_LOAD( "hustler.5h",   0x0800, 0x0800, CRC(8e062177) SHA1(7e52a1669804b6c2f694cfc64b04abc8246bb0c2) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "hustler.clr",  0x0000, 0x0020, CRC(aa1f7f5e) SHA1(311dd17aa11490a1173c76223e4ccccf8ea29850) )
ROM_END

ROM_START( hustlerd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vh1.ic5",    0x0000, 0x1000, CRC(7cb6a940) SHA1(8b51072563f7a63aab5d5ee4e835dc7275a2b98a) )
	ROM_LOAD( "vh2.ic6",    0x1000, 0x1000, CRC(4ca45239) SHA1(729ec16c0a192f957ba454d4acbe873a71030a22) )
	ROM_LOAD( "vh3.ic7",    0x2000, 0x1000, CRC(4c752453) SHA1(467ffd6e3ec13a27fc2979883678a1e7531d98ac) )
	/* 3000-3fff space for diagnostics ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ho1.ic32",    0x0000, 0x0800, CRC(7a946544) SHA1(7ee2ad3fdf996f08534fb87fc02b619c168f420c) )
	ROM_LOAD( "ho2.ic33",    0x0800, 0x0800, CRC(3db57351) SHA1(e5075a7130a80d2bf24f0556c2589dff0625ee60) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "hc2",   0x0000, 0x0800, CRC(0bdfad0e) SHA1(8e6f1737604f3801c03fa2e9a5e6a2778b54bae8) )
	ROM_LOAD( "hc1",   0x0800, 0x0800, CRC(8e062177) SHA1(7e52a1669804b6c2f694cfc64b04abc8246bb0c2) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "hustler.clr",  0x0000, 0x0020, CRC(aa1f7f5e) SHA1(311dd17aa11490a1173c76223e4ccccf8ea29850) )
ROM_END

ROM_START( billiard )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a",            0x0000, 0x1000, CRC(b7eb50c0) SHA1(213d177d2b2af648a18d196b83e96d804947fd40) )
	ROM_LOAD( "b",            0x1000, 0x1000, CRC(988fe1c5) SHA1(0aaa13006e1832d7c25984b38c2e52fb6a5fdbbc) )
	ROM_LOAD( "c",            0x2000, 0x1000, CRC(7b8de793) SHA1(f54aa0bd558b156c2511a36518963df0e493dbb4) )
	/* 3000-3fff space for diagnostics ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "hustler.6",    0x0000, 0x0800, CRC(7a946544) SHA1(7ee2ad3fdf996f08534fb87fc02b619c168f420c) )
	ROM_LOAD( "hustler.7",    0x0800, 0x0800, CRC(3db57351) SHA1(e5075a7130a80d2bf24f0556c2589dff0625ee60) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "hustler.5f",   0x0000, 0x0800, CRC(0bdfad0e) SHA1(8e6f1737604f3801c03fa2e9a5e6a2778b54bae8) )
	ROM_LOAD( "hustler.5h",   0x0800, 0x0800, CRC(8e062177) SHA1(7e52a1669804b6c2f694cfc64b04abc8246bb0c2) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "hustler.clr",  0x0000, 0x0020, CRC(aa1f7f5e) SHA1(311dd17aa11490a1173c76223e4ccccf8ea29850) )
ROM_END

/* this is identical to billiard, but with a different memory map */
ROM_START( hustlerb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hustler.2c",   0x0000, 0x1000, CRC(3a1ac6a9) SHA1(f9a6137bab78ce64ec119f199b2bd5ed37e61099) )
	ROM_LOAD( "hustler.2f",   0x1000, 0x1000, CRC(dc6752ec) SHA1(b103021079646286156e4141fe34dd92ccfd34bd) )
	ROM_LOAD( "hustler.2j",   0x2000, 0x1000, CRC(27c1e0f8) SHA1(9713e84b3aa4ed4829de2f62059229564d2b9f72) )
	/* 3000-3fff space for diagnostics ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "hustler.11d",  0x0000, 0x0800, CRC(b559bfde) SHA1(f7733fbc5cabb441ba039b9d7202aaf0cebb9a85) )
	ROM_LOAD( "hustler.10d",  0x0800, 0x0800, CRC(6ef96cfb) SHA1(eba0bdc8bc1652ff2f62594371ded711dbfcce86) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "hustler.5f",   0x0000, 0x0800, CRC(0bdfad0e) SHA1(8e6f1737604f3801c03fa2e9a5e6a2778b54bae8) )
	ROM_LOAD( "hustler.5h",   0x0800, 0x0800, CRC(8e062177) SHA1(7e52a1669804b6c2f694cfc64b04abc8246bb0c2) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "hustler.clr",  0x0000, 0x0020, CRC(aa1f7f5e) SHA1(311dd17aa11490a1173c76223e4ccccf8ea29850) )
ROM_END

ROM_START( hustlerb2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b1.1",   0x0000, 0x1000, CRC(2ce5e7b9) SHA1(e47e89bd085b40d7dd38be51fd69f69b143daf1b) )
	ROM_LOAD( "b1.2",   0x1000, 0x1000, CRC(dc6752ec) SHA1(b103021079646286156e4141fe34dd92ccfd34bd) )
	ROM_LOAD( "b3.3",   0x2000, 0x1000, CRC(23092f67) SHA1(3c04544c636eab8998752e1f521e34b1345e39ff) )
	/* 3000-3fff space for diagnostics ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b6.bin",    0x0000, 0x0800, CRC(7a946544) SHA1(7ee2ad3fdf996f08534fb87fc02b619c168f420c) )
	ROM_LOAD( "b7.bin",    0x0800, 0x0800, CRC(3db57351) SHA1(e5075a7130a80d2bf24f0556c2589dff0625ee60) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "b4.r2",   0x0000, 0x0800, CRC(0bdfad0e) SHA1(8e6f1737604f3801c03fa2e9a5e6a2778b54bae8) )
	ROM_LOAD( "b5",      0x0800, 0x0800, CRC(8e062177) SHA1(7e52a1669804b6c2f694cfc64b04abc8246bb0c2) ) // broken rom, assumed to be the same

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "27s19.bin",  0x0000, 0x0020, CRC(aa1f7f5e) SHA1(311dd17aa11490a1173c76223e4ccccf8ea29850) )
ROM_END

ROM_START( hustlerb3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "billiard_ic4.a4", 0x0000, 0x2000, CRC(545a27fd) SHA1(8b064dd6a9a82248301e0f53dc1c4fd91e506025) )
	ROM_LOAD( "billiard_ic3.a3", 0x2000, 0x2000, CRC(bec503b1) SHA1(cdbe650b829cd4424141058467cd64cfffe1b1e1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "hustler.6",    0x0000, 0x0800, BAD_DUMP CRC(7a946544) SHA1(7ee2ad3fdf996f08534fb87fc02b619c168f420c) ) // not found on board - taken from parent
	ROM_LOAD( "hustler.7",    0x0800, 0x0800, BAD_DUMP CRC(3db57351) SHA1(e5075a7130a80d2bf24f0556c2589dff0625ee60) ) // not found on board - taken from parent

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "billiard_ic11.d1",   0x0000, 0x0800, CRC(0bdfad0e) SHA1(8e6f1737604f3801c03fa2e9a5e6a2778b54bae8) )
	ROM_LOAD( "billiard_ic12.d2",   0x0800, 0x0800, CRC(8e062177) SHA1(7e52a1669804b6c2f694cfc64b04abc8246bb0c2) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "ic10.c3",  0x0000, 0x0020, CRC(aa1f7f5e) SHA1(311dd17aa11490a1173c76223e4ccccf8ea29850) )

	ROM_REGION( 0x0020, "user1", 0 ) /* decode PROMs */
	ROM_LOAD( "ic7.b3", 0x0000, 0x0020, CRC(4ac17114) SHA1(1fa34a556fe445a6bdabfe75b4b679cab6553c8b) )
ROM_END

ROM_START( mimonkey )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mm1.2e",       0x0000, 0x1000, CRC(9019f1b1) SHA1(0c45f64e39b9a182f6162ab520ced6ef0686466c) )
	ROM_LOAD( "mm2.2e",       0x1000, 0x1000, CRC(043e97d6) SHA1(924c0165dfcf01182696b0d259718ac625573d9a) )
	ROM_LOAD( "mm3.2f",       0x2000, 0x1000, CRC(1052726a) SHA1(2fdd3064f02babd2d496a38c7aee094cb3666f24) )
	ROM_LOAD( "mm4.2h",       0x3000, 0x1000, CRC(7b3f35ff) SHA1(b52c46c3f166346d3b25cd2ab09781afc703de08) )
	ROM_LOAD( "mm5.2j",       0xc000, 0x1000, CRC(b4e5c32d) SHA1(18e53519e8f4e813109cfaf45f2f66444e6fa1a2) )
	ROM_LOAD( "mm6.2l",       0xd000, 0x1000, CRC(409036c4) SHA1(a9640da91156504bfc8fedcda30f81169b28a0c9) )
	ROM_LOAD( "mm7.2m",       0xe000, 0x1000, CRC(119c08fa) SHA1(6e19ab874b735fe7339bcf651111664263ea4ef9) )
	ROM_LOAD( "mm8.2p",       0xf000, 0x1000, CRC(f7989f04) SHA1(d6e301414a807f5e9feed92ce53ab73d6bd46c45) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "mm13.11d",     0x0000, 0x1000, CRC(2d14c527) SHA1(062414ce0415b6c471149319ecae22f465df3a4f) )
	ROM_LOAD( "mm14.10d",     0x1000, 0x1000, CRC(35ed0f96) SHA1(5aaacae5c2acf97540b72491f71ea823f5eeae1a) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "mm12.5h",      0x0000, 0x1000, CRC(f73a8412) SHA1(9baf4336cceb9b039372b0a1c733910aeab5ec6d) )
	ROM_LOAD( "mm10.5h",      0x1000, 0x1000, CRC(3828c9db) SHA1(eaf9e81c803ad2be6c2db3104f07f80788378286) )
	ROM_LOAD( "mm11.5f",      0x2000, 0x1000, CRC(9e0e9289) SHA1(79d412dbceb364bc798feda658b15792feb63338) )
	ROM_LOAD( "mm9.5f",       0x3000, 0x1000, CRC(92085b0c) SHA1(a791703fa9f17e42450c871d902430fc3c6b10ef) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.6e",    0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )
ROM_END

ROM_START( mimonsco )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fra_1a",       0x0000, 0x1000, CRC(8e7a7379) SHA1(06b945a5d237384bfd1b4c9a7449f5a1701a352c) )
	ROM_LOAD( "fra_1b",       0x1000, 0x1000, CRC(ab08cbfe) SHA1(edccefefc0ed476d94acccf7f92115c5d6945679) )
	ROM_LOAD( "fra_2a",       0x2000, 0x1000, CRC(2d4da24d) SHA1(d922713084c9981169f35b41c71c8afa3d7f947d) )
	ROM_LOAD( "fra_2b",       0x3000, 0x1000, CRC(8d88fc7c) SHA1(1ba2d6d448a2c993f398f4457efb1e3535de9ea2) )
	ROM_LOAD( "fra_3a",       0xc000, 0x1000, CRC(b4e5c32d) SHA1(18e53519e8f4e813109cfaf45f2f66444e6fa1a2) )
	ROM_LOAD( "fra_3b",       0xd000, 0x1000, CRC(409036c4) SHA1(a9640da91156504bfc8fedcda30f81169b28a0c9) )
	ROM_LOAD( "fra_4a",       0xe000, 0x1000, CRC(119c08fa) SHA1(6e19ab874b735fe7339bcf651111664263ea4ef9) )
	ROM_LOAD( "fra_4b",       0xf000, 0x1000, CRC(d700fd03) SHA1(3e804a42ecc166d8723f0b0a4906212addbbad7b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "mmsound1",     0x0000, 0x1000, CRC(2d14c527) SHA1(062414ce0415b6c471149319ecae22f465df3a4f) )
	ROM_LOAD( "mmsnd2a",      0x1000, 0x1000, CRC(35ed0f96) SHA1(5aaacae5c2acf97540b72491f71ea823f5eeae1a) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "mmgfx1",       0x0000, 0x2000, CRC(4af47337) SHA1(225f7bcfbb61e3a163ecaed675d4c81b3609562f) )
	ROM_LOAD( "mmgfx2",       0x2000, 0x2000, CRC(def47da8) SHA1(8e62e5dc5c810efaa204d0fcb3d02bc84f61ba35) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.6e",    0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )
ROM_END


GAME( 1981, stratgyx,  0,        stratgyx,  stratgyx,  scramble_state,  stratgyx,     ROT0,   "Konami", "Strategy X", GAME_SUPPORTS_SAVE )
GAME( 1981, stratgys,  stratgyx, stratgyx,  stratgyx,  scramble_state,  stratgyx,     ROT0,   "Konami (Stern Electronics license)", "Strategy X (Stern Electronics)", GAME_SUPPORTS_SAVE )
GAME( 1982, strongx,   stratgyx, stratgyx,  stratgyx,  scramble_state,  stratgyx,     ROT0,   "bootleg", "Strong X", GAME_SUPPORTS_SAVE )
GAME( 1982, darkplnt,  0,        darkplnt,  darkplnt,  scramble_state,  darkplnt,     ROT180, "Stern Electronics", "Dark Planet", GAME_SUPPORTS_SAVE )
GAME( 1982, tazmani2,  tazmania, type2,     tazmania,  scramble_state,  tazmani2,     ROT90,  "Stern Electronics", "Tazz-Mania (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1982, rescue,    0,        rescue,    rescue,    scramble_state,  rescue,       ROT90,  "Stern Electronics", "Rescue", GAME_SUPPORTS_SAVE )
GAME( 1982, rescueb,   rescue,   rescue,    rescue,    scramble_state,  rescue,       ROT90,  "bootleg (Videl Games)", "Rescue (bootleg)", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
GAME( 1982, aponow,    rescue,   rescue,    rescue,    scramble_state,  rescue,       ROT90,  "bootleg", "Apocaljpse Now", GAME_SUPPORTS_SAVE )
GAME( 1983, minefld,   0,        minefld,   minefld,   scramble_state,  minefld,      ROT90,  "Stern Electronics", "Minefield", GAME_SUPPORTS_SAVE )
GAME( 1981, hustler,   0,        hustler,   hustler,   scramble_state,  hustler,      ROT90,  "Konami", "Video Hustler", GAME_SUPPORTS_SAVE )
GAME( 1981, hustlerd,  hustler,  hustler,   hustler,   scramble_state,  hustlerd,     ROT90,  "Konami (Dynamo Games license)", "Video Hustler (Dynamo Games)", GAME_SUPPORTS_SAVE )
GAME( 1981, billiard,  hustler,  hustler,   hustler,   scramble_state,  billiard,     ROT90,  "bootleg", "The Billiards (Video Hustler bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1981, hustlerb,  hustler,  hustlerb,  hustler,   driver_device,   0,            ROT90,  "bootleg (Digimatic)", "Video Hustler (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1981, hustlerb2, hustler,  hustler,   hustler,   scramble_state,  hustlerd,     ROT90,  "bootleg", "Fatsy Gambler (Video Hustler bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1981, hustlerb3, hustler,  hustlerb3, hustlerb3, scramble_state,  hustlerd,     ROT90,  "bootleg (Videotron)", "Video Pool (Video Hustler bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1982, mimonkey,  0,        mimonkey,  mimonkey,  scramble_state,  mimonkey,     ROT90,  "Universal Video Games", "Mighty Monkey", GAME_SUPPORTS_SAVE )
GAME( 1982, mimonsco,  mimonkey, mimonkey,  mimonsco,  scramble_state,  mimonsco,     ROT90,  "bootleg", "Mighty Monkey (bootleg on Super Cobra hardware)", GAME_SUPPORTS_SAVE )
