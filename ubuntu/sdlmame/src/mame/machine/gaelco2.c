/***************************************************************************

    Gaelco CG-1V/GAE1 based games

    Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
    I/O ports)

***************************************************************************/

#include "emu.h"
#include "machine/eeprom.h"
#include "includes/gaelco2.h"

/***************************************************************************

    Split even/odd bytes from ROMs in 16 bit mode to different memory areas

***************************************************************************/

static void gaelco2_ROM16_split_gfx(running_machine &machine, const char *src_reg, const char *dst_reg, int start, int length, int dest1, int dest2)
{
	int i;

	/* get a pointer to the source data */
	UINT8 *src = (UINT8 *)machine.root_device().memregion(src_reg)->base();

	/* get a pointer to the destination data */
	UINT8 *dst = (UINT8 *)machine.root_device().memregion(dst_reg)->base();

	/* fill destination areas with the proper data */
	for (i = 0; i < length/2; i++){
		dst[dest1 + i] = src[start + i*2 + 0];
		dst[dest2 + i] = src[start + i*2 + 1];
	}
}


/***************************************************************************

    Driver init routines

***************************************************************************/

DRIVER_INIT_MEMBER(gaelco2_state,alighunt)
{
	/*
	For "gfx2" we have this memory map:
	    0x0000000-0x03fffff ROM u48
	    0x0400000-0x07fffff ROM u47
	    0x0800000-0x0bfffff ROM u50
	    0x0c00000-0x0ffffff ROM u49

	and we are going to construct this one for "gfx1":
	    0x0000000-0x01fffff ROM u48 even bytes
	    0x0200000-0x03fffff ROM u47 even bytes
	    0x0400000-0x05fffff ROM u48 odd bytes
	    0x0600000-0x07fffff ROM u47 odd bytes
	    0x0800000-0x09fffff ROM u50 even bytes
	    0x0a00000-0x0bfffff ROM u49 even bytes
	    0x0c00000-0x0dfffff ROM u50 odd bytes
	    0x0e00000-0x0ffffff ROM u49 odd bytes
	*/

	/* split ROM u48 */
	gaelco2_ROM16_split_gfx(machine(), "gfx2", "gfx1", 0x0000000, 0x0400000, 0x0000000, 0x0400000);

	/* split ROM u47 */
	gaelco2_ROM16_split_gfx(machine(), "gfx2", "gfx1", 0x0400000, 0x0400000, 0x0200000, 0x0600000);

	/* split ROM u50 */
	gaelco2_ROM16_split_gfx(machine(), "gfx2", "gfx1", 0x0800000, 0x0400000, 0x0800000, 0x0c00000);

	/* split ROM u49 */
	gaelco2_ROM16_split_gfx(machine(), "gfx2", "gfx1", 0x0c00000, 0x0400000, 0x0a00000, 0x0e00000);
}


DRIVER_INIT_MEMBER(gaelco2_state,touchgo)
{
	/*
	For "gfx2" we have this memory map:
	    0x0000000-0x03fffff ROM ic65
	    0x0400000-0x05fffff ROM ic66
	    0x0800000-0x0bfffff ROM ic67

	and we are going to construct this one for "gfx1":
	    0x0000000-0x01fffff ROM ic65 even bytes
	    0x0200000-0x02fffff ROM ic66 even bytes
	    0x0400000-0x05fffff ROM ic65 odd bytes
	    0x0600000-0x06fffff ROM ic66 odd bytes
	    0x0800000-0x09fffff ROM ic67 even bytes
	    0x0c00000-0x0dfffff ROM ic67 odd bytes
	*/

	/* split ROM ic65 */
	gaelco2_ROM16_split_gfx(machine(), "gfx2", "gfx1", 0x0000000, 0x0400000, 0x0000000, 0x0400000);

	/* split ROM ic66 */
	gaelco2_ROM16_split_gfx(machine(), "gfx2", "gfx1", 0x0400000, 0x0200000, 0x0200000, 0x0600000);

	/* split ROM ic67 */
	gaelco2_ROM16_split_gfx(machine(), "gfx2", "gfx1", 0x0800000, 0x0400000, 0x0800000, 0x0c00000);
}


DRIVER_INIT_MEMBER(gaelco2_state,snowboar)
{
	/*
	For "gfx2" we have this memory map:
	    0x0000000-0x03fffff ROM sb44
	    0x0400000-0x07fffff ROM sb45
	    0x0800000-0x0bfffff ROM sb46

	and we are going to construct this one for "gfx1":
	    0x0000000-0x01fffff ROM sb44 even bytes
	    0x0200000-0x03fffff ROM sb45 even bytes
	    0x0400000-0x05fffff ROM sb44 odd bytes
	    0x0600000-0x07fffff ROM sb45 odd bytes
	    0x0800000-0x09fffff ROM sb46 even bytes
	    0x0c00000-0x0dfffff ROM sb46 odd bytes
	*/

	/* split ROM sb44 */
	gaelco2_ROM16_split_gfx(machine(), "gfx2", "gfx1", 0x0000000, 0x0400000, 0x0000000, 0x0400000);

	/* split ROM sb45 */
	gaelco2_ROM16_split_gfx(machine(), "gfx2", "gfx1", 0x0400000, 0x0400000, 0x0200000, 0x0600000);

	/* split ROM sb46 */
	gaelco2_ROM16_split_gfx(machine(), "gfx2", "gfx1", 0x0800000, 0x0400000, 0x0800000, 0x0c00000);
}

/***************************************************************************

    Coin counters/lockouts

***************************************************************************/

