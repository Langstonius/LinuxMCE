/*
$Id: ow_w1_dispatch.c,v 1.27 2010/12/27 12:56:39 alfille Exp $
W1 Announce -- daemon  for showing w1 busmasters using Avahi
Written 2008 Paul H Alfille
email: paul.alfille@gmail.com
Released under the GPLv2
Much thanks to Evgeniy Polyakov
This file itself  is amodestly modified version of w1d by Evgeniy Polyakov
*/

/*
 * 	w1d.c
 *
 * Copyright (c) 2004 Evgeniy Polyakov <johnpol@2ka.mipt.ru>
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <config.h>
#include "owfs_config.h"

#if OW_W1 && OW_MT

#include "ow_w1.h"
#include "ow_connection.h"

// write to an internal pipe (in another thread). Use a timepout in case that thread has terminated.
static GOOD_OR_BAD W1_write_pipe( FILE_DESCRIPTOR_OR_ERROR file_descriptor, struct netlink_parse * nlp )
{
	int size = nlp->nlm->nlmsg_len ;
	do {
		int select_value ;
		struct timeval tv = { Globals.timeout_w1, 0 } ;

		fd_set writeset ;
		FD_ZERO(&writeset) ;
		FD_SET(file_descriptor,&writeset) ;

		select_value = select(file_descriptor+1,NULL,&writeset,NULL,&tv) ;

		if ( select_value == -1 ) {
			if (errno != EINTR) {
				ERROR_CONNECT("Netlink dispatch error");
				return gbBAD ;
			}
			// repeat path for EINTR
		} else if ( select_value == 0 ) {
			LEVEL_DEBUG("Netlink dispatch timeout");
			return gbBAD;
		} else {
			int write_ret = write( file_descriptor, (const void *)nlp->nlm, size ) ;
			if (write_ret < 0 ) {
				ERROR_DEBUG("Netlink dispatch write error");
			} else if ( write_ret < size ) {
				LEVEL_DEBUG("Only able to write %d of %d bytes to pipe",write_ret, size);
				return gbBAD ;
			}
			return gbGOOD ;
		}
	} while (1) ;
}

// Get the w1 bus id from the nlm sequence number and dispatch to that bus
static void Dispatch_Packet( struct netlink_parse * nlp)
{
	int bus = NL_BUS(nlp->nlm->nlmsg_seq) ;
	struct connection_in * in ;

	if ( bus == 0 ) { // root w1 master message -- add and remove
		/* Need to run the add/remove in a separate thread so that netlink messages can still be parsed and CONNIN_RLOCK won't deadlock */
		pthread_t thread ;
		// make a copy for the new thread (which we will have to destroy)
		struct netlink_parse * nlp_copy = owmalloc( sizeof(struct netlink_parse) ) ;
		if ( nlp_copy == NULL ) {
			return ;
		}
		memcpy( nlp_copy, nlp, sizeof(struct netlink_parse) ) ;
		if ( pthread_create( &thread, DEFAULT_THREAD_ATTR, w1_master_command, (void *) nlp_copy ) == 0 ) {
			LEVEL_DEBUG("Sending this packet to root bus");
		} else {
			LEVEL_DEBUG("Thread creation problem");
		}
		return ;
	}

	CONNIN_RLOCK ;
	for ( in = Inbound_Control.head ; in != NO_CONNECTION ; in = in->next ) {
		//printf("Matching %d/%s/%s/%s/ to bus.%d %d/%s/%s/%s/\n",bus_zero,name,type,domain,now->index,now->busmode,now->master.tcp.name,now->master.tcp.type,now->master.tcp.domain);
		if ( in->busmode == bus_w1 && in->master.w1.id == bus ) {
			LEVEL_DEBUG("Sending this packet to w1_bus_master%d",bus);
			if ( GOOD( W1_write_pipe(in->master.w1.netlink_pipe[fd_pipe_write], nlp) ) ) {
				LEVEL_DEBUG("Sending this packet to w1_bus_master%d",bus);
			} else {
				LEVEL_DEBUG("Error sending w1_bus_master%d",bus);
			}
			CONNIN_RUNLOCK ;
			return ;
		}
	}
	CONNIN_RUNLOCK ;
	LEVEL_DEBUG("W1 netlink message for non-existent bus %d",bus);
}

// Infinite loop waiting for netlink packets, to be sent to internal pipes as appropriate
void * W1_Dispatch( void * v )
{
	(void) v ;

	DETACH_THREAD;

	w1_list_masters() ; // ask for master list

	while ( FILE_DESCRIPTOR_VALID( SOC(Inbound_Control.w1_monitor)->file_descriptor ) ) {
		struct netlink_parse nlp ;

		LEVEL_DEBUG("Dispatch loop");
		if ( Netlink_Parse_Get( &nlp ) == 0 ) {
			Dispatch_Packet( &nlp ) ;
			Netlink_Parse_Destroy(&nlp) ;
		}
	}
	LEVEL_DEBUG("Normal exit.");
	return VOID_RETURN;
}

#endif /* OW_W1 && OW_MT */
