/*
$Id: ow_com_close.c,v 1.1 2010/12/22 02:27:09 alfille Exp $
    OWFS -- One-Wire filesystem
    OWHTTPD -- One-Wire Web Server
    Written 2003 Paul H Alfille
	email: palfille@earthlink.net
	Released under the GPL
	See the header file: ow.h for full attribution
	1wire/iButton system from Dallas Semiconductor
*/

#include <config.h>
#include "owfs_config.h"
#include "ow.h"
#include "ow_connection.h"

#ifdef HAVE_LINUX_LIMITS_H
#include <linux/limits.h>
#endif

void COM_close(struct connection_in *connection)
{
	if (connection == NO_CONNECTION) {
		LEVEL_DEBUG("Attempt to close a NULL device");
		return ;
	}

	switch ( SOC(connection)->type ) {
		case ct_unknown:
		case ct_none:
		case ct_usb:
			LEVEL_DEBUG("ERROR!!! ----------- ERROR!");
			return ;
		case ct_telnet:
		case ct_tcp:
			break ;
		case ct_i2c:
		case ct_netlink:
			LEVEL_DEBUG("Unimplemented!!!");
			return ;
		case ct_serial:
			break ;
	}

	switch ( SOC(connection)->state ) {
		case cs_deflowered:
			Test_and_Close( &( SOC(connection)->file_descriptor) ) ;
			break ;
		case cs_virgin:
			break ;
	}
}
