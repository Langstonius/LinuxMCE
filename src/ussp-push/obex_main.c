/*
 * UNrooted.net example code
 *
 * Most of these functions are just rips from the Affix Bluetooth project OBEX
 * programs.  There are comments in the code about where the functions came 
 * from and what if anything was changed.
 *
 * Throughout these functions I had to change the BTDEBUG and BTERROR macros to
 * printfs.  I could have just defined BTERROR and BTDEBUG, but I didn't for
 * some reason.. who knows.
 *
 * The original header comment specifying the copyright holder and the 
 * requirements of the GPL are pasted below.
 */

/*
 * Original comment header block:
 * ----------------------------------------------------------------------

   Affix - Bluetooth Protocol Stack for Linux
   Copyright (C) 2001 Nokia Corporation
   Original Author: Dmitry Kasatkin <dmitry.kasatkin@nokia.com>

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

   $Id: obex_client.c,v 1.22 2002/06/25 10:16:49 kds Exp $

   OBEX client lib 

   Fixes:	Dmitry Kasatkin <dmitry.kasatkin@nokia.com>

 * ----------------------------------------------------------------------
 * End original comment block
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <glib.h>

#include <openobex/obex.h>

#include "obex_socket.h"

#define BT_SERVICE "OBEX"
#define OBEX_PUSH        5


/*
 * This struct came from the affix/include/affix/obex.h file, it holds all the
 * information for a client connection like what operation is currently being
 * carried out and the openobex connection handle.
 */

typedef struct client_context
{
    gboolean    serverdone;
    gboolean    clientdone;
    gchar       *get_name;  /* Name of last get-request */

    int     rsp;        /* error code */
    int     opcode;
    char        *arg;       /* response storage place */
    guint32     con_id;     /* connection ide */

    int     fd;
    gpointer    userdata;
} client_context_t;


/*
 * prototypes for local functions
 */

obex_t *__obex_connect(void *addr, int *err);
void obex_event(obex_t *handle, obex_object_t *object, gint mode, gint event, 
                 gint obex_cmd, gint obex_rsp);
int obex_disconnect(obex_t *handle);
guint8* easy_readfile(const char *filename, int *file_size);
gint get_filesize(const char *filename);
void request_done(obex_t *handle, obex_object_t *object, gint obex_cmd,
        gint obex_rsp);


/*
 * These two functions are from affix/profiles/obex/obex_io.c
 */

gint get_filesize(const char *filename)
{
    struct stat stats;

    stat(filename, &stats);
    return (gint) stats.st_size;
}

guint8* easy_readfile(const char *filename, int *file_size)
{
    int actual;
    int fd;
    guint8 *buf;


    fd = open(filename, O_RDONLY, 0);
    if (fd == -1) {
        return NULL;
    }
    *file_size = get_filesize(filename);
    printf("name=%s, size=%d\n", filename, *file_size);
    if(! (buf = g_malloc(*file_size)) ) {
        return NULL;
    }

    actual = read(fd, buf, *file_size);
    close(fd);

    *file_size = actual;
    return buf;
}


/*
 * This function comes from affix/profiles/obex/obex_client.c .. All I changed 
 * was a BTERROR() macro.  The OBEX_HandleInput() calls inside the loop should
 * result in request_done() getting called eventually.  request_done() sets 
 * the clientdone flag which releases the function from its loop
 */

int handle_response(obex_t *handle, char *service)
{
    int         err = 0;
    client_context_t    *gt = OBEX_GetUserData(handle);

    gt->clientdone = FALSE;
    while(!gt->clientdone ) {
        if( (err = OBEX_HandleInput(handle, 1)) < 0) {
            printf("Error while doing OBEX_HandleInput()");
            break;
        }
        err = (gt->rsp == OBEX_RSP_SUCCESS)?0:gt->rsp;
    }
    return err;
}


/*
 * This function comes from affix/profiles/obex/obex_client.c ... I just 
 * trimmed out handling of other transport styles.. that stuff was using some
 * globals and I didn't want to pull them in.
 */

