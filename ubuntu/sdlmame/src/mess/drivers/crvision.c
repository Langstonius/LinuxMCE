/*
    This 1980s computer was manufactured by Vtech of Hong Kong.
    known as: CreatiVision, Dick Smith Wizzard, Funvision, Rameses, VZ 2000 and possibly others.

    There is also a CreatiVision Mk 2, possibly also known as the Laser 500. This was a hardware variant,
    sort of "evolved" hardware made compatible for the scheduled "ColecoVision expansion module", which
    never actually shipped for CreatiVision, but for the Laser 2001 home computer (successor to
    CreatiVision).

    TODO:

    CreatiVision

    - fix Diagnostic A (video) sprites generator test
    - proper keyboard emulation, need keyboard schematics
    - memory expansion 16K, can be chained
    - centronics control/status port
    - non-working cartridges:
        * Diagnostic B (keyboard)
    - homebrew roms with graphics issues:
        * Christmas Demo 1.0
        * Titanic Frogger Demo 1.0
        * Titanic Frogger Demo 1.1

    Salora Manager

    - keyboard Ctrl+I/J/N don't print out BASIC commands
    - RAM mapping
    - cassette (figure out correct input level)
    - floppy (interface cartridge needed)

*/

/*

Salora Manager

PCB Layout
----------

Main board

35 0352 02
700391F

|-------------------------------------------------------------------------------|
|               |-----CN1-----|             |-----CN2-----|                     |
|                                                                   10.738MHz   --|
|                                   4116                                    CN3   |
|       ROM01                                               VDC                   |
|                                   4116                                        --|
|                                                                               |
|       ROM23                       4116    -                                   |
|                                           |                                   |
|                                   4116    |                                   |
|   LS04                                   CN6                                  --|
|                                   4116    |                  6821               |
|   LS32                                    |                                     |
|                                   4116    -                                     |
|   LS139                                           PSG                           |
|                                   4116                                          |
|   LS138                                                                         |
|                                   4116                                    CN4   |
|   LS244                                                                         |
|                                                                                 |
|   LS245                                                                         |
|                                                                                 |
|                   LS244                                                         |
|       6502                                                                    --|
|                   LS244                                                       |
|                                           |-------------CN5-------------|     |
|-------------------------------------------------------------------------------|

Notes:
All IC's shown. Prototype-ish board, with many jumper wires and extra capacitors.

ROM01   - Toshiba TMM2464P 8Kx8 one-time PROM, labeled "0.1"
ROM23   - Toshiba TMM2464P 8Kx8 one-time PROM, labeled "23"
6502    - Rockwell R6502AP 8-bit Microprocessor
6821    - Hitachi HD468B21P Peripheral Interface Adaptor
VDC     - Texas Instruments TMS9929A Video Display Controller (covered w/heatsink)
PSG     - Texas Instruments SN76489AN Programmable Sound Generator
4116    - Toshiba TMM416P-3 16Kx1 RAM (covered w/heatsink)
CN1     - sub board connector (17x2 pin header)
CN2     - RF board connector (17x1 pin header)
CN3     - printer connector (7x2 PCB edge male)
CN4     - expansion connector (30x2 PCB edge male)
CN5     - cartridge connector (18x2 PCB edge female)
CN6     - keyboard connector (16x1 pin header)


Sub board

700472
35 0473 03

|---------------------------------------|
|   17.73447MHz |-----CN1-----|         |
|                                       |
|   74S04                       4116    |
|                                       |
|   LS90                        4116    |
|                                       |
|   LS10                        4116    |
|                                       |
|   LS367                       4116    |
|                                       |
|   LS393                       4116    |
|                                       |
|   LS244                       4116    |
|                                       |
|   LS257                       4116    |
|                                       |
|   LS257                       4116    |
|                                       |
|                               LS139   |
|                                       |
|---------------------------------------|

Notes:
All IC's shown.

4116    - Toshiba TMM416P-3 16Kx1 RAM
CN1     - main board connector (17x2 pin header)

*/

#include "includes/crvision.h"

/***************************************************************************
    READ/WRITE HANDLERS
***************************************************************************/

/*-------------------------------------------------
    centronics_status_r - centronics status
-------------------------------------------------*/

READ8_MEMBER( crvision_state::centronics_status_r )
{
	UINT8 data = 0;

	data |= m_centronics->busy_r() << 7;

	return data;
}

/*-------------------------------------------------
    centronics_ctrl_w - centronics control
-------------------------------------------------*/

WRITE8_MEMBER( crvision_state::centronics_ctrl_w )
{
	m_centronics->strobe_w(BIT(data, 4));
}

