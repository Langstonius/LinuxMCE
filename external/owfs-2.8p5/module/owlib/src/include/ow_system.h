/*
$Id: ow_system.h,v 1.11 2009/12/12 02:10:16 alfille Exp $
    OWFS -- One-Wire filesystem
    OWHTTPD -- One-Wire Web Server
    Written 2003 Paul H Alfille
	email: palfille@earthlink.net
	Released under the GPL
	See the header file: ow.h for full attribution
	1wire/iButton system from Dallas Semiconductor
*/

#ifndef OW_SYSTEM_H
#define OW_SYSTEM_H

#ifndef OWFS_CONFIG_H
#error Please make sure owfs_config.h is included *before* this header file
#endif
#include "ow_standard.h"

/* -------- Structures ---------- */
DeviceHeader(sys_process);
DeviceHeader(sys_connections);
DeviceHeader(sys_configure);

#endif							/* OW_SYSTEM_H */
