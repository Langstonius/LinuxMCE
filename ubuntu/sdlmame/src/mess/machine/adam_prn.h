/**********************************************************************

    Coleco Adam floppy disk controller emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __ADAM_PRN__
#define __ADAM_PRN__

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/adamnet.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> adam_printer_device

class adam_printer_device :  public device_t,
								public device_adamnet_card_interface
{
public:
	// construction/destruction
	adam_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

	// not really public
	DECLARE_WRITE8_MEMBER( p1_w );
	DECLARE_READ8_MEMBER( p2_r );
	DECLARE_WRITE8_MEMBER( p2_w );
	DECLARE_READ8_MEMBER( p3_r );
	DECLARE_READ8_MEMBER( p4_r );
	DECLARE_WRITE8_MEMBER( p4_w );

protected:
	// device-level overrides
	virtual void device_config_complete() { m_shortname = "adam_prn"; }
	virtual void device_start();

	// device_adamnet_card_interface overrides
	virtual void adamnet_reset_w(int state);

	required_device<cpu_device> m_maincpu;
};


// device type definition
extern const device_type ADAM_PRN;



#endif
