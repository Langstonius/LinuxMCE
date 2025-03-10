/***************************************************************************

    Explorer 85

    12/05/2009 Skeleton driver.

    Setting Up
    ==========
    The terminal must be set for
    - Baud: 9600
    - Format: 7E1
    If it isn't, adjust the settings, then restart the system.

    Once started, press Space. The system will start up.
    All input must be in upper case.

****************************************************************************/

/*

    TODO:

    - dump of the hexadecimal keyboard monitor ROM
    - disable ROM mirror after boot
    - RAM expansions

*/


#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "imagedev/cassette.h"
#include "machine/i8155.h"
#include "machine/i8355.h"
#include "machine/terminal.h"
#include "sound/speaker.h"
#include "machine/ram.h"
#include "includes/exp85.h"

/* Memory Maps */

static ADDRESS_MAP_START( exp85_mem, AS_PROGRAM, 8, exp85_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xdfff) AM_ROM
	AM_RANGE(0xf000, 0xf7ff) AM_ROM
	AM_RANGE(0xf800, 0xf8ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( exp85_io, AS_IO, 8, exp85_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xf0, 0xf3) AM_DEVREADWRITE(I8355_TAG, i8355_device, io_r, io_w)
	AM_RANGE(0xf8, 0xfd) AM_DEVREADWRITE(I8155_TAG, i8155_device, io_r, io_w)
//  AM_RANGE(0xfe, 0xff) AM_DEVREADWRITE(I8279_TAG, i8279_r, i8279_w)
ADDRESS_MAP_END

/* Input Ports */

INPUT_CHANGED_MEMBER( exp85_state::trigger_reset )
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);
}

INPUT_CHANGED_MEMBER( exp85_state::trigger_rst75 )
{
	m_maincpu->set_input_line(I8085_RST75_LINE, newval ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START( exp85 )

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("R") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, exp85_state, trigger_reset, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("I") PORT_CODE(KEYCODE_F2) PORT_CHANGED_MEMBER(DEVICE_SELF, exp85_state, trigger_rst75, 0)
INPUT_PORTS_END

/* 8155 Interface */

static I8155_INTERFACE( i8155_intf )
{
	DEVCB_NULL, /* port A read */
	DEVCB_NULL, /* port A write */
	DEVCB_NULL, /* port B read */
	DEVCB_NULL, /* port B write */
	DEVCB_NULL, /* port C read */
	DEVCB_NULL, /* port C write */
	DEVCB_NULL  /* timer output */
};

/* 8355 Interface */

READ8_MEMBER( exp85_state::i8355_a_r )
{
	/*

	    bit     description

	    PA0     tape control
	    PA1     jumper S17 (open=+5V closed=GND)
	    PA2     J5:13
	    PA3
	    PA4     J2:22
	    PA5
	    PA6
	    PA7     speaker output

	*/

	return 0x02;
}

WRITE8_MEMBER( exp85_state::i8355_a_w )
{
	/*

	    bit     description

	    PA0     tape control
	    PA1     jumper S17 (open=+5V closed=GND)
	    PA2     J5:13
	    PA3
	    PA4     J2:22
	    PA5
	    PA6
	    PA7     speaker output

	*/

	/* tape control */
	m_tape_control = BIT(data, 0);

	/* speaker output */
	speaker_level_w(m_speaker, !BIT(data, 7));
}

static I8355_INTERFACE( i8355_intf )
{
	DEVCB_DRIVER_MEMBER(exp85_state, i8355_a_r),
	DEVCB_DRIVER_MEMBER(exp85_state, i8355_a_w),
	DEVCB_NULL,
	DEVCB_NULL
};

/* I8085A Interface */

READ_LINE_MEMBER( exp85_state::sid_r )
{
	int data = 1;

	if (m_tape_control)
	{
		data = (m_cassette->input() > +0.0);
	}
	else
	{
		data = m_terminal->tx_r();
	}

	return data;
}

WRITE_LINE_MEMBER( exp85_state::sod_w )
{
	if (m_tape_control)
	{
		m_cassette->output(state ? -1.0 : +1.0);
	}
	else
	{
		m_terminal->rx_w(state);
	}
}

static I8085_CONFIG( exp85_i8085_config )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(exp85_state, sid_r),
	DEVCB_DRIVER_LINE_MEMBER(exp85_state, sod_w)
};

/* Terminal Interface */

static const serial_terminal_interface terminal_intf =
{
	DEVCB_NULL
};

/* Machine Initialization */

void exp85_state::machine_start()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	/* setup memory banking */
	program.install_read_bank(0x0000, 0x07ff, "bank1");
	program.unmap_write(0x0000, 0x07ff);
	membank("bank1")->configure_entry(0, memregion(I8085A_TAG)->base() + 0xf000);
	membank("bank1")->configure_entry(1, memregion(I8085A_TAG)->base());
	membank("bank1")->set_entry(0);
}

