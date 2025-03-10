/***************************************************************************

  a2bus.c - Apple II slot bus and card emulation

  by R. Belmont

  Pinout (/ indicates an inverted signal, ie, one that would have a bar over it
          on a schematic diagram)

      (rear of computer)

     GND  26  25  +5V
  DMA IN  27  24  DMA OUT
  INT IN  28  23  INT OUT
    /NMI  29  22  /DMA
    /IRQ  30  21  RDY
    /RES  31  20  /IOSTB
    /INH  32  19  N.C.
    -12V  33  18  R/W
     -5V  34  17  A15
    N.C.  35  16  A14
      7M  36  15  A13
      Q3  37  14  A12
     PH1  38  13  A11
  USER 1  39  12  A10
     PH0  40  11  A9
 /DEVSEL  41  10  A8
      D7  42   9  A7
      D6  43   8  A6
      D5  44   7  A5
      D4  45   6  A4
      D3  46   5  A3
      D2  47   4  A2
      D1  48   3  A1
      D0  49   2  A0
    -12V  50   1  /IOSEL

     (front of computer)

    Signal descriptions:
    GND - power supply ground
    DMA IN - daisy chain of DMA signal from higher priority devices.  usually connected to DMA OUT.
    INT IN - similar to DMA IN but for INT instead of DMA.
    /NMI - non-maskable interrupt input to the 6502
    /IRQ - maskable interrupt input to the 6502
    /RES - system reset signal
    /INH - On the II and II+, inhibits the motherboard ROMs, allowing a card to replace them.
           On the IIe, inhibits all motherboard RAM/ROM for both CPU and DMA accesses.
           On the IIgs, works like the IIe except for the address range 0x6000 to 0x9FFF where
           it will cause bus contention.
    -12V - negative 12 volts DC power
     -5V - negative 5 volts DC power
      7M - 7 MHz clock (1/4th of the master clock on the IIgs, 1/2 on 8-bit IIs)
      Q3 - 2 MHz asymmetrical clock
     PH1 - 6502 phase 1 clock
 /DEVSEL - asserted on an access to C0nX, where n = the slot number plus 8.
   D0-D7 - 8-bit data bus
     +5V - 5 volts DC power
 DMA OUT - see DMA IN
 INT OUT - see INT IN
    /DMA - pulling this low disconnects the 6502 from the bus and halts it
     RDY - 6502 RDY input.  Pulling this low when PH1 is active will halt the
           6502 and latch the current address bus value.
  /IOSTB - asserted on an access between C800 and CFFF.
  A0-A15 - 16-bit address bus
  /IOSEL - asserted on accesses to CnXX where n is the slot number.
           Not present on slot 0.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "machine/a2bus.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_SLOT = &device_creator<a2bus_slot_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a2bus_slot_device - constructor
//-------------------------------------------------
a2bus_slot_device::a2bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, A2BUS_SLOT, "Apple II Slot", tag, owner, clock),
		device_slot_interface(mconfig, *this)
{
}

a2bus_slot_device::a2bus_slot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, type, name, tag, owner, clock),
		device_slot_interface(mconfig, *this)
{
}

void a2bus_slot_device::static_set_a2bus_slot(device_t &device, const char *tag, const char *slottag)
{
	a2bus_slot_device &a2bus_card = dynamic_cast<a2bus_slot_device &>(device);
	a2bus_card.m_a2bus_tag = tag;
	a2bus_card.m_a2bus_slottag = slottag;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_slot_device::device_start()
{
	device_a2bus_card_interface *dev = dynamic_cast<device_a2bus_card_interface *>(get_card_device());

	if (dev) device_a2bus_card_interface::static_set_a2bus_tag(*dev, m_a2bus_tag, m_a2bus_slottag);
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS = &device_creator<a2bus_device>;

void a2bus_device::static_set_cputag(device_t &device, const char *tag)
{
	a2bus_device &a2bus = downcast<a2bus_device &>(device);
	a2bus.m_cputag = tag;
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void a2bus_device::device_config_complete()
{
	// inherit a copy of the static data
	const a2bus_interface *intf = reinterpret_cast<const a2bus_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<a2bus_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_irq_cb, 0, sizeof(m_out_irq_cb));
		memset(&m_out_nmi_cb, 0, sizeof(m_out_nmi_cb));
		memset(&m_out_inh_cb, 0, sizeof(m_out_inh_cb));
	}
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a2bus_device - constructor
//-------------------------------------------------

a2bus_device::a2bus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, A2BUS, "Apple II Bus", tag, owner, clock)
{
}

a2bus_device::a2bus_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, type, name, tag, owner, clock)
{
}
//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_device::device_start()
{
	m_maincpu = machine().device<cpu_device>(m_cputag);

	// resolve callbacks
	m_out_irq_func.resolve(m_out_irq_cb, *this);
	m_out_nmi_func.resolve(m_out_nmi_cb, *this);
	m_out_inh_func.resolve(m_out_inh_cb, *this);

	// clear slots
	for (int i = 0; i < 8; i++)
	{
		m_device_list[i] = NULL;
	}
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void a2bus_device::device_reset()
{
}

device_a2bus_card_interface *a2bus_device::get_a2bus_card(int slot)
{
	if (slot < 0)
	{
		return NULL;
	}

	return m_device_list[slot];
}

void a2bus_device::add_a2bus_card(int slot, device_a2bus_card_interface *card)
{
	m_device_list[slot] = card;
}

void a2bus_device::set_irq_line(int state)
{
	m_out_irq_func(state);
}

void a2bus_device::set_nmi_line(int state)
{
	m_out_nmi_func(state);
}

void a2bus_device::set_inh_slotnum(int slot)
{
	m_out_inh_func(slot);
}

// interrupt request from a2bus card
WRITE_LINE_MEMBER( a2bus_device::irq_w ) { m_out_irq_func(state); }
WRITE_LINE_MEMBER( a2bus_device::nmi_w ) { m_out_nmi_func(state); }

//**************************************************************************
//  DEVICE CONFIG A2BUS CARD INTERFACE
//**************************************************************************


//**************************************************************************
//  DEVICE A2BUS CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_a2bus_card_interface - constructor
//-------------------------------------------------

device_a2bus_card_interface::device_a2bus_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_a2bus(NULL),
		m_a2bus_tag(NULL)
{
}


//-------------------------------------------------
//  ~device_a2bus_card_interface - destructor
//-------------------------------------------------

device_a2bus_card_interface::~device_a2bus_card_interface()
{
}

void device_a2bus_card_interface::static_set_a2bus_tag(device_t &device, const char *tag, const char *slottag)
{
	device_a2bus_card_interface &a2bus_card = dynamic_cast<device_a2bus_card_interface &>(device);
	a2bus_card.m_a2bus_tag = tag;
	a2bus_card.m_a2bus_slottag = slottag;
}

void device_a2bus_card_interface::set_a2bus_device()
{
	// extract the slot number from the last digit of the slot tag
	int tlen = strlen(m_a2bus_slottag);

	m_slot = (m_a2bus_slottag[tlen-1] - '0');

	if (m_slot < 0 || m_slot > 7)
	{
		fatalerror("Slot %x out of range for Apple II Bus\n", m_slot);
	}

	m_a2bus = dynamic_cast<a2bus_device *>(device().machine().device(m_a2bus_tag));
	m_a2bus->add_a2bus_card(m_slot, this);
}
