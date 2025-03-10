/***************************************************************************

  nubus.h - NuBus bus and card emulation

  by R. Belmont, based heavily on Miodrag Milanovic's ISA8/16 implementation

***************************************************************************/

#pragma once

#ifndef __NUBUS_H__
#define __NUBUS_H__

#include "emu.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_NUBUS_BUS_ADD(_tag, _cputag, _config) \
	MCFG_DEVICE_ADD(_tag, NUBUS, 0) \
	MCFG_DEVICE_CONFIG(_config) \
	nubus_device::static_set_cputag(*device, _cputag); \

#define MCFG_NUBUS_SLOT_ADD(_nbtag, _tag, _slot_intf, _def_slot, _def_inp) \
	MCFG_DEVICE_ADD(_tag, NUBUS_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, false) \
	nubus_slot_device::static_set_nubus_slot(*device, _nbtag, _tag); \

#define MCFG_NUBUS_SLOT_REMOVE(_tag)    \
	MCFG_DEVICE_REMOVE(_tag)

#define MCFG_NUBUS_ONBOARD_ADD(_nbtag, _tag, _dev_type, _def_inp) \
	MCFG_DEVICE_ADD(_tag, _dev_type, 0) \
	MCFG_DEVICE_INPUT_DEFAULTS(_def_inp) \
	device_nubus_card_interface::static_set_nubus_tag(*device, _nbtag, _tag);

#define MCFG_NUBUS_BUS_REMOVE(_tag) \
	MCFG_DEVICE_REMOVE(_tag)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class nubus_device;

class nubus_slot_device : public device_t,
							public device_slot_interface
{
public:
	// construction/destruction
	nubus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	nubus_slot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();

	// inline configuration
	static void static_set_nubus_slot(device_t &device, const char *tag, const char *slottag);
protected:
	// configuration
	const char *m_nubus_tag, *m_nubus_slottag;
};

// device type definition
extern const device_type NUBUS_SLOT;

// ======================> nbbus_interface

struct nbbus_interface
{
	devcb_write_line    m_out_irq9_cb;
	devcb_write_line    m_out_irqa_cb;
	devcb_write_line    m_out_irqb_cb;
	devcb_write_line    m_out_irqc_cb;
	devcb_write_line    m_out_irqd_cb;
	devcb_write_line    m_out_irqe_cb;
};

class device_nubus_card_interface;
// ======================> nubus_device
class nubus_device : public device_t,
					public nbbus_interface
{
public:
	// construction/destruction
	nubus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	nubus_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	// inline configuration
	static void static_set_cputag(device_t &device, const char *tag);

	void add_nubus_card(device_nubus_card_interface *card);
	void install_device(offs_t start, offs_t end, read8_delegate rhandler, write8_delegate whandler, UINT32 mask=0xffffffff);
	void install_device(offs_t start, offs_t end, read16_delegate rhandler, write16_delegate whandler, UINT32 mask=0xffffffff);
	void install_device(offs_t start, offs_t end, read32_delegate rhandler, write32_delegate whandler, UINT32 mask=0xffffffff);
	void install_readonly_device(offs_t start, offs_t end, read32_delegate rhandler, UINT32 mask=0xffffffff);
	void install_writeonly_device(offs_t start, offs_t end, write32_delegate whandler, UINT32 mask=0xffffffff);
	void install_bank(offs_t start, offs_t end, offs_t mask, offs_t mirror, const char *tag, UINT8 *data);
	void set_irq_line(int slot, int state);

	DECLARE_WRITE_LINE_MEMBER( irq9_w );
	DECLARE_WRITE_LINE_MEMBER( irqa_w );
	DECLARE_WRITE_LINE_MEMBER( irqb_w );
	DECLARE_WRITE_LINE_MEMBER( irqc_w );
	DECLARE_WRITE_LINE_MEMBER( irqd_w );
	DECLARE_WRITE_LINE_MEMBER( irqe_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete();

	// internal state
	cpu_device   *m_maincpu;

	devcb_resolved_write_line   m_out_irq9_func;
	devcb_resolved_write_line   m_out_irqa_func;
	devcb_resolved_write_line   m_out_irqb_func;
	devcb_resolved_write_line   m_out_irqc_func;
	devcb_resolved_write_line   m_out_irqd_func;
	devcb_resolved_write_line   m_out_irqe_func;

	simple_list<device_nubus_card_interface> m_device_list;
	const char *m_cputag;
};


// device type definition
extern const device_type NUBUS;

// ======================> device_nubus_card_interface

// class representing interface-specific live nubus card
class device_nubus_card_interface : public device_slot_card_interface
{
	friend class nubus_device;
public:
	// construction/destruction
	device_nubus_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_nubus_card_interface();

	device_nubus_card_interface *next() const { return m_next; }

	void set_nubus_device();

	// helper functions for card devices
	void install_declaration_rom(device_t *dev, const char *romregion, bool mirror_all_mb = false, bool reverse_rom = false);
	void install_bank(offs_t start, offs_t end, offs_t mask, offs_t mirror, const char *tag, UINT8 *data);

	UINT32 get_slotspace() { return 0xf0000000 | (m_slot<<24); }
	UINT32 get_super_slotspace() { return m_slot<<28; }

	void raise_slot_irq() { m_nubus->set_irq_line(m_slot, ASSERT_LINE); }
	void lower_slot_irq() { m_nubus->set_irq_line(m_slot, CLEAR_LINE); }

	// inline configuration
	static void static_set_nubus_tag(device_t &device, const char *tag, const char *slottag);
public:
	nubus_device  *m_nubus;
	const char *m_nubus_tag, *m_nubus_slottag;
	int m_slot;
	device_nubus_card_interface *m_next;
};

#endif  /* __NUBUS_H__ */