/***************************************************************************
    MEMORY MAPS
***************************************************************************/

/*-------------------------------------------------
    ADDRESS_MAP( crvision_map )
-------------------------------------------------*/

static ADDRESS_MAP_START( crvision_map, AS_PROGRAM, 8, crvision_state )
	AM_RANGE(0x0000, 0x03ff) AM_MIRROR(0x0c00) AM_RAM
	AM_RANGE(0x1000, 0x1003) AM_MIRROR(0x0ffc) AM_DEVREADWRITE(PIA6821_TAG, pia6821_device, read, write)
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x0ffe) AM_DEVREAD(TMS9929_TAG, tms9928a_device, vram_read)
	AM_RANGE(0x2001, 0x2001) AM_MIRROR(0x0ffe) AM_DEVREAD(TMS9929_TAG, tms9928a_device, register_read)
	AM_RANGE(0x3000, 0x3000) AM_MIRROR(0x0ffe) AM_DEVWRITE(TMS9929_TAG, tms9928a_device, vram_write)
	AM_RANGE(0x3001, 0x3001) AM_MIRROR(0x0ffe) AM_DEVWRITE(TMS9929_TAG, tms9928a_device, register_write)
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK(BANK_ROM2)
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK(BANK_ROM1)
//  AM_RANGE(0xc000, 0xe7ff) AM_RAMBANK(3)
	AM_RANGE(0xe800, 0xe800) AM_DEVWRITE(CENTRONICS_TAG, centronics_device, write)
	AM_RANGE(0xe801, 0xe801) AM_READWRITE(centronics_status_r, centronics_ctrl_w)
//  AM_RANGE(0xe802, 0xf7ff) AM_RAMBANK(4)
	AM_RANGE(0xf800, 0xffff) AM_ROM AM_REGION(M6502_TAG, 0)
ADDRESS_MAP_END

/*-------------------------------------------------
    ADDRESS_MAP( lasr2001_map )
-------------------------------------------------*/

static ADDRESS_MAP_START( lasr2001_map, AS_PROGRAM, 8, laser2001_state )
	AM_RANGE(0x0000, 0x03ff) AM_MIRROR(0x0c00) AM_RAM
	AM_RANGE(0x1000, 0x1003) AM_MIRROR(0x0ffc) AM_DEVREADWRITE(PIA6821_TAG, pia6821_device, read, write)
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x0ffe) AM_DEVREAD(TMS9929_TAG, tms9928a_device, vram_read)
	AM_RANGE(0x2001, 0x2001) AM_MIRROR(0x0ffe) AM_DEVREAD(TMS9929_TAG, tms9928a_device, register_read)
	AM_RANGE(0x3000, 0x3000) AM_MIRROR(0x0ffe) AM_DEVWRITE(TMS9929_TAG, tms9928a_device, vram_write)
	AM_RANGE(0x3001, 0x3001) AM_MIRROR(0x0ffe) AM_DEVWRITE(TMS9929_TAG, tms9928a_device, register_write)
	AM_RANGE(0x4000, 0x7fff) AM_RAMBANK(BANK_ROM2)
	AM_RANGE(0x8000, 0xbfff) AM_RAMBANK(BANK_ROM1)
	AM_RANGE(0xc000, 0xffff) AM_ROM AM_REGION(M6502_TAG, 0)
ADDRESS_MAP_END

/***************************************************************************
    INPUT PORTS
***************************************************************************/

/*-------------------------------------------------
    INPUT_CHANGED_MEMBER( trigger_nmi )
-------------------------------------------------*/

INPUT_CHANGED_MEMBER( crvision_state::trigger_nmi )
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}

/*-------------------------------------------------
    INPUT_PORTS( crvision )
-------------------------------------------------*/

static INPUT_PORTS_START( crvision )
	// Player 1 Joystick

	PORT_START("PA0-0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA0-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0xfd, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA0-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0xf3, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA0-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0xf7, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA0-4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA0-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0xdf, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA0-6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA0-7")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Button 2 / CNT'L") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)

	// Player 1 Keyboard

	PORT_START("PA1-0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x81, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA1-1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x83, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA1-2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x87, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA1-3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x8f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA1-4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x9f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA1-5")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0xbf, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA1-6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA1-7")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Button 1 / SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)

	// Player 2 Joystick

	PORT_START("PA2-0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA2-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0xfd, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA2-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0xf3, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA2-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0xf7, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA2-4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA2-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0xdf, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA2-6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA2-7")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Button 2 / \xE2\x86\x92") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9) PORT_PLAYER(2)

	// Player 2 Keyboard

	PORT_START("PA3-0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RET'N") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x81, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA3-1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('@')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x83, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA3-2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x87, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA3-3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x8f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA3-4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT( 0x9f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA3-5")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0xbf, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA3-6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA3-7")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Button 1 / - =") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=') PORT_PLAYER(2)

	PORT_START("NMI")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F10) PORT_CHANGED_MEMBER(DEVICE_SELF, crvision_state, trigger_nmi, 0)
