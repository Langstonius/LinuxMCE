/*
$Id: owserver.c,v 1.126 2010/07/18 01:49:33 alfille Exp $
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

/* --- Prototypes ------------ */
static void ow_exit(int e);
static void SetupAntiloop(void);

static void ow_exit(int e)
{
    if (IS_MAINTHREAD) {
		LEVEL_DEBUG("ow_exit %d", e);
		LibClose();
	} else {
		LEVEL_DEBUG("ow_exit (not mainthread) %d", e);
	}
#ifdef __UCLIBC__
	/* Process never die on WRT54G router with uClibc if exit() is used */
	_exit(e);
#else
	exit(e);
#endif
}

static void exit_handler(int signo, siginfo_t * info, void *context)
{
	(void) context;
#if OW_MT
	if (info) {
		LEVEL_DEBUG
			("exit_handler: for signo=%d, errno %d, code %d, pid=%ld, self=%lu main=%lu",
			 signo, info->si_errno, info->si_code, (long int) info->si_pid, pthread_self(), main_threadid);
	} else {
		LEVEL_DEBUG("exit_handler: for signo=%d, self=%lu, main=%lu", signo, pthread_self(), main_threadid);
	}
	if (StateInfo.shutting_down) {
	  LEVEL_DEBUG("exit_handler: shutdown already in progress. signo=%d, self=%lu, main=%lu", signo, pthread_self(), main_threadid);
	}
	if (!StateInfo.shutting_down) {
		StateInfo.shutting_down = 1;

		if (info != NULL) {
			if (SI_FROMUSER(info)) {
				LEVEL_DEBUG("exit_handler: kill from user");
			}
			if (SI_FROMKERNEL(info)) {
				LEVEL_DEBUG("exit_handler: kill from kernel");
			}
		}
		if (!IS_MAINTHREAD) {
			LEVEL_DEBUG("exit_handler: kill mainthread %lu self=%lu signo=%d", main_threadid, pthread_self(), signo);
			pthread_kill(main_threadid, signo);
		} else {
			LEVEL_DEBUG("exit_handler: ignore signal, mainthread %lu self=%lu signo=%d", main_threadid, pthread_self(), signo);
		}
	}
#else
	(void) signo;
	(void) info;
	StateInfo.shutting_down = 1;
#endif
	return;
}

int main(int argc, char **argv)
{
	int c;

	/* Set up owlib */
	LibSetup(opt_server);

	/* grab our executable name */
	if (argc > 0) {
		Globals.progname = owstrdup(argv[0]);
	}

	while ((c = getopt_long(argc, argv, OWLIB_OPT, owopts_long, NULL)) != -1) {
		switch (c) {
		case 'V':
			fprintf(stderr, "%s version:\n\t" VERSION "\n", argv[0]);
			break;
		default:
			break;
		}
		if ( BAD( owopt(c, optarg) ) ) {
			ow_exit(0);			/* rest of message */
		}
	}

	/* non-option arguments */
	while (optind < argc) {
		ARG_Generic(argv[optind]);
		++optind;
	}

	if (Outbound_Control.active == 0) {
		ARG_Server(NULL);		// make the default assignment
	}


	set_exit_signal_handlers(exit_handler);
	set_signal_handlers(NULL);

	/* become a daemon if not told otherwise */
	if ( BAD(EnterBackground()) ) {
		ow_exit(1);
	}

	/* Set up adapters */
	if ( BAD(LibStart()) ) {
		ow_exit(1);
	}

#if OW_MT
	main_threadid = pthread_self();
	_MUTEX_INIT(persistence_mutex);
	LEVEL_DEBUG("main_threadid = %lu", (unsigned long int) main_threadid);
#endif
	/* Set up "Antiloop" -- a unique token */
	SetupAntiloop();
	ServerProcess(Handler);
	LEVEL_DEBUG("ServerProcess done");
#if OW_MT
	_MUTEX_DESTROY(persistence_mutex);
#endif
	ow_exit(0);
	return 0;
}

static void SetupAntiloop(void)
{
	struct tms t;
	Globals.Token.simple.pid = getpid();
	Globals.Token.simple.clock = times(&t);
}
