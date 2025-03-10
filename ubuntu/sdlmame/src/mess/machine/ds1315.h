/*********************************************************************

    ds1315.h

    Dallas Semiconductor's Phantom Time Chip DS1315.

    by tim lindner, November 2001.

*********************************************************************/

#ifndef __DS1315_H__
#define __DS1315_H__

#include "emu.h"


/***************************************************************************
    MACROS
***************************************************************************/

class ds1315_device : public device_t
{
public:
	ds1315_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~ds1315_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
private:
	// internal state
	void *m_token;
};

extern const device_type DS1315;




/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_DS1315_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, DS1315, 0)


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

DECLARE_READ8_DEVICE_HANDLER( ds1315_r_0 );
DECLARE_READ8_DEVICE_HANDLER( ds1315_r_1 );
DECLARE_READ8_DEVICE_HANDLER( ds1315_r_data );
DECLARE_WRITE8_DEVICE_HANDLER( ds1315_w_data );

#endif /* __DS1315_H__ */