INPUT_PORTS_END

/*-------------------------------------------------
    INPUT_PORTS( manager )
-------------------------------------------------*/

static INPUT_PORTS_START( manager )
	PORT_START("ROW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('-')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')

	PORT_START("ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xC3\x84 \xC3\xA4") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(0x00C4) PORT_CHAR(0x00E4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')

	PORT_START("ROW2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xC3\x85 \xC3\xA5") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(0x00C5) PORT_CHAR(0x00E5)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("ROW3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xC3\x96 \xC3\xB6") PORT_CODE(KEYCODE_COLON) PORT_CHAR(0x00D6) PORT_CHAR(0x00F6)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')

	PORT_START("ROW4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')

	PORT_START("ROW5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('=')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')

	PORT_START("ROW6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')

	PORT_START("ROW7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')

	PORT_START("JOY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("JOY1")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START("JOY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("JOY3")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
INPUT_PORTS_END

/***************************************************************************
    DEVICE CONFIGURATION
***************************************************************************/

/*-------------------------------------------------
    TMS9928a_interface tms9918_intf
-------------------------------------------------*/

static TMS9928A_INTERFACE( vdp_intf )
{
	SCREEN_TAG,
	0x4000,
	DEVCB_CPU_INPUT_LINE(M6502_TAG, INPUT_LINE_IRQ0)
};

/*-------------------------------------------------
    pia6821_interface pia_intf
-------------------------------------------------*/

WRITE8_MEMBER( crvision_state::pia_pa_w )
{
	/*
	    Signal  Description

	    PA0     Keyboard raster player 1 output (joystick)
	    PA1     Keyboard raster player 1 output (hand keys)
	    PA2     Keyboard raster player 2 output (joystick)
	    PA3     Keyboard raster player 2 output (hand keys)
	    PA4     ?
	    PA5     ?
	    PA6     Cassette motor
	    PA7     Cassette data in/out
	*/

	/* keyboard raster */
	m_keylatch = ~data & 0x0f;

	/* cassette motor */
	m_cassette->change_state(BIT(data,6) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);

	/* cassette data output */
	m_cassette->output( BIT(data, 7) ? +1.0 : -1.0);
}

UINT8 crvision_state::read_keyboard(int pa)
{
	int i;
	UINT8 value;
	static const char *const keynames[4][8] =
			{
				{ "PA0-0", "PA0-1", "PA0-2", "PA0-3", "PA0-4", "PA0-5", "PA0-6", "PA0-7" },
				{ "PA1-0", "PA1-1", "PA1-2", "PA1-3", "PA1-4", "PA1-5", "PA1-6", "PA1-7" },
				{ "PA2-0", "PA2-1", "PA2-2", "PA2-3", "PA2-4", "PA2-5", "PA2-6", "PA2-7" },
				{ "PA3-0", "PA3-1", "PA3-2", "PA3-3", "PA3-4", "PA3-5", "PA3-6", "PA3-7" }
			};

	for (i = 0; i < 8; i++)
	{
		value = ioport(keynames[pa][i])->read();

		if (value != 0xff)
		{
			if (value == 0xff - (1 << i))
				return value;
			else
				return value - (1 << i);
		}
	}

	return 0xff;
}

READ8_MEMBER( crvision_state::pia_pa_r )
{
	/*
	    PA0     Keyboard raster player 1 output (joystick)
	    PA1     Keyboard raster player 1 output (hand keys)
	    PA2     Keyboard raster player 2 output (joystick)
	    PA3     Keyboard raster player 2 output (hand keys)
	    PA4     ?
	    PA5     ?
	    PA6     Cassette motor
	    PA7     Cassette data in/out
	*/

	UINT8 data = 0x7f;

	if ((m_cassette)->input() > -0.1469) data |= 0x80;

	return data;
}

READ8_MEMBER( crvision_state::pia_pb_r )
{
	/*
	    Signal  Description

	    PB0     Keyboard input
	    PB1     Keyboard input
	    PB2     Keyboard input
	    PB3     Keyboard input
	    PB4     Keyboard input
	    PB5     Keyboard input
	    PB6     Keyboard input
	    PB7     Keyboard input
	*/

	UINT8 data = 0xff;

	if (BIT(m_keylatch, 0)) data &= read_keyboard(0);
	if (BIT(m_keylatch, 1)) data &= read_keyboard(1);
	if (BIT(m_keylatch, 2)) data &= read_keyboard(2);
	if (BIT(m_keylatch, 3)) data &= read_keyboard(3);

	return data;
}

static const pia6821_interface pia_intf =
{
	DEVCB_DRIVER_MEMBER(crvision_state, pia_pa_r),      // input A
	DEVCB_DRIVER_MEMBER(crvision_state, pia_pb_r),      // input B
	DEVCB_LINE_VCC,                                     // input CA1 (+5V)
	DEVCB_DEVICE_LINE_MEMBER(SN76489_TAG, sn76496_base_device, ready_r),    // input CB1
	DEVCB_LINE_VCC,                                     // input CA2 (+5V)
	DEVCB_LINE_VCC,                                     // input CB2 (+5V)
	DEVCB_DRIVER_MEMBER(crvision_state, pia_pa_w),      // output A
	DEVCB_DEVICE_MEMBER(SN76489_TAG, sn76496_base_device, write),       // output B
	DEVCB_NULL,                                         // output CA2
	DEVCB_NULL,                                         // output CB2 (SN76489 pin CE_)
	DEVCB_NULL,                                         // irq A
	DEVCB_NULL                                          // irq B
};

/*-------------------------------------------------
    pia6821_interface lasr2001_pia_intf
-------------------------------------------------*/

READ8_MEMBER( laser2001_state::pia_pa_r )
{
	/*
	    Signal  Description

	    PA0     Keyboard column 0
	    PA1     Keyboard column 1
	    PA2     Keyboard column 2
	    PA3     Keyboard column 3
	    PA4     Keyboard column 4
	    PA5     Keyboard column 5
	    PA6     Keyboard column 6
	    PA7     Keyboard column 7
	*/

	UINT8 data = 0xff;

	if (!BIT(m_keylatch, 0)) data &= ioport("ROW0")->read();
	if (!BIT(m_keylatch, 1)) data &= ioport("ROW1")->read();
	if (!BIT(m_keylatch, 2)) data &= ioport("ROW2")->read();
	if (!BIT(m_keylatch, 3)) data &= ioport("ROW3")->read();
	if (!BIT(m_keylatch, 4)) data &= ioport("ROW4")->read();
	if (!BIT(m_keylatch, 5)) data &= ioport("ROW5")->read();
	if (!BIT(m_keylatch, 6)) data &= ioport("ROW6")->read();
	if (!BIT(m_keylatch, 7)) data &= ioport("ROW7")->read();

	return data;
}

WRITE8_MEMBER( laser2001_state::pia_pa_w )
{
	/*
	    PA0     Joystick player 1 output 0
	    PA1     Joystick player 1 output 1
	    PA2     Joystick player 2 output 0
	    PA3     Joystick player 2 output 1
	    PA4     ?
	    PA5     ?
	    PA6     ?
	    PA7     ?
	*/

	m_joylatch = data;
}

READ8_MEMBER( laser2001_state::pia_pb_r )
{
	UINT8 data = 0xff;

	if (!BIT(m_joylatch, 0)) data &= ioport("JOY0")->read();
	if (!BIT(m_joylatch, 1)) data &= ioport("JOY1")->read();
	if (!BIT(m_joylatch, 2)) data &= ioport("JOY2")->read();
	if (!BIT(m_joylatch, 3)) data &= ioport("JOY3")->read();

	return data;
}

WRITE8_MEMBER( laser2001_state::pia_pb_w )
{
	/*
	    Signal  Description

	    PB0     Keyboard row 0, PSG data 7, centronics data 0
	    PB1     Keyboard row 1, PSG data 6, centronics data 1
	    PB2     Keyboard row 2, PSG data 5, centronics data 2
	    PB3     Keyboard row 3, PSG data 4, centronics data 3
	    PB4     Keyboard row 4, PSG data 3, centronics data 4
	    PB5     Keyboard row 5, PSG data 2, centronics data 5
	    PB6     Keyboard row 6, PSG data 1, centronics data 6
	    PB7     Keyboard row 7, PSG data 0, centronics data 7
	*/

	/* keyboard latch */
	m_keylatch = data;

	/* centronics data */
	m_centronics->write(space, 0, data);
}

READ_LINE_MEMBER( laser2001_state::pia_ca1_r )
{
	return (m_cassette)->input() > -0.1469;
}

WRITE_LINE_MEMBER( laser2001_state::pia_ca2_w )
{
	m_cassette->output(state ? +1.0 : -1.0);
}

READ_LINE_MEMBER( laser2001_state::pia_cb1_r )
{
	/* actually this is a diode-AND (READY & _BUSY), but ctronics.c returns busy status if printer image is not mounted -> Manager won't boot */
	return m_psg->ready_r() && (m_centronics->not_busy_r() || m_pia->ca2_output_z());
}

WRITE_LINE_MEMBER( laser2001_state::pia_cb2_w )
{
	if (m_pia->ca2_output_z())
	{
		if (!state) m_psg->write(m_keylatch);
	}
	else
	{
		m_centronics->strobe_w(state);
	}
}

static const pia6821_interface lasr2001_pia_intf =
{
	DEVCB_DRIVER_MEMBER(laser2001_state, pia_pa_r),             // input A
	DEVCB_DRIVER_MEMBER(laser2001_state, pia_pb_r),             // input B
	DEVCB_DRIVER_LINE_MEMBER(laser2001_state, pia_ca1_r),       // input CA1
	DEVCB_DRIVER_LINE_MEMBER(laser2001_state, pia_cb1_r),       // input CB1
	DEVCB_LINE_GND,                                             // input CA2
	DEVCB_LINE_VCC,                                             // input CB2 (+5V)
	DEVCB_DRIVER_MEMBER(laser2001_state, pia_pa_w),             // output A
	DEVCB_DRIVER_MEMBER(laser2001_state, pia_pb_w),             // output B
	DEVCB_DRIVER_LINE_MEMBER(laser2001_state, pia_ca2_w),       // output CA2
	DEVCB_DRIVER_LINE_MEMBER(laser2001_state, pia_cb2_w),       // output CB2
	DEVCB_NULL,                                                 // irq A (floating)
	DEVCB_NULL                                                  // irq B (floating)
};

/*-------------------------------------------------
    cassette_interface crvision_cassette_interface
-------------------------------------------------*/

static const cassette_interface crvision_cassette_interface =
{
	cassette_default_formats,
	NULL,
	(cassette_state)(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED),
	NULL,
	NULL
};

/*-------------------------------------------------
    cassette_interface lasr2001_cassette_interface
-------------------------------------------------*/

static const cassette_interface lasr2001_cassette_interface =
{
	cassette_default_formats,
	NULL,
	(cassette_state)(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED),
	NULL,
	NULL
};

/*-------------------------------------------------
    floppy_interface lasr2001_floppy_interface
-------------------------------------------------*/

static const floppy_interface lasr2001_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_SSDD,
	LEGACY_FLOPPY_OPTIONS_NAME(default),
	NULL,
	NULL
};

/*-------------------------------------------------
    centronics_interface lasr2001_centronics_intf
-------------------------------------------------*/

static const centronics_interface lasr2001_centronics_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(PIA6821_TAG, pia6821_device, cb1_w)
};

/*-------------------------------------------------
    sn76496_config psg_intf
-------------------------------------------------*/

static const sn76496_config psg_intf =
{
	DEVCB_NULL
};

/***************************************************************************
    MACHINE INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    MACHINE_START( creativision )
-------------------------------------------------*/

void crvision_state::machine_start()
{
	// state saving
	save_item(NAME(m_keylatch));
}

void crvision_pal_state::machine_start()
{
	// state saving
	save_item(NAME(m_keylatch));
}

/*-------------------------------------------------
    MACHINE_START( lasr2001 )
-------------------------------------------------*/

void laser2001_state::machine_start()
{
	// state saving
	save_item(NAME(m_keylatch));
}

/***************************************************************************
    CARTRIDGE
***************************************************************************/

static DEVICE_IMAGE_LOAD( crvision_cart )
{
	UINT32 size;
	UINT8 *temp_copy;
	running_machine &machine = image.device().machine();
	crvision_state *state = machine.driver_data<crvision_state>();
	UINT8 *mem = state->memregion(M6502_TAG)->base();
	address_space &program = machine.device(M6502_TAG)->memory().space(AS_PROGRAM);

	if (image.software_entry() == NULL)
	{
		size = image.length();
		temp_copy = auto_alloc_array(machine, UINT8, size);
		image.fread( temp_copy, size);
	}
	else
	{
		size= image.get_software_region_length("rom");
		temp_copy = auto_alloc_array(machine, UINT8, size);
		memcpy(temp_copy, image.get_software_region("rom"), size);
	}

	switch (size)
	{
	case 0x1000: // 4K
		memcpy(mem + 0x9000, temp_copy, 0x1000);            // load 4KB at 0x9000
		memcpy(mem + 0xb000, mem + 0x9000, 0x1000);         // mirror 4KB at 0xb000
		program.install_read_bank(0x8000, 0xbfff, 0, 0x2000, BANK_ROM1);
		break;

	case 0x1800: // 6K
		memcpy(mem + 0x9000, temp_copy, 0x1000);            // load lower 4KB at 0x9000
		memcpy(mem + 0xb000, mem + 0x9000, 0x1000);         // mirror lower 4KB at 0xb000
		memcpy(mem + 0x8000, temp_copy + 0x1000, 0x0800);   // load higher 2KB at 0x8000
		memcpy(mem + 0x8800, mem + 0x8000, 0x0800);         // mirror higher 2KB at 0x8800
		memcpy(mem + 0xa000, mem + 0x8000, 0x0800);         // mirror higher 2KB at 0xa000
		memcpy(mem + 0xa800, mem + 0x8000, 0x0800);         // mirror higher 2KB at 0xa800
		program.install_read_bank(0x8000, 0xbfff, 0, 0x2000, BANK_ROM1);
		break;

	case 0x2000: // 8K
		memcpy(mem + 0x8000, temp_copy, 0x2000);            // load 8KB at 0x8000
		memcpy(mem + 0xa000, mem + 0x8000, 0x2000);         // mirror 8KB at 0xa000
		program.install_read_bank(0x8000, 0xbfff, 0, 0x2000, BANK_ROM1);
		break;

	case 0x2800: // 10K
		memcpy(mem + 0x8000, temp_copy, 0x2000);            // load lower 8KB at 0x8000
		memcpy(mem + 0xa000, mem + 0x8000, 0x2000);         // mirror lower 8KB at 0xa000
		memcpy(mem + 0x4000, temp_copy + 0x2000, 0x0800);   // load higher 2KB at 0x4000
		memcpy(mem + 0x4800, mem + 0x4000, 0x0800);         // mirror higher 2KB at 0x4800
		memcpy(mem + 0x5000, mem + 0x4000, 0x0800);         // mirror higher 2KB at 0x5000
		memcpy(mem + 0x5800, mem + 0x4000, 0x0800);         // mirror higher 2KB at 0x5800
		memcpy(mem + 0x6000, mem + 0x4000, 0x0800);         // mirror higher 2KB at 0x6000
		memcpy(mem + 0x6800, mem + 0x4000, 0x0800);         // mirror higher 2KB at 0x6800
		memcpy(mem + 0x7000, mem + 0x4000, 0x0800);         // mirror higher 2KB at 0x7000
		memcpy(mem + 0x7800, mem + 0x4000, 0x0800);         // mirror higher 2KB at 0x7800
		program.install_read_bank(0x8000, 0xbfff, BANK_ROM1);
		program.install_read_bank(0x4000, 0x7fff, BANK_ROM2);
		break;

	case 0x3000: // 12K
		memcpy(mem + 0x8000, temp_copy, 0x2000);            // load lower 8KB at 0x8000
		memcpy(mem + 0xa000, mem + 0x8000, 0x2000);         // mirror lower 8KB at 0xa000
		memcpy(mem + 0x4000, temp_copy + 0x2000, 0x1000);   // load higher 4KB at 0x4000
		memcpy(mem + 0x5000, mem + 0x4000, 0x1000);         // mirror higher 4KB at 0x5000
		memcpy(mem + 0x6000, mem + 0x4000, 0x1000);         // mirror higher 4KB at 0x6000
		memcpy(mem + 0x7000, mem + 0x4000, 0x1000);         // mirror higher 4KB at 0x7000
		program.install_read_bank(0x8000, 0xbfff, BANK_ROM1);
		program.install_read_bank(0x4000, 0x7fff, BANK_ROM2);
		break;

	case 0x4000: // 16K
		memcpy(mem + 0xa000, temp_copy, 0x2000);            // load lower 8KB at 0xa000
		memcpy(mem + 0x8000, temp_copy + 0x2000, 0x2000);   // load higher 8KB at 0x8000
		program.install_read_bank(0x8000, 0xbfff, BANK_ROM1);
		program.install_read_bank(0x4000, 0x7fff, BANK_ROM2);
		break;

	case 0x4800: // 18K
		memcpy(mem + 0xa000, temp_copy, 0x2000);            // load lower 8KB at 0xa000
		memcpy(mem + 0x8000, temp_copy + 0x2000, 0x2000);   // load higher 8KB at 0x8000
		memcpy(mem + 0x4000, temp_copy + 0x4000, 0x0800);   // load higher 2KB at 0x4000
		memcpy(mem + 0x4800, mem + 0x4000, 0x0800);         // mirror higher 2KB at 0x4800
		memcpy(mem + 0x5000, mem + 0x4000, 0x0800);         // mirror higher 2KB at 0x5000
		memcpy(mem + 0x5800, mem + 0x4000, 0x0800);         // mirror higher 2KB at 0x5800
		memcpy(mem + 0x6000, mem + 0x4000, 0x0800);         // mirror higher 2KB at 0x6000
		memcpy(mem + 0x6800, mem + 0x4000, 0x0800);         // mirror higher 2KB at 0x6800
		memcpy(mem + 0x7000, mem + 0x4000, 0x0800);         // mirror higher 2KB at 0x7000
		memcpy(mem + 0x7800, mem + 0x4000, 0x0800);         // mirror higher 2KB at 0x7800
		program.install_read_bank(0x8000, 0xbfff, BANK_ROM1);
		program.install_read_bank(0x4000, 0x7fff, BANK_ROM2);
		break;

	default:
		auto_free(machine, temp_copy);
		return IMAGE_INIT_FAIL;
	}

	state->membank("bank1")->configure_entry(0, mem + 0x8000);
	state->membank("bank1")->set_entry(0);

	state->membank("bank2")->configure_entry(0, mem + 0x4000);
	state->membank("bank2")->set_entry(0);

	auto_free(machine, temp_copy);

	return IMAGE_INIT_PASS;
}

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

/*-------------------------------------------------
    MACHINE_CONFIG_START( creativision, crvision_state )
-------------------------------------------------*/

static MACHINE_CONFIG_START( creativision, crvision_state )
	// basic machine hardware
	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_2MHz)
	MCFG_CPU_PROGRAM_MAP(crvision_map)

	// devices
	MCFG_PIA6821_ADD(PIA6821_TAG, pia_intf)
	MCFG_CASSETTE_ADD(CASSETTE_TAG, crvision_cassette_interface)
	MCFG_CENTRONICS_PRINTER_ADD(CENTRONICS_TAG, standard_centronics)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SN76489_TAG, SN76489A, XTAL_2MHz)
	MCFG_SOUND_CONFIG(psg_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_SOUND_WAVE_ADD(WAVE_TAG, CASSETTE_TAG)
	MCFG_SOUND_ROUTE(1, "mono", 0.25)

	// cartridge
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("bin,rom")
	MCFG_CARTSLOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("crvision_cart")
	MCFG_CARTSLOT_LOAD(crvision_cart)

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("1K") // MAIN RAM
	MCFG_RAM_EXTRA_OPTIONS("15K") // 16K expansion (lower 14K available only, upper 2K shared with BIOS ROM)

	// software lists
	MCFG_SOFTWARE_LIST_ADD("cart_list","crvision")
MACHINE_CONFIG_END

/*-------------------------------------------------
    MACHINE_CONFIG_DERIVED( ntsc, creativision )
-------------------------------------------------*/

static MACHINE_CONFIG_DERIVED( ntsc, creativision )
	// video hardware
	MCFG_TMS9928A_ADD( TMS9929_TAG, TMS9918, vdp_intf )
	MCFG_TMS9928A_SCREEN_ADD_NTSC( SCREEN_TAG )
	MCFG_SCREEN_UPDATE_DEVICE( TMS9929_TAG, tms9918_device, screen_update )
MACHINE_CONFIG_END

/*-------------------------------------------------
    MACHINE_CONFIG_DERIVED( pal, creativision )
-------------------------------------------------*/

static MACHINE_CONFIG_DERIVED_CLASS( pal, creativision, crvision_pal_state )
	// video hardware
	MCFG_TMS9928A_ADD( TMS9929_TAG, TMS9929, vdp_intf )
	MCFG_TMS9928A_SCREEN_ADD_PAL( SCREEN_TAG )
	MCFG_SCREEN_UPDATE_DEVICE( TMS9929_TAG, tms9929_device, screen_update )
MACHINE_CONFIG_END

/*-------------------------------------------------
    MACHINE_CONFIG_START( lasr2001, laser2001_state )
-------------------------------------------------*/

static MACHINE_CONFIG_START( lasr2001, laser2001_state )
	// basic machine hardware
	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_17_73447MHz/9)
	MCFG_CPU_PROGRAM_MAP(lasr2001_map)

	// devices
	MCFG_PIA6821_ADD(PIA6821_TAG, lasr2001_pia_intf)
	MCFG_CASSETTE_ADD(CASSETTE_TAG, lasr2001_cassette_interface)
	MCFG_LEGACY_FLOPPY_DRIVE_ADD(FLOPPY_0, lasr2001_floppy_interface)
	MCFG_CENTRONICS_PRINTER_ADD(CENTRONICS_TAG, lasr2001_centronics_intf)

	// video hardware
	MCFG_TMS9928A_ADD( TMS9929_TAG, TMS9929A, vdp_intf )
	MCFG_TMS9928A_SCREEN_ADD_PAL( SCREEN_TAG )
	MCFG_SCREEN_UPDATE_DEVICE( TMS9929_TAG, tms9929a_device, screen_update )

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SN76489_TAG, SN76489A, XTAL_17_73447MHz/9)
	MCFG_SOUND_CONFIG(psg_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_SOUND_WAVE_ADD(WAVE_TAG, CASSETTE_TAG)
	MCFG_SOUND_ROUTE(1, "mono", 0.25)

	// cartridge
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("bin,rom")
	MCFG_CARTSLOT_INTERFACE("crvision_cart")
	MCFG_CARTSLOT_LOAD(crvision_cart)

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("16K")
	MCFG_RAM_EXTRA_OPTIONS("32K")

	// software list
	MCFG_SOFTWARE_LIST_ADD("cart_list","crvision")
MACHINE_CONFIG_END

/***************************************************************************
    ROMS
***************************************************************************/

ROM_START( crvision )
	ROM_REGION( 0x10000, M6502_TAG, 0 )
	ROM_LOAD( "crvision.u20", 0x0000, 0x0800, CRC(c3c590c6) SHA1(5ac620c529e4965efb5560fe824854a44c983757) )
ROM_END

ROM_START( fnvision )
	ROM_REGION( 0x10000, M6502_TAG, 0 )
	ROM_LOAD( "funboot.rom",  0x0000, 0x0800, CRC(05602697) SHA1(c280b20c8074ba9abb4be4338b538361dfae517f) )
ROM_END

#define rom_wizzard rom_crvision
#define rom_crvisioj rom_crvision
#define rom_crvisio2 rom_crvision
#define rom_rameses rom_fnvision
#define rom_vz2000 rom_fnvision

ROM_START( manager )
	ROM_REGION( 0x10000, M6502_TAG, 0 )
	ROM_LOAD( "01", 0x0000, 0x2000, CRC(702f4cf5) SHA1(cd14ee74e787d24b76c166de484dae24206e219b) )
	ROM_LOAD( "23", 0x2000, 0x2000, CRC(46489d88) SHA1(467f5bcd62d0b4117c443e13373df8f3c45df7b2) )

	ROM_REGION( 0x1000, "disk", 0 )
	ROM_LOAD( "floppy interface cartridge", 0x0000, 0x1000, NO_DUMP )
ROM_END

/***************************************************************************
    SYSTEM DRIVERS
***************************************************************************/

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT       INIT    COMPANY                   FULLNAME */
CONS( 1982, crvision,   0,          0,      pal,        crvision, driver_device,    0,      "Video Technology",         "CreatiVision", 0 )
CONS( 1982, fnvision,   crvision,   0,      pal,        crvision, driver_device,    0,      "Video Technology",         "FunVision", 0 )
CONS( 1982, crvisioj,   crvision,   0,      ntsc,       crvision, driver_device,    0,      "Cheryco",                  "CreatiVision (Japan)", 0 )
CONS( 1982, wizzard,    crvision,   0,      pal,        crvision, driver_device,    0,      "Dick Smith Electronics",   "Wizzard (Oceania)", 0 )
CONS( 1982, rameses,    crvision,   0,      pal,        crvision, driver_device,    0,      "Hanimex",                  "Rameses (Oceania)", 0 )
CONS( 1983, vz2000,     crvision,   0,      pal,        crvision, driver_device,    0,      "Dick Smith Electronics",   "VZ 2000 (Oceania)", 0 )
CONS( 1983, crvisio2,   crvision,   0,      pal,        crvision, driver_device,    0,      "Video Technology",         "CreatiVision MK-II (Europe)", 0 )
//COMP( 1983, lasr2001,   0,          0,      lasr2001,   lasr2001, driver_device,   0,      "Video Technology",         "Laser 2001", GAME_NOT_WORKING )
//COMP( 1983, vz2001,     lasr2001,   0,      lasr2001,   lasr2001, driver_device,   0,      "Dick Smith Electronics",   "VZ 2001 (Oceania)", GAME_NOT_WORKING )
COMP( 1983, manager,    0,          0,      lasr2001,   manager, driver_device, 0,      "Salora",                   "Manager (Finland)", 0 )