int obex_disconnect(obex_t *handle)
{
    int         err;
    obex_object_t       *oo;    // OBEX Object
    client_context_t    *gt = OBEX_GetUserData(handle);

    oo = OBEX_ObjectNew(handle, OBEX_CMD_DISCONNECT);
    err = OBEX_Request(handle, oo);
    if( err )
        return err;
    handle_response(handle, BT_SERVICE);

    cobex_close(gt->userdata);//ctrans
    close(gt->fd);

    return 0;
}


/*
 * This function came from the affix/profiles/obex/obex_client.c file.
 * Initially it did some checks to see what type of link it was working over,
 * supporting a few different transports it looked like.  But the logic around
 * choosing what to do relied on a couple of global variables, so I just pulled
 * the decision paths I didn't want and left the ones I did.
 */

obex_t *__obex_connect(void *addr, int *err)
{
	obex_t			*handle;
	obex_object_t		*oo;	// OBEX Object
    obex_headerdata_t	hv;
    client_context_t	*gt;
    obex_ctrans_t 		custfunc;


    gt = malloc(sizeof(client_context_t));
    if( gt == NULL )
        return NULL;

    memset(gt, 0, sizeof(client_context_t));

    gt->userdata = cobex_open(addr);
    if( gt->userdata == NULL ) {
        printf("cobex_open() failed");
        *err = -1;
        return NULL;
    }
    if(! (handle = OBEX_Init(OBEX_TRANS_CUST, obex_event, 0))) {
        printf( "OBEX_Init failed:%s", strerror(errno));
        close(gt->fd);
        *err = -1;
        return NULL;
    }

    custfunc.customdata = gt->userdata;
    custfunc.connect = cobex_connect;
    custfunc.disconnect = cobex_disconnect;
    custfunc.write = cobex_write;
    custfunc.handleinput = cobex_handle_input;
    custfunc.listen = cobex_connect;	// Listen and connect is 100% same on cable

    if(OBEX_RegisterCTransport(handle, &custfunc) < 0) {
        printf("Custom transport callback-registration failed");
        close(gt->fd);
        *err = -1;
        return NULL;
    }

    printf( "Registered transport\n" );


    OBEX_SetUserData(handle, gt);

    printf( "\nset user data\n" );
    // create new object
    oo = OBEX_ObjectNew(handle, OBEX_CMD_CONNECT);
    printf( "\ncreated new objext\n" );
    *err = OBEX_Request(handle, oo);
    printf( "\nstarted a new request\n" );
    if( *err )
        goto exit;
    *err = handle_response(handle, BT_SERVICE);
    printf("\nConnection return code: %d, id: %d\n", *err, gt->con_id);
    if( *err )
        goto exit;
    printf("Connection established\n");
    return handle;
exit:
	obex_disconnect(handle);
	return NULL;	
}


/*
 * These next two functions come from affix/profiles/obex/obex_client.c
 * All they do are set a few flags in the structs here or disconnect on error.
 * The obex_event() function is called by the obex library when it has an event
 * to deliver to us.. as simple as this is this means just setting that
 * clientdone flag in the user data struct.  Yea... really, that's it.
 */

void request_done(obex_t *handle, obex_object_t *object, gint obex_cmd,
                  gint obex_rsp)  
{           
    client_context_t *gt = OBEX_GetUserData(handle);

    printf("Command (%02x) has now finished, rsp: %02x", obex_cmd, obex_rsp);

    switch (obex_cmd) {
        case OBEX_CMD_DISCONNECT:
            printf("Disconnect done!");
            OBEX_TransportDisconnect(handle);
            gt->clientdone = TRUE;
            break;
        case OBEX_CMD_CONNECT:
            printf("Connected!\n");
            //connect_client(handle, object, obex_rsp);
            gt->clientdone = TRUE;
            break;
        case OBEX_CMD_GET:
            printf( "\n\n\n***warning, getclient commented out\n" );
            //get_client(handle, object, obex_rsp);
            gt->clientdone = TRUE;
            break;
        case OBEX_CMD_PUT:
            //put_client(handle, object, obex_rsp);
            gt->clientdone = TRUE;
            break;
        case OBEX_CMD_SETPATH:
            printf( "\n\n\n***warning, setpath_cleitn commented out\n" );
            //setpath_client(handle, object, obex_rsp);
            gt->clientdone = TRUE;
            break;

        default:
            printf("Command (%02x) has now finished", obex_cmd);
            break;
    }
}

