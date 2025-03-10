/***************************************************************************

        VTech PreComputer 2000

        04/12/2009 Skeleton driver.

        TODO:
        - fix the keys label
        - dump the chargen
        - verify the cartridge interface (I don't have dump for test it)
        - the HD44780 busy line is never checked, it's this correct??

****************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/hd44780.h"
#include "sound/beep.h"
#include "imagedev/cartslot.h"
#include "rendlay.h"


class pc2000_state : public driver_device
{
public:
	pc2000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_lcdc(*this, "hd44780"),
			m_beep(*this, BEEPER_TAG)
		{ }

	required_device<hd44780_device> m_lcdc;
	required_device<beep_device> m_beep;

	UINT8 m_mux_data;
	UINT8 m_beep_state;

	virtual void machine_start();
	virtual void machine_reset();
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER( key_matrix_r );
	DECLARE_WRITE8_MEMBER( key_matrix_w );
	DECLARE_WRITE8_MEMBER( rombank1_w );
	DECLARE_WRITE8_MEMBER( rombank2_w );
	DECLARE_READ8_MEMBER( beep_r );
	DECLARE_WRITE8_MEMBER( beep_w );
	virtual void palette_init();
};


/* TODO: put a breakpoint at 1625 and test the inputs, writes at dce4 are the scancode values */
READ8_MEMBER( pc2000_state::key_matrix_r )
{
	static const char *const bitnames[2][8] =
	{
		{"IN0", "IN1", "IN2", "IN3", "IN4", "IN5", "IN6", "IN7"},
		{"IN8", "IN9", "INA", "INB", "INC", "IND", "INE", "INF"}
	};

	UINT8 data = 0xff;

	for (int line=0; line<8; line++)
		if (m_mux_data & (1<<line))
			data &= ioport(bitnames[offset][line])->read();

	return data;
}

WRITE8_MEMBER( pc2000_state::key_matrix_w )
{
	m_mux_data = data;
}

WRITE8_MEMBER( pc2000_state::rombank1_w )
{
	membank("bank1")->set_entry(data & 0x0f);
}

WRITE8_MEMBER( pc2000_state::rombank2_w )
{
	if (data == 0x80)
	{
		membank("bank2")->set_entry(data & 0x10);   //cartridge
	}
	else
	{
		membank("bank2")->set_entry(data & 0x0f);
	}
}

READ8_MEMBER( pc2000_state::beep_r )
{
	return m_beep_state;
}

WRITE8_MEMBER( pc2000_state::beep_w )
{
	beep_set_state(m_beep, BIT(data, 3));
	m_beep_state = data;
}