WRITE16_MEMBER(gaelco2_state::gaelco2_coin_w)
{
	/* Coin Lockouts */
	coin_lockout_w(machine(), 0, ~data & 0x01);
	coin_lockout_w(machine(), 1, ~data & 0x02);

	/* Coin Counters */
	coin_counter_w(machine(), 0, data & 0x04);
	coin_counter_w(machine(), 1, data & 0x08);
}

WRITE16_MEMBER(gaelco2_state::gaelco2_coin2_w)
{
	/* coin counters */
	coin_counter_w(machine(), offset & 0x01,  data & 0x01);
}

WRITE16_MEMBER(gaelco2_state::wrally2_coin_w)
{
	/* coin counters */
	coin_counter_w(machine(), (offset >> 3) & 0x01,  data & 0x01);
}

WRITE16_MEMBER(gaelco2_state::touchgo_coin_w)
{
	if ((offset >> 2) == 0){
		coin_counter_w(machine(), 0, data & 0x01);
		coin_counter_w(machine(), 1, data & 0x02);
		coin_counter_w(machine(), 2, data & 0x04);
		coin_counter_w(machine(), 3, data & 0x08);
	}
}

/***************************************************************************

    Bang

***************************************************************************/


DRIVER_INIT_MEMBER(gaelco2_state,bang)
{
	m_clr_gun_int = 0;
}

WRITE16_MEMBER(gaelco2_state::bang_clr_gun_int_w)
{
	m_clr_gun_int = 1;
}

TIMER_DEVICE_CALLBACK_MEMBER(gaelco2_state::bang_irq)
{
	int scanline = param;

	if (scanline == 256){
		m_maincpu->set_input_line(2, HOLD_LINE);
		m_clr_gun_int = 0;
	}

	if ((scanline % 64) == 0 && m_clr_gun_int)
		m_maincpu->set_input_line(4, HOLD_LINE);
}

/***************************************************************************

    World Rally 2 analog controls
    - added by Mirko Mattioli <els@fastwebnet.it>
    ---------------------------------------------------------------
    WR2 pcb has two ADC, one for each player. The ADCs have in common
    the clock signal line (adc_clk) and the chip enable signal line
    (adc_cs) and, of course,  two different data out signal lines.
    When "Pot Wheel" option is selected via dip-switch, then the gear
    is enabled (low/high shifter); the gear is disabled in joy mode by
    the CPU program code. No brakes are present in this game.
    Analog controls routines come from modified code wrote by Aaron
    Giles for gaelco3d driver.

***************************************************************************/


CUSTOM_INPUT_MEMBER(gaelco2_state::wrally2_analog_bit_r)
{
	int which = (FPTR)param;
	return (m_analog_ports[which] >> 7) & 0x01;
}


WRITE16_MEMBER(gaelco2_state::wrally2_adc_clk)
{
	/* a zero/one combo is written here to clock the next analog port bit */
	if (ACCESSING_BITS_0_7)
	{
		if (!(data & 0xff))
		{
			m_analog_ports[0] <<= 1;
			m_analog_ports[1] <<= 1;
		}
	}
	else
		logerror("%06X:analog_port_clock_w(%02X) = %08X & %08X\n", space.device().safe_pc(), offset, data, mem_mask);
}


WRITE16_MEMBER(gaelco2_state::wrally2_adc_cs)
{
	/* a zero is written here to read the analog ports, and a one is written when finished */
	if (ACCESSING_BITS_0_7)
	{
		if (!(data & 0xff))
		{
			m_analog_ports[0] = ioport("ANALOG0")->read_safe(0);
			m_analog_ports[1] = ioport("ANALOG1")->read_safe(0);
		}
	}
	else
		logerror("%06X:analog_port_latch_w(%02X) = %08X & %08X\n", space.device().safe_pc(), offset, data, mem_mask);
}

/***************************************************************************

    EEPROM (93C66)

***************************************************************************/

WRITE16_MEMBER(gaelco2_state::gaelco2_eeprom_cs_w)
{
	/* bit 0 is CS (active low) */
	eeprom_device *eeprom = downcast<eeprom_device *>(machine().device("eeprom"));
	eeprom->set_cs_line((data & 0x01) ? CLEAR_LINE : ASSERT_LINE);
}

WRITE16_MEMBER(gaelco2_state::gaelco2_eeprom_sk_w)
{
	/* bit 0 is SK (active high) */
	eeprom_device *eeprom = downcast<eeprom_device *>(machine().device("eeprom"));
	eeprom->set_clock_line((data & 0x01) ? ASSERT_LINE : CLEAR_LINE);
}

WRITE16_MEMBER(gaelco2_state::gaelco2_eeprom_data_w)
{
	/* bit 0 is EEPROM data (DIN) */
	eeprom_device *eeprom = downcast<eeprom_device *>(machine().device("eeprom"));
	eeprom->write_bit(data & 0x01);
}

/***************************************************************************

    Protection

***************************************************************************/


/*
    The game writes 2 values and then reads from a memory address.
    If the read value is wrong, the game can crash in some places.
    If we always return 0, the game doesn't crash but you can't see
    the full intro (because it expects 0xffff somewhere).

    The protection handles sound, controls, gameplay and some sprites
*/

READ16_MEMBER(gaelco2_state::snowboar_protection_r)
{
	logerror("%06x: protection read from %04x\n", space.device().safe_pc(), offset*2);
	return 0x0000;
}

WRITE16_MEMBER(gaelco2_state::snowboar_protection_w)
{
	COMBINE_DATA(&m_snowboar_protection[offset]);
	logerror("%06x: protection write %04x to %04x\n", space.device().safe_pc(), data, offset*2);

}
