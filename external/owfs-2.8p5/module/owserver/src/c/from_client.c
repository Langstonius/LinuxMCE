/*
$Id: from_client.c,v 1.20 2010/06/14 19:37:14 alfille Exp $
    OW_HTML -- OWFS used for the web
    OW -- One-Wire filesystem

    Written 2004 Paul H Alfille

 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* owserver -- responds to requests over a network socket, and processes them on the 1-wire bus/
         Basic idea: control the 1-wire bus and answer queries over a network socket
         Clients can be owperl, owfs, owhttpd, etc...
         Clients can be local or remote
                 Eventually will also allow bounce servers.

         syntax:
                 owserver
                 -u (usb)
                 -d /dev/ttyS1 (serial)
                 -p tcp port
                 e.g. 3001 or 10.183.180.101:3001 or /tmp/1wire
*/

#include "owserver.h"

/* read from client, free return pointer if not Null */
int FromClient(struct handlerdata *hd)
{
	BYTE *msg;
	ssize_t trueload;
	size_t actual_read ;
	struct timeval tv = { Globals.timeout_server, 0, };
	//printf("FromClient\n");

	/* Clear return structure */
	memset(&hd->sp, 0, sizeof(struct serverpackage));

	/* read header */
	tcp_read(hd->file_descriptor, (BYTE *) &hd->sm, sizeof(struct server_msg), &tv, &actual_read) ;
	if (actual_read != sizeof(struct server_msg)) {
		hd->sm.type = msg_error;
		return -EIO;
	}

	/* translate endian state */
	hd->sm.version = ntohl(hd->sm.version);
	hd->sm.payload = ntohl(hd->sm.payload);
	hd->sm.type = ntohl(hd->sm.type);
	hd->sm.control_flags = ntohl(hd->sm.control_flags);
	hd->sm.size = ntohl(hd->sm.size);
	hd->sm.offset = ntohl(hd->sm.offset);

	LEVEL_DEBUG("FromClient payload=%d size=%d type=%d sg=0x%X offset=%d", hd->sm.payload, hd->sm.size, hd->sm.type, hd->sm.control_flags, hd->sm.offset);

	/* figure out length of rest of message: payload plus tokens */
	trueload = hd->sm.payload;
	if (isServermessage(hd->sm.version)) {
		trueload += sizeof(union antiloop) * Servertokens(hd->sm.version);
		LEVEL_DEBUG("FromClient (servermessage) payload=%d nrtokens=%d trueload=%d size=%d type=%d controlflags=0x%X offset=%d", hd->sm.payload, Servertokens(hd->sm.version), trueload, hd->sm.size, hd->sm.type, hd->sm.control_flags, hd->sm.offset);
	} else {
		LEVEL_DEBUG("FromClient (no servermessage) payload=%d size=%d type=%d controlflags=0x%X offset=%d", hd->sm.payload, hd->sm.size, hd->sm.type, hd->sm.control_flags, hd->sm.offset);
	}
	if (trueload == 0) {
		return 0;
	}

	/* valid size? */
	if ((hd->sm.payload < 0) || (trueload > MAX_OWSERVER_PROTOCOL_PACKET_SIZE)) {
		hd->sm.type = msg_error;
		return -EMSGSIZE;
	}

	/* Can allocate space? */
	if ((msg = owmalloc(trueload)) == NULL) {	/* create a buffer */
		hd->sm.type = msg_error;
		return -ENOMEM;
	}

	/* read in data */
	tcp_read(hd->file_descriptor, msg, trueload, &tv, &actual_read) ;
	if ((ssize_t)actual_read != trueload) {	/* read in the expected data */
		hd->sm.type = msg_error;
		owfree(msg);
		return -EIO;
	}

	/* path has null termination? */
	if (hd->sm.payload) {
		int pathlen;
		if (memchr(msg, 0, (size_t) hd->sm.payload) == NULL) {
			hd->sm.type = msg_error;
			owfree(msg);
			return -EINVAL;
		}
		pathlen = strlen( (char *) msg) + 1;
		hd->sp.data = & msg[pathlen];
		hd->sp.datasize = hd->sm.payload - pathlen;
	} else {
		hd->sp.data = NULL;
		hd->sp.datasize = 0;
	}

	if (isServermessage(hd->sm.version)) {	/* make sure no loop */
		size_t i;
		BYTE *p = &msg[hd->sm.payload];	// end of normal buffer
		hd->sp.tokenstring = p;
		hd->sp.tokens = Servertokens(hd->sm.version);
		for (i = 0; i < hd->sp.tokens; ++i, p += sizeof(union antiloop)) {
			if (memcmp(p, &(Globals.Token), sizeof(union antiloop)) == 0) {
				owfree(msg);
				hd->sm.type = msg_error;
				LEVEL_CALL("owserver loop suppression");
				return -ELOOP;
			}
		}
	}
	hd->sp.path = (char *) msg;
	return 0;
}
