/**********************************************************************

    Intel 8355 - 16,384-Bit ROM with I/O emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "emu.h"
#include "i8355.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0

enum
{
	REGISTER_PORT_A = 0,
	REGISTER_PORT_B,
	REGISTER_PORT_A_DDR,
	REGISTER_PORT_B_DDR
};

enum
{
	PORT_A = 0,
	PORT_B,
	PORT_COUNT
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type I8355 = &device_creator<i8355_device>;

// default address map
static ADDRESS_MAP_START( i8355, AS_0, 8, i8355_device )
	AM_RANGE(0x000, 0x7ff) AM_ROM
ADDRESS_MAP_END



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  read_port - read from input port
//-------------------------------------------------

inline UINT8 i8355_device::read_port(int port)
{
	UINT8 data = m_output[port] & m_ddr[port];

	if (m_ddr[port] != 0xff)
	{
		data |= m_in_port_func[port](0) & ~m_ddr[port];
	}

	return data;
}


//-------------------------------------------------
//  write_port - write to output port
//-------------------------------------------------

inline void i8355_device::write_port(int port, UINT8 data)
{
	m_output[port] = data;

	m_out_port_func[port](0, m_output[port] & m_ddr[port]);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  i8355_device - constructor
//-------------------------------------------------

i8355_device::i8355_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, I8355, "Intel 8355", tag, owner, clock),
		device_memory_interface(mconfig, *this),
		m_space_config("ram", ENDIANNESS_LITTLE, 8, 11, 0, NULL, *ADDRESS_MAP_NAME(i8355))
{

}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void i8355_device::device_config_complete()
{
	// inherit a copy of the static data
	const i8355_interface *intf = reinterpret_cast<const i8355_interface *>(static_config());
	if (intf != NULL)
		*static_cast<i8355_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_in_pa_cb, 0, sizeof(m_in_pa_cb));
		memset(&m_out_pa_cb, 0, sizeof(m_out_pa_cb));
		memset(&m_in_pb_cb, 0, sizeof(m_in_pb_cb));
		memset(&m_out_pb_cb, 0, sizeof(m_out_pb_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8355_device::device_start()
{
	// resolve callbacks
	m_in_port_func[0].resolve(m_in_pa_cb, *this);
	m_in_port_func[1].resolve(m_in_pb_cb, *this);
	m_out_port_func[0].resolve(m_out_pa_cb, *this);
	m_out_port_func[1].resolve(m_out_pb_cb, *this);

	// register for state saving
	save_item(NAME(m_output));
	save_item(NAME(m_ddr));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8355_device::device_reset()
{
	// set ports to input mode
	m_ddr[PORT_A] = 0;
	m_ddr[PORT_B] = 0;
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *i8355_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_0) ? &m_space_config : NULL;
}



//-------------------------------------------------
//  io_r - register read
//-------------------------------------------------

READ8_MEMBER( i8355_device::io_r )
{
	int port = offset & 0x01;

	UINT8 data = 0;

	switch (offset & 0x03)
	{
	case REGISTER_PORT_A:
	case REGISTER_PORT_B:
		data = read_port(port);
		break;

	case REGISTER_PORT_A_DDR:
	case REGISTER_PORT_B_DDR:
		// write only
		break;
	}

	return data;
}


//-------------------------------------------------
//  io_w - register write
//-------------------------------------------------

WRITE8_MEMBER( i8355_device::io_w )
{
	int port = offset & 0x01;

	switch (offset & 0x03)
	{
	case REGISTER_PORT_A:
	case REGISTER_PORT_B:
		if (LOG) logerror("I8355 '%s' Port %c Write %02x\n", tag(), 'A' + port, data);

		write_port(port, data);
		break;

	case REGISTER_PORT_A_DDR:
	case REGISTER_PORT_B_DDR:
		if (LOG) logerror("I8355 '%s' Port %c DDR: %02x\n", tag(), 'A' + port, data);

		m_ddr[port] = data;
		write_port(port, data);
		break;
	}
}


//-------------------------------------------------
//  memory_r - internal ROM read
//-------------------------------------------------

READ8_MEMBER( i8355_device::memory_r )
{
	return this->space().read_byte(offset);
}