void obex_event(obex_t *handle, obex_object_t *object, gint mode, gint event, 
                 gint obex_cmd, gint obex_rsp)
{
    switch (event)  {
        case OBEX_EV_PROGRESS:
            printf("Made some progress...\n");
            break;

        case OBEX_EV_ABORT:
            printf("Request aborted!\n");
            break;

        case OBEX_EV_REQDONE:
            printf("reqdone\n" );
            request_done(handle, object, obex_cmd, obex_rsp);
            // server_done(handle, object, obex_cmd, obex_rsp);
            break;

        case OBEX_EV_REQHINT:
            /* Accept any command. Not rellay good, but this is a test-program :
            * ) */
            // OBEX_ObjectSetRsp(object, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS);
            printf("setresp\n" );
            break;

        case OBEX_EV_REQ:
            //server_request(handle, object, event, obex_cmd);
            printf("server request\n" );
            break;

        case OBEX_EV_LINKERR:
            OBEX_TransportDisconnect(handle);
            printf("Link broken!\n");
            break;

        case OBEX_EV_PARSEERR:
            printf("Parse error!\n");
            break;

        default:
            printf("Unknown event %02x!\n", event);
            break;
    }
}


/*
 * This function also comes directly from affix/profiles/obex/obex_client.c
 * It calls __obex_connect() to connect to the device, and then uses the
 * OpenOBEX call interface to form a PUT request to send the file to the 
 * device. Originally this used the basename() of path as the name to put as,
 * but I added a remote parameter so that I didn't have to rename files before
 * moving them over.  Essentially nothing is different.
 */

int obex_push(void *addr, char *path, char *remote)
{
    int         err;
    obex_object_t       *oo;    // OBEX Object
    obex_headerdata_t   hv;
    obex_t          *handle;
    guint8          *buf;
    int         file_size;
    gchar           *namebuf;
    int         name_len;
    char            *name, *pathc;
    client_context_t    *gt;

    pathc = strdup(path);
    name = remote;

    name_len = (strlen(name)+1)<<1;
    if( (namebuf = g_malloc(name_len)) )    {
        OBEX_CharToUnicode(namebuf, name, name_len);
    }

    buf = easy_readfile(path, &file_size);
    if(buf == NULL) {
        printf("Can't find file %s\n", name);
        return -ENOENT;
    }

    handle = __obex_connect(addr, &err);
    if( handle == NULL ) {
        printf("unable to connect to the server\n");
        g_free(buf);
        return err;
    }
    printf("connected to server\n");
    gt = OBEX_GetUserData(handle);
    gt->opcode = OBEX_PUSH;


    printf("Sending file: %s, path: %s, size: %d\n", name, path, file_size);
    oo = OBEX_ObjectNew(handle, OBEX_CMD_PUT);
    hv.bs = namebuf;
    OBEX_ObjectAddHeader(handle, oo, OBEX_HDR_NAME, hv, name_len, 0);
    hv.bq4 = file_size;
    OBEX_ObjectAddHeader(handle, oo, OBEX_HDR_LENGTH, hv, sizeof(guint32), 0);
    hv.bs = buf;
    OBEX_ObjectAddHeader(handle, oo, OBEX_HDR_BODY, hv, file_size, 0);

    err = OBEX_Request(handle, oo);
    if( err )
        return err;
    err = handle_response(handle, BT_SERVICE);

    obex_disconnect(handle);
    g_free(buf);
    free(pathc);

    return err;
}


/*
 * That's all there is to it.  With it all setup like this all I have to do 
 * is call obex_push() to move a file.  Thank you Affix and OpenOBEX.  And of
 * course, Bluez.  Now I can show everyone I'm evil-l33t with a pretty GIR
 * background image on my phone.  PH33R!!!
 */

int main( int argc, char **argv )
{
    if ( argc != 4 ) {
        printf( "Usage:  %s <rfcomm tty device> <local file> <remote name>\n",
                argv[0] );
        return( -1 );
    }

    printf( "pushing file %s\n", argv[2] );
    if ( obex_push( (void *)argv[1], argv[2], argv[3] ) != 0 ) {
        printf( "error\n" );
        return( -1 );
    }

    printf( "pushed!!\n" );
    return( 0 );
}
