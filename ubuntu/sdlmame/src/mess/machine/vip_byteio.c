/**********************************************************************

    RCA Cosmac VIP Byte Input/Output port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "machine/vip_byteio.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type VIP_BYTEIO_PORT = &device_creator<vip_byteio_port_device>;



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_vip_byteio_port_interface - constructor
//-------------------------------------------------

device_vip_byteio_port_interface::device_vip_byteio_port_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig,device)
{
	m_slot = dynamic_cast<vip_byteio_port_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_vip_byteio_port_interface - destructor
//-------------------------------------------------

device_vip_byteio_port_interface::~device_vip_byteio_port_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vip_byteio_port_device - constructor
//-------------------------------------------------

vip_byteio_port_device::vip_byteio_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, VIP_BYTEIO_PORT, "VIP byte I/O port", tag, owner, clock),
		device_slot_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  vip_byteio_port_device - destructor
//-------------------------------------------------

vip_byteio_port_device::~vip_byteio_port_device()
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void vip_byteio_port_device::device_config_complete()
{
	// inherit a copy of the static data
	const vip_byteio_port_interface *intf = reinterpret_cast<const vip_byteio_port_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<vip_byteio_port_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_inst_cb, 0, sizeof(m_out_inst_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vip_byteio_port_device::device_start()
{
	m_cart = dynamic_cast<device_vip_byteio_port_interface *>(get_card_device());

	// resolve callbacks
	m_out_inst_func.resolve(m_out_inst_cb, *this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vip_byteio_port_device::device_reset()
{
}


UINT8 vip_byteio_port_device::in_r() { UINT8 data = 0xff; if (m_cart != NULL) data = m_cart->vip_in_r(); return data; }
void vip_byteio_port_device::out_w(UINT8 data) { if (m_cart != NULL) m_cart->vip_out_w(data); }
READ_LINE_MEMBER( vip_byteio_port_device::ef3_r ) { int state = CLEAR_LINE; if (m_cart != NULL) state = m_cart->vip_ef3_r(); return state; }
READ_LINE_MEMBER( vip_byteio_port_device::ef4_r ) { int state = CLEAR_LINE; if (m_cart != NULL) state = m_cart->vip_ef4_r(); return state; }
WRITE_LINE_MEMBER( vip_byteio_port_device::q_w ) { if (m_cart != NULL) m_cart->vip_q_w(state); }

WRITE_LINE_MEMBER( vip_byteio_port_device::inst_w ) { m_out_inst_func(state); }


//-------------------------------------------------
//  SLOT_INTERFACE( vip_byteio_cards )
//-------------------------------------------------

SLOT_INTERFACE_START( vip_byteio_cards )
	//SLOT_INTERFACE("exp2", VP576_BYTEIO)
	SLOT_INTERFACE("ascii", VP620)
SLOT_INTERFACE_END