/* Machine Driver */

static const cassette_interface exp85_cassette_interface =
{
	cassette_default_formats,
	NULL,
	(cassette_state)(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED),
	NULL,
	NULL
};

static MACHINE_CONFIG_START( exp85, exp85_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(I8085A_TAG, I8085A, XTAL_6_144MHz)
	MCFG_CPU_PROGRAM_MAP(exp85_mem)
	MCFG_CPU_IO_MAP(exp85_io)
	MCFG_CPU_CONFIG(exp85_i8085_config)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* devices */
	MCFG_I8155_ADD(I8155_TAG, XTAL_6_144MHz/2, i8155_intf)
	MCFG_I8355_ADD(I8355_TAG, XTAL_6_144MHz/2, i8355_intf)
	MCFG_CASSETTE_ADD(CASSETTE_TAG, exp85_cassette_interface)
	MCFG_SERIAL_TERMINAL_ADD(TERMINAL_TAG, terminal_intf,9600)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("256")
	MCFG_RAM_EXTRA_OPTIONS("4K")
MACHINE_CONFIG_END

/* ROMs */

ROM_START( exp85 )
	ROM_REGION( 0x10000, I8085A_TAG, 0 )
	ROM_DEFAULT_BIOS("eia")
	ROM_LOAD( "c000.bin", 0xc000, 0x0800, CRC(73ce4aad) SHA1(2c69cd0b6c4bdc92f4640bce18467e4e99255bab) )
	ROM_LOAD( "c800.bin", 0xc800, 0x0800, CRC(eb3fdedc) SHA1(af92d07f7cb7533841b16e1176401363176857e1) )
	ROM_LOAD( "d000.bin", 0xd000, 0x0800, CRC(c10c4a22) SHA1(30588ba0b27a775d85f8c581ad54400c8521225d) )
	ROM_LOAD( "d800.bin", 0xd800, 0x0800, CRC(dfa43ef4) SHA1(56a7e7a64928bdd1d5f0519023d1594cacef49b3) )
	ROM_SYSTEM_BIOS( 0, "eia", "EIA Terminal" )
	ROMX_LOAD( "ex 85.u105", 0xf000, 0x0800, CRC(1a99d0d9) SHA1(57b6d48e71257bc4ef2d3dddc9b30edf6c1db766), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "hex", "Hex Keyboard" )
	ROMX_LOAD( "1kbd.u105", 0xf000, 0x0800, NO_DUMP, ROM_BIOS(2) )

	ROM_REGION( 0x800, I8355_TAG, ROMREGION_ERASE00 )

/*  ROM_DEFAULT_BIOS("terminal")
    ROM_SYSTEM_BIOS( 0, "terminal", "Terminal" )
    ROMX_LOAD( "eia.u105", 0xf000, 0x0800, CRC(1a99d0d9) SHA1(57b6d48e71257bc4ef2d3dddc9b30edf6c1db766), ROM_BIOS(1) )
    ROM_SYSTEM_BIOS( 1, "hexkbd", "Hex Keyboard" )
    ROMX_LOAD( "hex.u105", 0xf000, 0x0800, NO_DUMP, ROM_BIOS(2) )*/
ROM_END

/* System Drivers */
/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   INIT    COMPANY         FULLNAME        FLAGS */
COMP( 1979, exp85,  0,      0,      exp85,   exp85, driver_device,  0,    "Netronics",    "Explorer/85", GAME_NOT_WORKING )