static ADDRESS_MAP_START(pc2000_mem, AS_PROGRAM, 8, pc2000_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_REGION("bios", 0x00000)
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank2")    //0x8000 - 0xbfff tests a cartridge, header is 0x55 0xaa 0x59 0x45, if it succeeds a jump at 0x8004 occurs
	AM_RANGE(0xc000, 0xdfff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pc2000_io , AS_IO, 8, pc2000_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01, 0x01) AM_WRITE(rombank1_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(rombank2_w)
	AM_RANGE(0x0a, 0x0a) AM_DEVREADWRITE("hd44780", hd44780_device, control_read, control_write)
	AM_RANGE(0x0b, 0x0b) AM_DEVREADWRITE("hd44780", hd44780_device, data_read, data_write)
	AM_RANGE(0x10, 0x11) AM_READWRITE(key_matrix_r, key_matrix_w)
	AM_RANGE(0x12, 0x12) AM_READWRITE(beep_r, beep_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( pc2000 )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "IN0" ) //0x83
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
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

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Caps Lock")  PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1")          PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q")          PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A")          PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z")          PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\")         PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space")      PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2")  PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3")  PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E")  PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S")  PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D")  PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X")  PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C")  PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W")  PORT_CODE(KEYCODE_W) PORT_CHAR('W')

	PORT_START("IN3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4")  PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5")  PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T")  PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F")  PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G")  PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V")  PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B")  PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R")  PORT_CODE(KEYCODE_R) PORT_CHAR('R')

	PORT_START("IN4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6")  PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7")  PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U")  PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H")  PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J")  PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N")  PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M")  PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y")  PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')

	PORT_START("IN5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8")  PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9")  PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O")  PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K")  PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L")  PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",")  PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".")  PORT_CODE(KEYCODE_STOP) PORT_CHAR('.')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I")  PORT_CODE(KEYCODE_I) PORT_CHAR('I')

	PORT_START("IN6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0")  PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("HOME")   PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("=")  PORT_CODE(KEYCODE_EQUALS)   PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";")  PORT_CODE(KEYCODE_COLON) PORT_CHAR(';')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\'") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/")  PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P")  PORT_CODE(KEYCODE_P) PORT_CHAR('P')

	PORT_START("IN7")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LEFT")   PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RIGHT")  PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER")  PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEL")    PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ESC")    PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Scramble")           PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Guess Rork")         PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Missing Letter")     PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Letter Hunt")        PORT_CODE(KEYCODE_F4)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Letter Zapper")      PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Letter Switch")      PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Sentence Jumble")    PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Grammar Challenge")  PORT_CODE(KEYCODE_F8)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("INA")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Plurals")            PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Past Tense")         PORT_CODE(KEYCODE_F10)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Synonyms")           PORT_CODE(KEYCODE_F11)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Antonyms")           PORT_CODE(KEYCODE_F12)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("INB")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Basic Math")         PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Advanced Math")      PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Math Riddles")       PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Number Challenge")   PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("INC")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ecology")            PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("History")            PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Geography")          PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Super Power")        PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IND")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Typing Game")        PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Alpha Jumble")       PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Word Flash")         PORT_CODE(KEYCODE_PGUP)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Sign Search")        PORT_CODE(KEYCODE_PGDN)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("INE")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PC2000 Basic")       PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Spell Checker")      PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Calculator")         PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Load Cartridge")     PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("INF")
	PORT_DIPNAME( 0x01, 0x01, "INF" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(DEF_STR( Level_Select )) PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num Players")        PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


void pc2000_state::machine_start()
{
	UINT8 *bios = memregion("bios")->base();
	UINT8 *cart = memregion("cart")->base();

	membank("bank1")->configure_entries(0, 0x10, bios, 0x4000);
	membank("bank2")->configure_entries(0, 0x10, bios, 0x4000);
	membank("bank2")->configure_entry(0x10, cart);
}

void pc2000_state::machine_reset()
{
	//set the initial bank
	membank("bank1")->set_entry(0);
	membank("bank2")->set_entry(0);
}

void pc2000_state::palette_init()
{
	palette_set_color(machine(), 0, MAKE_RGB(138, 146, 148));
	palette_set_color(machine(), 1, MAKE_RGB(92, 83, 88));
}

static const gfx_layout hd44780_charlayout =
{
	5, 8,                   /* 5 x 8 characters */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	{ 3, 4, 5, 6, 7},
	{ 0, 8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8},
	8*8                     /* 8 bytes */
};

static GFXDECODE_START( pc2000 )
	GFXDECODE_ENTRY( "hd44780", 0x0000, hd44780_charlayout, 0, 1 )
GFXDECODE_END

static HD44780_INTERFACE( pc2000_display )
{
	2,                  // number of lines
	20,                 // chars for line
	NULL                // pixel update callback
};

static MACHINE_CONFIG_START( pc2000, pc2000_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz) /* probably not accurate */
	MCFG_CPU_PROGRAM_MAP(pc2000_mem)
	MCFG_CPU_IO_MAP(pc2000_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", pc2000_state, irq0_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DEVICE("hd44780", hd44780_device, screen_update)
	MCFG_SCREEN_SIZE(120, 18) //2x20 chars
	MCFG_SCREEN_VISIBLE_AREA(0, 120-1, 0, 18-1)

	MCFG_PALETTE_LENGTH(2)
	MCFG_GFXDECODE(pc2000)
	MCFG_DEFAULT_LAYOUT(layout_lcd)

	MCFG_HD44780_ADD("hd44780", pc2000_display)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO( "mono" )
	MCFG_SOUND_ADD( BEEPER_TAG, BEEP, 0 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "mono", 1.00 )

	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("bin")
	MCFG_CARTSLOT_NOT_MANDATORY
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( pc2000 )
	ROM_REGION( 0x40000, "bios", ROMREGION_ERASEFF )
	ROM_LOAD( "lh532hee_9344_d.u4", 0x000000, 0x040000, CRC(0b03bf33) SHA1(cb344b94b14975c685041d3e669f386e8a21909f))

	ROM_REGION( 0x0860, "hd44780", ROMREGION_ERASE )
	ROM_LOAD( "44780a00.bin",    0x0000, 0x0860,  BAD_DUMP CRC(3a89024c) SHA1(5a87b68422a916d1b37b5be1f7ad0b3fb3af5a8d))

	ROM_REGION( 0x4000, "cart", ROMREGION_ERASEFF )
	ROM_CART_LOAD( "cart", 0, 0x4000, 0 )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1993, pc2000,  0,       0,    pc2000,     pc2000, driver_device,   0,  "Video Technology",   "PreComputer 2000",      GAME_NOT_WORKING)
