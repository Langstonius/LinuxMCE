/*!
 * \file        sccp_socket.c
 * \brief       SCCP Socket Class
 * \author      Sergio Chersovani <mlists [at] c-net.it>
 * \note                Reworked, but based on chan_sccp code.
 *              The original chan_sccp driver that was made by Zozo which itself was derived from the chan_skinny driver.
 *              Modified by Jan Czmok and Julien Goodwin
 * \note                This program is free software and may be modified and distributed under the terms of the GNU Public License.
 *              See the LICENSE file at the top of the source tree.
 *
 * $Date: 2015-10-19 09:42:29 +0000 (Mon, 19 Oct 2015) $
 * $Revision: 6266 $
 */

#include <netinet/in.h>
#include <config.h>
#include "common.h"
#include "sccp_socket.h"
#include "sccp_device.h"
#include "sccp_utils.h"
#include "sccp_cli.h"

SCCP_FILE_VERSION(__FILE__, "$Revision: 6266 $");
#ifndef CS_USE_POLL_COMPAT
#include <poll.h>
#include <sys/poll.h>
#else
#define AST_POLL_COMPAT 1
#include <asterisk/poll-compat.h>
#endif
#ifdef pbx_poll
#define sccp_socket_poll pbx_poll
#else
#define sccp_socket_poll poll
#endif
#include <asterisk/cli.h>
sccp_session_t *sccp_session_findByDevice(const sccp_device_t * device);

/* arbitrary values */
#define SOCKET_TIMEOUT_SEC 7											/* timeout after seven seconds when trying to read/write from/to a socket */
#define SOCKET_TIMEOUT_MILLISEC 0										/* "       "     0 milli seconds "    "    */
#define SOCKET_KEEPALIVE_IDLE GLOB(keepalive)									/* The time (in seconds) the connection needs to remain idle before TCP starts sending keepalive probes */
#define SOCKET_KEEPALIVE_INTVL 5										/* The time (in seconds) between individual keepalive probes, once we have started to probe. */
#define SOCKET_KEEPALIVE_CNT 5											/* The maximum number of keepalive probes TCP should send before dropping the connection. */
#define SOCKET_LINGER_ONOFF 1											/* linger=on */
#define SOCKET_LINGER_WAIT 0											/* but wait 0 milliseconds before closing socket and discard all outboung messages */
#define SOCKET_RCVBUF SCCP_MAX_PACKET										/* SO_RCVBUF */
#define SOCKET_SNDBUF SCCP_MAX_PACKET * 5									/* SO_SNDBUG */

#define READ_RETRIES 6												/* number of read retries */
#define READ_BACKOFF 150											/* backoff time in millisecs, doubled every read retry (150+300+600+1200+2400+4800 = 9450 millisecs = 9.5 sec)*/
#define WRITE_RETRIES 6												/* number of write retries */
#define WRITE_BACKOFF 150											/* backoff time in millisecs, doubled every write retry (150+300+600+1200+2400+4800 = 9450 millisecs = 9.5 sec) */

#define SESSION_DEVICE_CLEANUP_TIME 10										/* wait time before destroying a device on thread exit */
#define KEEPALIVE_ADDITIONAL_PERCENT 10										/* extra time allowed for device keepalive overrun (percentage of GLOB(keepalive)) */

/* Lock Macro for Sessions */
#define sccp_session_lock(x)			pbx_mutex_lock(&x->lock)
#define sccp_session_unlock(x)			pbx_mutex_unlock(&x->lock)
#define sccp_session_trylock(x)			pbx_mutex_trylock(&x->lock)
/* */

void destroy_session(sccp_session_t * s, uint8_t cleanupTime);
void sccp_session_close(sccp_session_t * s);
void sccp_socket_device_thread_exit(void *session);
void *sccp_socket_device_thread(void *session);

/*!
 * \brief SCCP Session Structure
 * \note This contains the current session the phone is in
 */
struct sccp_session {
	time_t lastKeepAlive;											/*!< Last KeepAlive Time */
	SCCP_RWLIST_ENTRY (sccp_session_t) list;								/*!< Linked List Entry for this Session */
	sccp_device_t *device;											/*!< Associated Device */
	struct pollfd fds[1];											/*!< File Descriptor */
	struct sockaddr_storage sin;										/*!< Incoming Socket Address */
	uint32_t protocolType;
	volatile boolean_t session_stop;										/*!< Signal Session Stop */
	sccp_mutex_t write_lock;										/*!< Prevent multiple threads writing to the socket at the same time */
	sccp_mutex_t lock;											/*!< Asterisk: Lock Me Up and Tie me Down */
	pthread_t session_thread;										/*!< Session Thread */
	struct sockaddr_storage ourip;										/*!< Our IP is for rtp use */
	struct sockaddr_storage ourIPv4;
	char designator[32];
};														/*!< SCCP Session Structure */

union sockaddr_union {
	struct sockaddr sa;
	struct sockaddr_storage ss;
	struct sockaddr_in sin;
	struct sockaddr_in6 sin6;
};

boolean_t sccp_socket_is_IPv4(const struct sockaddr_storage *sockAddrStorage)
{
	return (sockAddrStorage->ss_family == AF_INET) ? TRUE : FALSE;
}

boolean_t sccp_socket_is_IPv6(const struct sockaddr_storage *sockAddrStorage)
{
	return (sockAddrStorage->ss_family == AF_INET6) ? TRUE : FALSE;
}

uint16_t sccp_socket_getPort(const struct sockaddr_storage * sockAddrStorage)
{
	if (sccp_socket_is_IPv4(sockAddrStorage)) {
		return ntohs(((struct sockaddr_in *) sockAddrStorage)->sin_port);
	} else if (sccp_socket_is_IPv6(sockAddrStorage)) {
		return ntohs(((struct sockaddr_in6 *) sockAddrStorage)->sin6_port);
	}
	return 0;
}

void sccp_socket_setPort(const struct sockaddr_storage *sockAddrStorage, uint16_t port)
{
	if (sccp_socket_is_IPv4(sockAddrStorage)) {
		((struct sockaddr_in *) sockAddrStorage)->sin_port = htons(port);
	} else if (sccp_socket_is_IPv6(sockAddrStorage)) {
		((struct sockaddr_in6 *) sockAddrStorage)->sin6_port = htons(port);
	}
}

int sccp_socket_is_any_addr(const struct sockaddr_storage *sockAddrStorage)
{
	union sockaddr_union tmp_addr = {
		.ss = *sockAddrStorage,
	};

	return (sccp_socket_is_IPv4(sockAddrStorage) && (tmp_addr.sin.sin_addr.s_addr == INADDR_ANY)) || (sccp_socket_is_IPv6(sockAddrStorage) && IN6_IS_ADDR_UNSPECIFIED(&tmp_addr.sin6.sin6_addr));
}

boolean_t sccp_socket_getExternalAddr(struct sockaddr_storage *sockAddrStorage)
{
	//! \todo handle IPv4 / IPV6 family ?
	if (sccp_socket_is_any_addr(&GLOB(externip))) {
		sccp_log(DEBUGCAT_CORE) (VERBOSE_PREFIX_3 "SCCP: No externip set in sccp.conf. In case you are running your PBX on a seperate host behind a NATTED Firewall you need to set externip.\n");
		return FALSE;
	}
	memcpy(sockAddrStorage, &GLOB(externip), sizeof(struct sockaddr_storage));
	return TRUE;
}

boolean_t sccp_session_getOurIP(constSessionPtr session, struct sockaddr_storage * const sockAddrStorage, int family)
{
	if (session && sockAddrStorage) {
		if (!sccp_socket_is_any_addr(&session->ourip)) {
			switch (family) {
				case 0:
					memcpy(sockAddrStorage, &session->ourip, sizeof(struct sockaddr_storage));
					break;
				case AF_INET:
					((struct sockaddr_in *) sockAddrStorage)->sin_addr = ((struct sockaddr_in *) &session->ourip)->sin_addr;
					break;
				case AF_INET6:
					((struct sockaddr_in6 *) sockAddrStorage)->sin6_addr = ((struct sockaddr_in6 *) &session->ourip)->sin6_addr;
					break;
			}
			return TRUE;
		}
	}
	return FALSE;
}

boolean_t sccp_session_getSas(constSessionPtr session, struct sockaddr_storage * const sockAddrStorage)
{
	if (session && sockAddrStorage) {
		memcpy(sockAddrStorage, &session->sin, sizeof(struct sockaddr_storage));
		return TRUE;
	}
	return FALSE;
}

size_t sccp_socket_sizeof(const struct sockaddr_storage * sockAddrStorage)
{
	if (sccp_socket_is_IPv4(sockAddrStorage)) {
		return sizeof(struct sockaddr_in);
	} else if (sccp_socket_is_IPv6(sockAddrStorage)) {
		return sizeof(struct sockaddr_in6);
	}
	return 0;
}

static int sccp_socket_is_ipv6_link_local(const struct sockaddr_storage *sockAddrStorage)
{
	union sockaddr_union tmp_addr = {
		.ss = *sockAddrStorage,
	};
	return sccp_socket_is_IPv6(sockAddrStorage) && IN6_IS_ADDR_LINKLOCAL(&tmp_addr.sin6.sin6_addr);
}

boolean_t sccp_socket_is_mapped_IPv4(const struct sockaddr_storage *sockAddrStorage)
{
	if (sccp_socket_is_IPv6(sockAddrStorage)) {
		const struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) sockAddrStorage;

		return IN6_IS_ADDR_V4MAPPED(&sin6->sin6_addr);
	} else {
		return FALSE;
	}
}

boolean_t sccp_socket_ipv4_mapped(const struct sockaddr_storage *sockAddrStorage, struct sockaddr_storage *sockAddrStorage_mapped)
{
	const struct sockaddr_in6 *sin6;
	struct sockaddr_in sin4;

	if (!sccp_socket_is_IPv6(sockAddrStorage)) {
		return FALSE;
	}

	if (!sccp_socket_is_mapped_IPv4(sockAddrStorage)) {
		return FALSE;
	}

	sin6 = (const struct sockaddr_in6 *) sockAddrStorage;

	memset(&sin4, 0, sizeof(sin4));
	sin4.sin_family = AF_INET;
	sin4.sin_port = sin6->sin6_port;
	sin4.sin_addr.s_addr = ((uint32_t *) & sin6->sin6_addr)[3];

	memcpy(sockAddrStorage_mapped, &sin4, sizeof(sin4));
	return TRUE;
}

/*!
 * \brief
 * Compares the addresses of two ast_sockaddr structures.
 *
 * \retval -1 \a a is lexicographically smaller than \a b
 * \retval 0 \a a is equal to \a b
 * \retval 1 \a b is lexicographically smaller than \a a
 */
int sccp_socket_cmp_addr(const struct sockaddr_storage *a, const struct sockaddr_storage *b)
{
	//char *stra = ast_strdupa(sccp_socket_stringify_addr(a));
	//char *strb = ast_strdupa(sccp_socket_stringify_addr(b));

	const struct sockaddr_storage *a_tmp, *b_tmp;
	struct sockaddr_storage ipv4_mapped;
	size_t len_a = sccp_socket_sizeof(a);
	size_t len_b = sccp_socket_sizeof(b);
	int ret = -1;

	a_tmp = a;
	b_tmp = b;

	if (len_a != len_b) {
		if (sccp_socket_ipv4_mapped(a, &ipv4_mapped)) {
			a_tmp = &ipv4_mapped;
		} else if (sccp_socket_ipv4_mapped(b, &ipv4_mapped)) {
			b_tmp = &ipv4_mapped;
		}
	}
	if (len_a < len_b) {
		ret = -1;
		goto EXIT;
	} else if (len_a > len_b) {
		ret = 1;
		goto EXIT;
	}

	if (a_tmp->ss_family == b_tmp->ss_family) {
		if (a_tmp->ss_family == AF_INET) {
			ret = memcmp(&(((struct sockaddr_in *) a_tmp)->sin_addr), &(((struct sockaddr_in *) b_tmp)->sin_addr), sizeof(struct in_addr));
		} else {											// AF_INET6
			ret = memcmp(&(((struct sockaddr_in6 *) a_tmp)->sin6_addr), &(((struct sockaddr_in6 *) b_tmp)->sin6_addr), sizeof(struct in6_addr));
		}
	}
EXIT:
	//sccp_log(DEBUGCAT_HIGH)(VERBOSE_PREFIX_2 "SCCP: sccp_socket_cmp_addr(%s, %s) returning %d\n", stra, strb, ret);
	return ret;
}

/*!
 * \brief
 * Splits a string into its host and port components
 *
 * \param str       [in] The string to parse. May be modified by writing a NUL at the end of
 *                  the host part.
 * \param host      [out] Pointer to the host component within \a str.
 * \param port      [out] Pointer to the port component within \a str.
 * \param flags     If set to zero, a port MAY be present. If set to PARSE_PORT_IGNORE, a
 *                  port MAY be present but will be ignored. If set to PARSE_PORT_REQUIRE,
 *                  a port MUST be present. If set to PARSE_PORT_FORBID, a port MUST NOT
 *                  be present.
 *
 * \retval 1 Success
 * \retval 0 Failure
 */
int sccp_socket_split_hostport(char *str, char **host, char **port, int flags)
{
	char *s = str;
	char *orig_str = str;											/* Original string in case the port presence is incorrect. */
	char *host_end = NULL;											/* Delay terminating the host in case the port presence is incorrect. */

	sccp_log(DEBUGCAT_HIGH) (VERBOSE_PREFIX_4 "Splitting '%s' into...\n", str);
	*host = NULL;
	*port = NULL;
	if (*s == '[') {
		*host = ++s;
		for (; *s && *s != ']'; ++s) {
		}
		if (*s == ']') {
			host_end = s;
			++s;
		}
		if (*s == ':') {
			*port = s + 1;
		}
	} else {
		*host = s;
		for (; *s; ++s) {
			if (*s == ':') {
				if (*port) {
					*port = NULL;
					break;
				} else {
					*port = s;
				}
			}
		}
		if (*port) {
			host_end = *port;
			++*port;
		}
	}

	switch (flags & PARSE_PORT_MASK) {
		case PARSE_PORT_IGNORE:
			*port = NULL;
			break;
		case PARSE_PORT_REQUIRE:
			if (*port == NULL) {
				pbx_log(LOG_WARNING, "Port missing in %s\n", orig_str);
				return 0;
			}
			break;
		case PARSE_PORT_FORBID:
			if (*port != NULL) {
				pbx_log(LOG_WARNING, "Port disallowed in %s\n", orig_str);
				return 0;
			}
			break;
	}
	/* Can terminate the host string now if needed. */
	if (host_end) {
		*host_end = '\0';
	}
	sccp_log(DEBUGCAT_HIGH) (VERBOSE_PREFIX_4 "...host '%s' and port '%s'.\n", *host, *port ? *port : "");
	return 1;
}

AST_THREADSTORAGE(sccp_socket_stringify_buf);
char *sccp_socket_stringify_fmt(const struct sockaddr_storage *sockAddrStorage, int format)
{
	const struct sockaddr_storage *sockAddrStorage_tmp;
	char host[NI_MAXHOST] = "";
	char port[NI_MAXSERV] = "";
	struct ast_str *str;
	int e;
	static const size_t size = sizeof(host) - 1 + sizeof(port) - 1 + 4;

	if (!sockAddrStorage) {
		return "(null)";
	}

	if (!(str = ast_str_thread_get(&sccp_socket_stringify_buf, size))) {
		return "";
	}
	//if (sccp_socket_ipv4_mapped(sockAddrStorage, &sockAddrStorage_tmp_ipv4)) {
	//	struct sockaddr_storage sockAddrStorage_tmp_ipv4;
	//	sockAddrStorage_tmp = &sockAddrStorage_tmp_ipv4;
	//#if DEBUG
	//	sccp_log(0)("SCCP: ipv4 mapped in ipv6 address\n");
	//#endif
	//} else {
	sockAddrStorage_tmp = sockAddrStorage;
	//}

	if ((e = getnameinfo((struct sockaddr *) sockAddrStorage_tmp, sccp_socket_sizeof(sockAddrStorage_tmp), format & SCCP_SOCKADDR_STR_ADDR ? host : NULL, format & SCCP_SOCKADDR_STR_ADDR ? sizeof(host) : 0, format & SCCP_SOCKADDR_STR_PORT ? port : 0, format & SCCP_SOCKADDR_STR_PORT ? sizeof(port) : 0, NI_NUMERICHOST | NI_NUMERICSERV))) {
		sccp_log(DEBUGCAT_SOCKET) (VERBOSE_PREFIX_3 "SCCP: getnameinfo(): %s \n", gai_strerror(e));
		return "";
	}

	if ((format & SCCP_SOCKADDR_STR_REMOTE) == SCCP_SOCKADDR_STR_REMOTE) {
		char *p;

		if (sccp_socket_is_ipv6_link_local(sockAddrStorage_tmp) && (p = strchr(host, '%'))) {
			*p = '\0';
		}
	}
	switch ((format & SCCP_SOCKADDR_STR_FORMAT_MASK)) {
		case SCCP_SOCKADDR_STR_DEFAULT:
			ast_str_set(&str, 0, sockAddrStorage_tmp->ss_family == AF_INET6 ? "[%s]:%s" : "%s:%s", host, port);
			break;
		case SCCP_SOCKADDR_STR_ADDR:
			ast_str_set(&str, 0, "%s", host);
			break;
		case SCCP_SOCKADDR_STR_HOST:
			ast_str_set(&str, 0, sockAddrStorage_tmp->ss_family == AF_INET6 ? "[%s]" : "%s", host);
			break;
		case SCCP_SOCKADDR_STR_PORT:
			ast_str_set(&str, 0, "%s", port);
			break;
		default:
			ast_log(LOG_ERROR, "Invalid format\n");
			return "";
	}

	return ast_str_buffer(str);
}

/*!
 * \brief Exchange Socket Addres Information from them to us
 */
static int __sccp_socket_setOurAddressFromTheirs(const struct sockaddr_storage *them, struct sockaddr_storage *us)
{
	int sock;
	socklen_t slen;

	union sockaddr_union tmp_addr = {
		.ss = *them,
	};

	if (sccp_socket_is_IPv6(them)) {
		tmp_addr.sin6.sin6_port = htons(sccp_socket_getPort(&GLOB(bindaddr)));
		slen = sizeof(struct sockaddr_in6);
	} else if (sccp_socket_is_IPv4(them)) {
		tmp_addr.sin.sin_port = htons(sccp_socket_getPort(&GLOB(bindaddr)));
		slen = sizeof(struct sockaddr_in);
	} else {
		pbx_log(LOG_WARNING, "SCCP: getOurAddressfor Unspecified them format: %s\n", sccp_socket_stringify(them));
		return -1;
	}

	if ((sock = socket(tmp_addr.ss.ss_family, SOCK_DGRAM, 0)) < 0) {
		return -1;
	}

	if (connect(sock, &tmp_addr.sa, sizeof(tmp_addr))) {
		pbx_log(LOG_WARNING, "SCCP: getOurAddressfor Failed to connect to %s\n", sccp_socket_stringify(them));
		return -1;
	}
	if (getsockname(sock, &tmp_addr.sa, &slen)) {
		close(sock);
		return -1;
	}
	close(sock);
	memcpy(us, &tmp_addr, slen);
	return 0;
}

int sccp_session_setOurIP4Address(constSessionPtr session, const struct sockaddr_storage *addr)
{
	sccp_session_t * const s = (sccp_session_t * const)session;						/* discard const */
	if (s) {
		return __sccp_socket_setOurAddressFromTheirs(addr, &s->ourIPv4);
	}
	return -2;
}

static void __sccp_session_stopthread(sessionPtr session, uint8_t newRegistrationState)
{
	if (!session) {
		pbx_log(LOG_NOTICE, "SCCP: session already terminated\n");
		return;
	}
	sccp_log((DEBUGCAT_SOCKET)) (VERBOSE_PREFIX_2 "%s: Stopping Session Thread\n", DEV_ID_LOG(session->device));

	session->session_stop = TRUE;
	if (session->device) {
		sccp_device_setRegistrationState(session->device, newRegistrationState);
	}
	if (AST_PTHREADT_NULL != session->session_thread) {
		shutdown(session->fds[0].fd, SHUT_RD);								// this will also wake up poll
		// which is waiting for a read event and close down the thread nicely
	}
}

static int sccp_dissect_header(sccp_session_t * s, sccp_header_t * header)
{
	unsigned int packetSize = header->length;
	unsigned int protocolVersion = letohl(header->lel_protocolVer);
	unsigned int messageId = letohl(header->lel_messageId);

	// dissecting header to see if we have a valid sccp message, that we can handle
	if (packetSize < 4 || packetSize > SCCP_MAX_PACKET - 8) {
		pbx_log(LOG_ERROR, "SCCP: (sccp_read_data) Size of the data payload in the packet (messageId: %u, protocolVersion: %u / 0x0%x) is out of bounds: %d < %u > %d, cancelling read.\n", 4, messageId, messageId, protocolVersion, packetSize, (int) (SCCP_MAX_PACKET - 8));
		return -1;
	}
	if (protocolVersion > 0 && !(sccp_protocol_isProtocolSupported(s->protocolType, protocolVersion))) {
		pbx_log(LOG_ERROR, "SCCP: (sccp_read_data) protocolversion %u is unknown, cancelling read.\n", protocolVersion);
		return -1;
	}

	if (messageId < SCCP_MESSAGE_LOW_BOUNDARY || messageId > SCCP_MESSAGE_HIGH_BOUNDARY) {
		pbx_log(LOG_ERROR, "SCCP: (sccp_read_data) messageId out of bounds: %d < %u > %d, cancelling read.\n", SCCP_MESSAGE_LOW_BOUNDARY, messageId, SCCP_MESSAGE_HIGH_BOUNDARY);
		return -1;
	}
#if DEBUG
	boolean_t Found = FALSE;
	uint32_t x;

	if (messageId < SPCP_MESSAGE_OFFSET) {
		for (x = 0; x < ARRAY_LEN(sccp_messagetypes); x++) {
			if (messageId == x) {
				Found = TRUE;
				break;
			}
		}
	} else {
		for (x = 0; x < ARRAY_LEN(spcp_messagetypes); x++) {
			if (messageId - SPCP_MESSAGE_OFFSET == x) {
				Found = TRUE;
				break;
			}
		}
	}
	if (!Found) {
		pbx_log(LOG_ERROR, "SCCP: (sccp_read_data) messageId %d could not be found in the array of known messages, cancelling read.\n", messageId);
		//return -1;                                                                            /* not returning -1 in this case, so that we can see the message we would otherwise miss */
	}
#endif

	return msgtype2size(messageId);
}

static void sccp_socket_get_error(constSessionPtr s)
{
	if (!s || s->fds[0].fd <= 0) {
		return;
	}
	int socket = s->fds[0].fd;
	int error = 0;
	socklen_t error_len = sizeof(error);
	if ((socket && getsockopt(socket, SOL_SOCKET, SO_ERROR, &error, &error_len) == 0) && error != 0) {
		pbx_log(LOG_ERROR, "%s: SOL_SOCKET:SO_ERROR: %s (%d)\n", DEV_ID_LOG(s->device), strerror(error), error);
	}
}

/*!
 * \brief Read Data From Socket
 * \param s SCCP Session
 * \param msg Message Data Structure (sccp_msg_t)
 *
 * \lock
 *      - session
 */
static int sccp_read_data(sccp_session_t * s, sccp_msg_t * msg)
{
	if (!s || s->session_stop || s->fds[0].fd <= 0 || !msg) {
		return 0;
	}
	int socket = s->fds[0].fd;

	int msgDataSegmentSize = 0;										/* Size of sccp_data_t according to the sccp_msg_t */
	int UnreadBytesAccordingToPacket = 0;									/* Size of sccp_data_t according to the incomming Packet */
	uint bytesToRead = 0;
	uint bytesReadSoFar = 0;
	int readlen = 0;
	unsigned char buffer[SCCP_MAX_PACKET + 1] = { 0 };
	unsigned char *dataptr;

	errno = 0;
	int tries = 0;
	int backoff = READ_BACKOFF;

	// STAGE 1: read header
	memset(msg, 0, SCCP_MAX_PACKET);
	readlen = read(socket, (&msg->header), SCCP_PACKET_HEADER);
	if (readlen < 0 && (errno == EINTR || errno == EAGAIN)) {
		return 0;
	} /* try again later, return TRUE with empty r */
	else if (readlen <= 0) {
		goto READ_ERROR;
	}
	/* error || client closed socket */
	msg->header.length = letohl(msg->header.length);
	if ((msgDataSegmentSize = sccp_dissect_header(s, &msg->header)) < 0) {
		UnreadBytesAccordingToPacket = msg->header.length - 4;
		//goto READ_SKIP;
		goto READ_ERROR;
	}
	// STAGE 2: read message data
	msgDataSegmentSize -= SCCP_PACKET_HEADER;
	UnreadBytesAccordingToPacket = msg->header.length - 4;							/* correction because we count messageId as part of the header */
	bytesToRead = (UnreadBytesAccordingToPacket > msgDataSegmentSize) ? msgDataSegmentSize : UnreadBytesAccordingToPacket;	/* take the smallest of the two */
	sccp_log_and((DEBUGCAT_SOCKET + DEBUGCAT_HIGH)) (VERBOSE_PREFIX_3 "%s: Dissection %s (%d), msgDataSegmentSize: %d, UnreadBytesAccordingToPacket: %d, msg->header.length: %d, bytesToRead: %d, bytesReadSoFar: %d\n", DEV_ID_LOG(s->device), msgtype2str(letohl(msg->header.lel_messageId)), msg->header.lel_messageId, msgDataSegmentSize, UnreadBytesAccordingToPacket, msg->header.length, bytesToRead, bytesReadSoFar);
	dataptr = (unsigned char *) &msg->data;
	while (bytesToRead > 0) {
		sccp_log_and((DEBUGCAT_SOCKET + DEBUGCAT_HIGH)) (VERBOSE_PREFIX_3 "%s: Reading %s (%d), msgDataSegmentSize: %d, UnreadBytesAccordingToPacket: %d, bytesToRead: %d, bytesReadSoFar: %d\n", DEV_ID_LOG(s->device), msgtype2str(letohl(msg->header.lel_messageId)), msg->header.lel_messageId, msgDataSegmentSize, UnreadBytesAccordingToPacket, bytesToRead, bytesReadSoFar);
		readlen = read(socket, buffer, bytesToRead > SCCP_MAX_PACKET ? SCCP_MAX_PACKET : bytesToRead);					// use bufferptr instead
		if ((readlen < 0) && (tries++ < READ_RETRIES) && (errno == EINTR || errno == EAGAIN)) {
			usleep(backoff);
			backoff *= 2;
			continue;
		} /* try again, blocking */
		else if (readlen <= 0) {
			goto READ_ERROR;
		}												/* client closed socket, because read caused an error */
		bytesReadSoFar += readlen;
		bytesToRead -= readlen;										// if bytesToRead then more segments to read
		memcpy(dataptr, buffer, readlen);
		dataptr += readlen;
	}
	UnreadBytesAccordingToPacket -= bytesReadSoFar;
	tries = 0;

	sccp_log_and((DEBUGCAT_SOCKET + DEBUGCAT_HIGH)) (VERBOSE_PREFIX_3 "%s: Finished Reading %s (%d), msgDataSegmentSize: %d, UnreadBytesAccordingToPacket: %d, bytesToRead: %d, bytesReadSoFar: %d\n", DEV_ID_LOG(s->device), msgtype2str(letohl(msg->header.lel_messageId)), msg->header.lel_messageId, msgDataSegmentSize, UnreadBytesAccordingToPacket, bytesToRead, bytesReadSoFar);

	// STAGE 3: discard the rest of UnreadBytesAccordingToPacket if it was bigger then msgDataSegmentSize
//READ_SKIP:
	if (UnreadBytesAccordingToPacket > 0) {									/* checking to prevent unneeded allocation of discardBuffer */
		pbx_log(LOG_NOTICE, "%s: Going to Discard %d bytes of data for '%s' (%d) (Needs developer attention)\n", DEV_ID_LOG(s->device), UnreadBytesAccordingToPacket, msgtype2str(letohl(msg->header.lel_messageId)), msg->header.lel_messageId);
		unsigned char discardBuffer[SCCP_MAX_PACKET + 1];

		bytesToRead = UnreadBytesAccordingToPacket;
		while (bytesToRead > 0) {
			readlen = read(socket, discardBuffer, (bytesToRead > SCCP_MAX_PACKET) ? SCCP_MAX_PACKET : bytesToRead);
			if ((readlen < 0) && (tries++ < READ_RETRIES) && (errno == EINTR || errno == EAGAIN)) {
				usleep(backoff);
				backoff *= 2;
				continue;
			} /* try again, blocking */
			else if (readlen <= 0) {
				break;
			}											/* if we read EOF, just break reading (invalid packet information) */
			bytesToRead -= readlen;
			sccp_dump_packet((unsigned char *) discardBuffer, readlen);				/* dump the discarded bytes, for analysis */
		}
		pbx_log(LOG_NOTICE, "%s: Discarded %d bytes of data for '%s' (%d) (Needs developer attention)\nReturning Only:\n", DEV_ID_LOG(s->device), UnreadBytesAccordingToPacket, msgtype2str(letohl(msg->header.lel_messageId)), msg->header.lel_messageId);
		sccp_dump_msg(msg);
	}

	/* process message */
	if ((sccp_handle_message(msg, s) == 0)) {
		s->lastKeepAlive = time(0);
		return msg->header.length + 8;
	} else {
		return -2;
	}

READ_ERROR:
	if (errno) {
		pbx_log(LOG_ERROR, "%s: error '%s (%d)' while reading from socket.\n", DEV_ID_LOG(s->device), strerror(errno), errno);
		pbx_log(LOG_ERROR, "%s: stats: %s (%d), msgDataSegmentSize: %d, UnreadBytesAccordingToPacket: %d, bytesToRead: %d, bytesReadSoFar: %d\n", DEV_ID_LOG(s->device), msgtype2str(letohl(msg->header.lel_messageId)), msg->header.lel_messageId, msgDataSegmentSize, UnreadBytesAccordingToPacket, bytesToRead, bytesReadSoFar);
	}
	sccp_socket_get_error(s);
	memset(msg, 0, SCCP_MAX_PACKET);
	return -1;
}

/*!
 * \brief Find Session in Globals Lists
 * \param s SCCP Session
 * \return boolean
 *
 * \lock
 *      - session
 */
static boolean_t sccp_session_findBySession(sccp_session_t * s)
{
	sccp_session_t *session;
	boolean_t res = FALSE;

	SCCP_RWLIST_WRLOCK(&GLOB(sessions));
	SCCP_RWLIST_TRAVERSE(&GLOB(sessions), session, list) {
		if (session == s) {
			res = TRUE;
			break;
		}
	}
	SCCP_RWLIST_UNLOCK(&GLOB(sessions));
	return res;
}

/*!
 * \brief Add a session to the global sccp_sessions list
 * \param s SCCP Session
 * \return boolean
 *
 * \lock
 *      - session
 */
static boolean_t sccp_session_addToGlobals(sccp_session_t * s)
{
	boolean_t res = FALSE;

	if (s) {
		if (!sccp_session_findBySession(s)) {;
			SCCP_RWLIST_WRLOCK(&GLOB(sessions));
			SCCP_LIST_INSERT_HEAD(&GLOB(sessions), s, list);
			res = TRUE;
			SCCP_RWLIST_UNLOCK(&GLOB(sessions));
		}
	}
	return res;
}

/*!
 * \brief Removes a session from the global sccp_sessions list
 * \param s SCCP Session
 * \return boolean
 *
 * \lock
 *      - sessions
 */
static boolean_t sccp_session_removeFromGlobals(sccp_session_t * s)
{
	sccp_session_t *session;
	boolean_t res = FALSE;

	if (s) {
		SCCP_RWLIST_WRLOCK(&GLOB(sessions));
		SCCP_RWLIST_TRAVERSE_SAFE_BEGIN(&GLOB(sessions), session, list) {
			if (session == s) {
				SCCP_LIST_REMOVE_CURRENT(list);
				res = TRUE;
				break;
			}
		}
		SCCP_RWLIST_TRAVERSE_SAFE_END;
		SCCP_RWLIST_UNLOCK(&GLOB(sessions));
	}
	return res;
}


/*!
 * \brief Terminate all session
 *
 * \lock
 *      - socket_lock
 *      - Glob(sessions)
 */
void sccp_session_terminateAll()
{
	sccp_session_t *s = NULL;

	sccp_log((DEBUGCAT_CORE)) (VERBOSE_PREFIX_2 "SCCP: Removing Sessions\n");
	SCCP_RWLIST_TRAVERSE_SAFE_BEGIN(&GLOB(sessions), s, list) {
		sccp_session_stopthread(s, SKINNY_DEVICE_RS_NONE);
	}
	SCCP_RWLIST_TRAVERSE_SAFE_END;

	if (SCCP_LIST_EMPTY(&GLOB(sessions))) {
		SCCP_RWLIST_HEAD_DESTROY(&GLOB(sessions));
	}
}

/*!
 * \brief Release device pointer from session
 * \param session SCCP Session
 */
static sccp_device_t *__sccp_session_removeDevice(sessionPtr session)
{
	sccp_device_t *return_device = NULL;

	if (session && session->device) {
		if (session->device->session && session->device->session != session) {
			// cleanup previous/crossover session
			sccp_session_removeFromGlobals(session->device->session);
		}
		sccp_session_lock(session);
		sccp_device_setRegistrationState(session->device, SKINNY_DEVICE_RS_NONE);

		session->device->session = NULL;
		sccp_copy_string(session->designator, sccp_socket_stringify(&session->ourip), sizeof(session->designator));
		return_device = session->device;								// returning device reference
		session->device = NULL;										// clear device reference
		sccp_session_unlock(session);
	}
	return return_device;
}

/*!
 * \brief Retain device pointer in session. Replace existing pointer if necessary
 * \param session SCCP Session
 * \param device SCCP Device
 * \returns -1 when error happend, 0 if no new ref was taken and 1 if new device ref
 */
static int __sccp_session_addDevice(sessionPtr session, constDevicePtr device)
{
	int res = 0;
	sccp_device_t *new_device = NULL;
	if (session && (!device || (device && session->device != device))) {
		sccp_session_lock(session);
		new_device = sccp_device_retain(device);			/* do this before releasing anything, to prevent device cleanup if the same */
		if (session->device) {
			AUTO_RELEASE sccp_device_t * remDevice = NULL;
			remDevice = __sccp_session_removeDevice(session);		/* implicit release */
		}
		if (device) {
			if (new_device) {
				session->device = new_device;				/* keep newly retained device */
				session->device->session = session;			/* update device session pointer */

				char buf[16] = "";
				snprintf(buf,16, "%s:%d", device->id, session->fds[0].fd);
				sccp_copy_string(session->designator, buf, sizeof(session->designator));
				res = 1;
			} else {
				res = -1;
			}
		}
		sccp_session_unlock(session);
	}
	return res;
}

/*!
 * \brief Retain device pointer in session. Replace existing pointer if necessary (ConstWrapper)
 * \param session SCCP Session
 * \param device SCCP Device
 */
int sccp_session_retainDevice(constSessionPtr session, constDevicePtr device)
{
	if (session && (!device || (device && session->device != device))) {
		sccp_session_t * s = (sccp_session_t *)session;								/* discard const */
		sccp_log((DEBUGCAT_DEVICE)) (VERBOSE_PREFIX_3 "%s: Allocating device to session (%d) %s\n", DEV_ID_LOG(device), s->fds[0].fd, sccp_socket_stringify_addr(&s->sin));
		return __sccp_session_addDevice(s, device);
	}
	return 0;
}


void sccp_session_releaseDevice(constSessionPtr volatile session)
{
	sccp_session_t * s = (sccp_session_t *)session;									/* discard const */
	if (s) {
		AUTO_RELEASE sccp_device_t * device = NULL;
		device = __sccp_session_removeDevice(s);
	}
}

/*!
 * \brief Socket Session Close
 * \param s SCCP Session
 *
 * \callgraph
 * \callergraph
 *
 * \lock
 *      - see sccp_hint_eventListener() via sccp_event_fire()
 *      - session
 */
void sccp_session_close(sccp_session_t * s)
{

	sccp_session_lock(s);
	s->session_stop = TRUE;
	if (s->fds[0].fd > 0) {
		close(s->fds[0].fd);
		s->fds[0].fd = -1;
	}
	sccp_session_unlock(s);
	sccp_log((DEBUGCAT_SOCKET)) (VERBOSE_PREFIX_3 "%s: Old session marked down\n", DEV_ID_LOG(s->device));
}

/*!
 * \brief Destroy Socket Session
 * \param s SCCP Session
 * \param cleanupTime Cleanup Time as uint8_t, Max time before device cleanup starts
 *
 * \callgraph
 * \callergraph
 *
 * \lock
 *      - sessions
 *      - device
 */
void destroy_session(sccp_session_t * s, uint8_t cleanupTime)
{
	boolean_t found_in_list = FALSE;
	char addrStr[INET6_ADDRSTRLEN];

	if (!s) {
		return;
	}

	sccp_copy_string(addrStr, sccp_socket_stringify_addr(&s->sin), sizeof(addrStr));

	found_in_list = sccp_session_removeFromGlobals(s);

	if (!found_in_list) {
		sccp_log((DEBUGCAT_SOCKET)) (VERBOSE_PREFIX_3 "%s: Session could not be found in GLOB(session) %s\n", DEV_ID_LOG(s->device), addrStr);
	}

	/* cleanup device if this session is not a crossover session */
	if (s->device) {
		AUTO_RELEASE sccp_device_t *d = sccp_device_retain(s->device);

		if (d) {
			sccp_log((DEBUGCAT_SOCKET)) (VERBOSE_PREFIX_3 "%s: Destroy Device Session %s\n", DEV_ID_LOG(s->device), addrStr);
			sccp_device_setRegistrationState(d, SKINNY_DEVICE_RS_NONE);
			d->needcheckringback = 0;
			sccp_dev_clean(d, (d->realtime) ? TRUE : FALSE, cleanupTime);
		}
	}

	sccp_log((DEBUGCAT_SOCKET)) (VERBOSE_PREFIX_3 "SCCP: Destroy Session %s\n", addrStr);
	/* closing fd's */
	sccp_session_lock(s);
	if (s->fds[0].fd > 0) {
		close(s->fds[0].fd);
		s->fds[0].fd = -1;
	}
	sccp_session_unlock(s);

	/* destroying mutex and cleaning the session */
	sccp_mutex_destroy(&s->lock);
	sccp_free(s);
	s = NULL;
}

/*!
 * \brief Socket Device Thread Exit
 * \param session SCCP Session
 *
 * \callgraph
 * \callergraph
 */
void sccp_socket_device_thread_exit(void *session)
{
	sccp_session_t *s = (sccp_session_t *) session;

	if (!s->device) {
		sccp_log(DEBUGCAT_SOCKET) (VERBOSE_PREFIX_3 "SCCP: Session without a device attached !\n");
	}

	sccp_log((DEBUGCAT_SOCKET)) (VERBOSE_PREFIX_3 "%s: cleanup session\n", DEV_ID_LOG(s->device));
	sccp_session_close(s);
	s->session_thread = AST_PTHREADT_NULL;
	destroy_session(s, SESSION_DEVICE_CLEANUP_TIME);
}

/*!
 * \brief Socket Device Thread
 * \param session SCCP Session
 *
 * \callgraph
 * \callergraph
 */
void *sccp_socket_device_thread(void *session)
{
	sccp_session_t *s = (sccp_session_t *) session;

	if (!s) {
		return NULL;
	}
	uint8_t keepaliveAdditionalTimePercent = KEEPALIVE_ADDITIONAL_PERCENT;
	int res;
	double maxWaitTime;
	int pollTimeout;
	int read_result = 0;
	sccp_msg_t msg = { {0,} };
	char addrStr[INET6_ADDRSTRLEN];

	pthread_cleanup_push(sccp_socket_device_thread_exit, session);

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	/* we increase additionalTime for wireless/slower devices */
	if (s->device && (s->device->skinny_type == SKINNY_DEVICETYPE_CISCO7920 || s->device->skinny_type == SKINNY_DEVICETYPE_CISCO7921 || s->device->skinny_type == SKINNY_DEVICETYPE_CISCO7925 || s->device->skinny_type == SKINNY_DEVICETYPE_CISCO7926 || s->device->skinny_type == SKINNY_DEVICETYPE_CISCO7975 || s->device->skinny_type == SKINNY_DEVICETYPE_CISCO7970 || s->device->skinny_type == SKINNY_DEVICETYPE_CISCO6911)) {
		keepaliveAdditionalTimePercent += KEEPALIVE_ADDITIONAL_PERCENT;
	}

	while (s->fds[0].fd > 0 && !s->session_stop) {
		/* create cancellation point */
		pthread_testcancel();
		if (s->device) {
			pbx_mutex_lock(&GLOB(lock));
			if (GLOB(reload_in_progress) == FALSE && s && s->device && (!(s->device->pendingUpdate == FALSE && s->device->pendingDelete == FALSE))) {
				sccp_device_check_update(s->device);
			}
			pbx_mutex_unlock(&GLOB(lock));
		}
		/* calculate poll timout using keepalive interval */
		maxWaitTime = (s->device) ? s->device->keepalive : GLOB(keepalive);
		maxWaitTime += (maxWaitTime / 100) * keepaliveAdditionalTimePercent;
		pollTimeout = maxWaitTime * 1000;

		sccp_log_and((DEBUGCAT_SOCKET + DEBUGCAT_HIGH)) (VERBOSE_PREFIX_4 "%s: set poll timeout %d/%d for session %d\n", DEV_ID_LOG(s->device), (int) maxWaitTime, pollTimeout / 1000, s->fds[0].fd);

		pthread_testcancel();										/* poll is also a cancellation point */
		res = sccp_socket_poll(s->fds, 1, pollTimeout);
		pthread_testcancel();
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		if (-1 == res) {										/* poll data processing */
			if (errno > 0 && (errno != EAGAIN) && (errno != EINTR)) {
				sccp_copy_string(addrStr, sccp_socket_stringify_addr(&s->sin), sizeof(addrStr));
				pbx_log(LOG_ERROR, "%s: poll() returned %d. errno: %s, (ip-address: %s)\n", DEV_ID_LOG(s->device), errno, strerror(errno), addrStr);
				__sccp_session_stopthread(s, SKINNY_DEVICE_RS_FAILED);
				break;
			}
		} else if (0 == res) {										/* poll timeout */
			sccp_log((DEBUGCAT_HIGH)) (VERBOSE_PREFIX_2 "%s: Poll Timeout.\n", DEV_ID_LOG(s->device));
			if (((int) time(0) > ((int) s->lastKeepAlive + (int) maxWaitTime))) {
				sccp_copy_string(addrStr, sccp_socket_stringify_addr(&s->sin), sizeof(addrStr));
				ast_log(LOG_NOTICE, "%s: Closing session because connection timed out after %d seconds (timeout: %d) (ip-address: %s).\n", DEV_ID_LOG(s->device), (int) maxWaitTime, pollTimeout, addrStr);
				__sccp_session_stopthread(s, SKINNY_DEVICE_RS_TIMEOUT);
				break;
			}
		} else if (res > 0) {										/* poll data processing */
			if (s->fds[0].revents & POLLIN || s->fds[0].revents & POLLPRI) {			/* POLLIN | POLLPRI */
				/* we have new data -> continue */
				sccp_log_and((DEBUGCAT_SOCKET + DEBUGCAT_HIGH)) (VERBOSE_PREFIX_2 "%s: Session New Data Arriving\n", DEV_ID_LOG(s->device));
				while ((read_result = sccp_read_data(s, &msg)) > 0) {
					s->lastKeepAlive = time(0);
				}
				if (read_result < 0) {
					if (s->device) {
						sccp_device_sendReset(s->device, SKINNY_DEVICE_RESTART);
					}
					__sccp_session_stopthread(s, SKINNY_DEVICE_RS_FAILED);
					break;
				}
			} else {										/* POLLHUP / POLLERR */
				pbx_log(LOG_NOTICE, "%s: Closing session because we received POLLPRI/POLLHUP/POLLERR\n", DEV_ID_LOG(s->device));
				__sccp_session_stopthread(s, SKINNY_DEVICE_RS_FAILED);
				break;
			}
		} else {											/* poll returned invalid res */
			pbx_log(LOG_NOTICE, "%s: Poll Returned invalid result: %d.\n", DEV_ID_LOG(s->device), res);
		}
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	}
	sccp_log((DEBUGCAT_SOCKET)) (VERBOSE_PREFIX_3 "%s: Exiting sccp_socket device thread\n", DEV_ID_LOG(s->device));

	pthread_cleanup_pop(1);

	return NULL;
}

#define SCCP_SETSOCKETOPTION(_SOCKET, _LEVEL,_OPTIONNAME, _OPTIONVAL, _OPTIONLEN) 							\
	if (setsockopt(_SOCKET, _LEVEL, _OPTIONNAME, (void*)_OPTIONVAL, _OPTIONLEN)  == -1) {						\
		if (errno != ENOTSUP) {													\
			pbx_log(LOG_WARNING, "Failed to set SCCP socket: " #_LEVEL ":" #_OPTIONNAME " error: '%s'\n", strerror(errno));	\
		}															\
	}

void sccp_socket_setoptions(int new_socket)
{
	int on = 1;
	int value;

	SCCP_SETSOCKETOPTION(new_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	SCCP_SETSOCKETOPTION(new_socket, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
	value = (int) GLOB(sccp_tos);
	SCCP_SETSOCKETOPTION(new_socket, IPPROTO_IP, IP_TOS, &value, sizeof(value));
#if defined(linux)
	value = (int) GLOB(sccp_cos);
	SCCP_SETSOCKETOPTION(new_socket, SOL_SOCKET, SO_PRIORITY, &value, sizeof(value));

	/* timeeo */
	struct timeval mytv = { SOCKET_TIMEOUT_SEC, SOCKET_TIMEOUT_MILLISEC };					/* timeout after seven seconds when trying to read/write from/to a socket */
	SCCP_SETSOCKETOPTION(new_socket, SOL_SOCKET, SO_RCVTIMEO, &mytv, sizeof(tv));
	SCCP_SETSOCKETOPTION(new_socket, SOL_SOCKET, SO_SNDTIMEO, &mytv, sizeof(tv));

	/* keepalive */
	int ip_keepidle  = SOCKET_KEEPALIVE_IDLE;								/* The time (in seconds) the connection needs to remain idle before TCP starts sending keepalive probes */
	int ip_keepintvl = SOCKET_KEEPALIVE_INTVL;								/* The time (in seconds) between individual keepalive probes, once we have started to probe. */
	int ip_keepcnt   = SOCKET_KEEPALIVE_CNT;								/* The maximum number of keepalive probes TCP should send before dropping the connection. */
	SCCP_SETSOCKETOPTION(new_socket, SOL_TCP, TCP_KEEPIDLE, &ip_keepidle, sizeof(int));
	SCCP_SETSOCKETOPTION(new_socket, SOL_TCP, TCP_KEEPINTVL, &ip_keepintvl, sizeof(int));
	SCCP_SETSOCKETOPTION(new_socket, SOL_TCP, TCP_KEEPCNT, &ip_keepcnt, sizeof(int));
	SCCP_SETSOCKETOPTION(new_socket, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on));

	/* linger */
	struct linger so_linger = {SOCKET_LINGER_ONOFF, SOCKET_LINGER_WAIT};					/* linger=on but wait 0 milliseconds before closing socket and discard all outboung messages */
	SCCP_SETSOCKETOPTION(new_socket, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));

	/* thin-tcp */
#ifdef TCP_THIN_LINEAR_TIMEOUTS
	SCCP_SETSOCKETOPTION(new_socket, IPPROTO_TCP, TCP_THIN_LINEAR_TIMEOUTS, &on, sizeof(on));
	SCCP_SETSOCKETOPTION(new_socket, IPPROTO_TCP, TCP_THIN_DUPACK, &on, sizeof(on));
#endif
	/* */
	/* rcvbuf / sndbug */
	int so_rcvbuf = SOCKET_RCVBUF;
	int so_sndbuf = SOCKET_SNDBUF;
	SCCP_SETSOCKETOPTION(new_socket, SOL_SOCKET, SO_RCVBUF, &so_rcvbuf, sizeof(int));
	SCCP_SETSOCKETOPTION(new_socket, SOL_SOCKET, SO_SNDBUF, &so_sndbuf, sizeof(int));
#endif
}

#undef SCCP_SETSOCKETOPTION
/*!
 * \brief Socket Accept Connection
 *
 * \lock
 *      - sessions
 */
static void sccp_accept_connection(void)
{
	/* called without GLOB(sessions_lock) */
	struct sockaddr_storage incoming;
	sccp_session_t *s;
	int new_socket;
	char addrStr[INET6_ADDRSTRLEN];

	socklen_t length = (socklen_t) (sizeof(struct sockaddr_storage));

	if ((new_socket = accept(GLOB(descriptor), (struct sockaddr *) &incoming, &length)) < 0) {
		pbx_log(LOG_ERROR, "Error accepting new socket %s\n", strerror(errno));
		return;
	}
	sccp_socket_setoptions(new_socket);

	s = sccp_malloc(sizeof(struct sccp_session));
	memset(s, 0, sizeof(sccp_session_t));
	memcpy(&s->sin, &incoming, sizeof(s->sin));
	sccp_mutex_init(&s->lock);

	sccp_session_lock(s);
	s->fds[0].events = POLLIN | POLLPRI;
	s->fds[0].revents = 0;
	s->fds[0].fd = new_socket;

	if (!GLOB(ha)) {
		pbx_log(LOG_NOTICE, "No global ha list\n");
	}

	sccp_copy_string(addrStr, sccp_socket_stringify(&s->sin), sizeof(addrStr));

	/* check ip address against global permit/deny ACL */
	if (GLOB(ha) && sccp_apply_ha(GLOB(ha), &s->sin) != AST_SENSE_ALLOW) {
		struct ast_str *buf = pbx_str_alloca(DEFAULT_PBX_STR_BUFFERSIZE);

		sccp_print_ha(buf, DEFAULT_PBX_STR_BUFFERSIZE, GLOB(ha));
		sccp_log(0) ("SCCP: Rejecting Connection: Ip-address '%s' denied. Check general deny/permit settings (%s).\n", addrStr, pbx_str_buffer(buf));
		pbx_log(LOG_WARNING, "SCCP: Rejecting Connection: Ip-address '%s' denied. Check general deny/permit settings (%s).\n", addrStr, pbx_str_buffer(buf));
		sccp_session_reject(s, "Device ip not authorized");
		sccp_session_unlock(s);
		destroy_session(s, 0);
		return;
	}
	sccp_session_addToGlobals(s);

	/** set default handler for registration to sccp */
	s->protocolType = SCCP_PROTOCOL;

	s->lastKeepAlive = time(0);
	sccp_log((DEBUGCAT_CORE)) (VERBOSE_PREFIX_3 "SCCP: Accepted Client Connection from %s\n", addrStr);

	if (sccp_socket_is_any_addr(&GLOB(bindaddr))) {
		__sccp_socket_setOurAddressFromTheirs(&incoming, &s->ourip);
	} else {
		memcpy(&s->ourip, &GLOB(bindaddr), sizeof(s->ourip));
	}
	sccp_copy_string(s->designator, sccp_socket_stringify(&s->ourip), sizeof(s->designator));

	sccp_log((DEBUGCAT_CORE)) (VERBOSE_PREFIX_3 "SCCP: Connected on server via %s\n", s->designator);

	size_t stacksize = 0;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	pbx_pthread_create(&s->session_thread, &attr, sccp_socket_device_thread, s);
	if (!pthread_attr_getstacksize(&attr, &stacksize)) {
		sccp_log((DEBUGCAT_HIGH)) (VERBOSE_PREFIX_3 "SCCP: Using %d memory for this thread\n", (int) stacksize);
	}
	sccp_session_unlock(s);
}

static void sccp_socket_cleanup_timed_out(void)
{
	sccp_session_t *session;


	SCCP_LIST_TRAVERSE_SAFE_BEGIN(&GLOB(sessions), session, list) {
		if (session->lastKeepAlive == 0) {
			// final resort
			destroy_session(session, 0);
		} else if ((time(0) - session->lastKeepAlive) > (5 * GLOB(keepalive)) && (session->session_thread != AST_PTHREADT_NULL)) {
			pbx_mutex_lock(&GLOB(lock));
			if (GLOB(module_running) && !GLOB(reload_in_progress)) {
				__sccp_session_stopthread(session, SKINNY_DEVICE_RS_FAILED);
				session->session_thread = AST_PTHREADT_NULL;
				session->lastKeepAlive = 0;
			}
			pbx_mutex_unlock(&GLOB(lock));
		}
	}
	SCCP_LIST_TRAVERSE_SAFE_END;
}

/*!
 * \brief Socket Thread
 * \param ignore None
 *
 * \lock
 *      - sessions
 *      - globals
 *          - see sccp_device_check_update()
 *        - see sccp_socket_poll()
 *        - see sccp_session_close()
 *        - see destroy_session()
 *        - see sccp_read_data()
 *        - see sccp_process_data()
 *        - see sccp_handle_message()
 *        - see sccp_device_sendReset()
 */
void *sccp_socket_thread(void *ignore)
{
	struct pollfd fds[1];

	fds[0].events = POLLIN | POLLPRI;
	fds[0].revents = 0;

	int res;

	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	while (GLOB(descriptor) > -1) {
		fds[0].fd = GLOB(descriptor);
		res = sccp_socket_poll(fds, 1, GLOB(keepalive));

		if (res < 0) {
			if (errno == EINTR || errno == EAGAIN) {
				pbx_log(LOG_ERROR, "SCCP poll() returned %d. errno: %d (%s) -- ignoring.\n", res, errno, strerror(errno));
			} else {
				pbx_log(LOG_ERROR, "SCCP poll() returned %d. errno: %d (%s)\n", res, errno, strerror(errno));
			}
		} else if (res == 0) {
			// poll timeout
			sccp_socket_cleanup_timed_out();
		} else {
			sccp_log((DEBUGCAT_SOCKET)) (VERBOSE_PREFIX_3 "SCCP: Accept Connection\n");
			pbx_mutex_lock(&GLOB(lock));
			if (GLOB(module_running) && !GLOB(reload_in_progress)) {
				sccp_accept_connection();
			}
			pbx_mutex_unlock(&GLOB(lock));
		}
	}

	sccp_log((DEBUGCAT_SOCKET)) (VERBOSE_PREFIX_3 "SCCP: Exit from the socket thread\n");
	return NULL;
}

/*!
 * \brief Socket Send Message
 * \param device SCCP Device
 * \param t SCCP Message
 */
void sccp_session_sendmsg(const sccp_device_t * device, sccp_mid_t t)
{
	if (!device || !device->session) {
		sccp_log((DEBUGCAT_SOCKET)) (VERBOSE_PREFIX_3 "SCCP: (sccp_session_sendmsg) No device available to send message to\n");
		return;
	}

	sccp_msg_t *msg = sccp_build_packet(t, 0);

	if (msg) {
		sccp_session_send(device, msg);
	}
}

/*!
 * \brief Socket Send
 * \param device SCCP Device
 * \param msg Message Data Structure (sccp_msg_t)
 * \return SCCP Session Send
 */
int sccp_session_send(constDevicePtr device, const sccp_msg_t * msg_in)
{
	const sccp_session_t * const s = sccp_session_findByDevice(device);
	sccp_msg_t *msg = (sccp_msg_t *) msg_in;				/* discard const * const */

	if (s && !s->session_stop) {
		return sccp_session_send2(s, msg);
	} else {
		return -1;
	}
}

/*!
 * \brief Socket Send Message
 * \param s Session SCCP Session (can't be null)
 * \param msg Message Data Structure (sccp_msg_t) (Will be freed automatically at the end)
 * \return Result as Int
 *
 * \lock
 *      - session
 */
int sccp_session_send2(constSessionPtr session, sccp_msg_t * msg)
{
	sccp_session_t * const s = (sessionPtr) session;								/* discard const */
	ssize_t res = 0;
	uint32_t msgid = letohl(msg->header.lel_messageId);
	ssize_t bytesSent;
	ssize_t bufLen;
	uint8_t *bufAddr;
	unsigned int try, backoff;

	if (s && s->session_stop) {
		return -1;
	}

	if (!s || s->fds[0].fd <= 0) {
		sccp_log((DEBUGCAT_HIGH)) (VERBOSE_PREFIX_3 "SCCP: Tried to send packet over DOWN device.\n");
		if (s) {
			__sccp_session_stopthread(s, SKINNY_DEVICE_RS_FAILED);
		}
		sccp_free(msg);
		msg = NULL;
		return -1;
	}
	int socket = s->fds[0].fd;

	if (msgid == KeepAliveAckMessage || msgid == RegisterAckMessage || msgid == UnregisterAckMessage) {
		msg->header.lel_protocolVer = 0;
	} else if (s->device && s->device->inuseprotocolversion >= 17) {
		msg->header.lel_protocolVer = htolel(0x11);							/* we should always send 0x11 */
	} else {
		msg->header.lel_protocolVer = 0;
	}

	if (msg && (GLOB(debug) & DEBUGCAT_MESSAGE) != 0) {
		uint32_t mid = letohl(msg->header.lel_messageId);

		pbx_log(LOG_NOTICE, "%s: Send Message: %s(0x%04X) %d bytes length\n", DEV_ID_LOG(s->device), msgtype2str(mid), mid, msg ? msg->header.length : 0);
		sccp_dump_msg(msg);
	}

	try = 0;
	backoff = WRITE_BACKOFF;
	bytesSent = 0;
	bufAddr = ((uint8_t *) msg);
	bufLen = (ssize_t) (letohl(msg->header.length) + 8);
	do {
		try++;
		pbx_mutex_lock(&s->write_lock);									/* prevent two threads writing at the same time. That should happen in a synchronized way */
		res = write(socket, bufAddr + bytesSent, bufLen - bytesSent);
		pbx_mutex_unlock(&s->write_lock);
		if (res < 0) {
			if ((errno == EINTR || errno == EAGAIN) && try < WRITE_RETRIES) {
				usleep(backoff);								/* back off to give network/other threads some time */
				backoff *= 2;
				continue;
			}
			pbx_log(LOG_ERROR, "%s: write returned %d (error: '%s (%d)'). Sent %d of %d for Message: '%s' with total length %d \n", DEV_ID_LOG(s->device), (int) res, strerror(errno), errno, (int) bytesSent, (int) bufLen, msgtype2str(letohl(msg->header.lel_messageId)), letohl(msg->header.length));
			sccp_socket_get_error(s);
			if (s) {
				__sccp_session_stopthread(s, SKINNY_DEVICE_RS_FAILED);
			}
			res = -1;
			break;
		}
		bytesSent += res;
	} while (bytesSent < bufLen && try < WRITE_RETRIES && s && !s->session_stop && socket > 0);

	sccp_free(msg);
	msg = NULL;

	if (bytesSent < bufLen) {
		ast_log(LOG_ERROR, "%s: Could only send %d of %d bytes!\n", DEV_ID_LOG(s->device), (int) bytesSent, (int) bufLen);
		res = -1;
	}

	return res;
}

/*!
 * \brief Find session for device
 * \param device SCCP Device
 * \return SCCP Session
 *
 * \lock
 *      - sessions
 */
sccp_session_t *sccp_session_findByDevice(const sccp_device_t * device)
{
	if (!device) {
		sccp_log((DEBUGCAT_SOCKET)) (VERBOSE_PREFIX_3 "SCCP: (sccp_session_find) No device available to find session\n");
		return NULL;
	}

	return device->session;
}

/* defined but not used */
/*
   static sccp_session_t *sccp_session_findSessionForDevice(const sccp_device_t * device)
   {
   sccp_session_t *session;

   SCCP_LIST_TRAVERSE_SAFE_BEGIN(&GLOB(sessions), session, list) {
   if (session->device == device) {
   break;
   }
   }
   SCCP_LIST_TRAVERSE_SAFE_END;

   return session;
   }
 */

/*!
 * \brief Send a Reject Message to Device.
 * \param session SCCP Session Pointer
 * \param message Message as char (reason of rejection)
 */
sccp_session_t *sccp_session_reject(constSessionPtr session, char *message)
{
	sccp_msg_t *msg = NULL;
	sccp_session_t * const s = (sccp_session_t * const) session;			/* discard const */

	REQ(msg, RegisterRejectMessage);
	sccp_copy_string(msg->data.RegisterRejectMessage.text, message, sizeof(msg->data.RegisterRejectMessage.text));
	sccp_session_send2(s, msg);

	/* if we reject the connection during accept connection, thread is not ready */
	__sccp_session_stopthread(s, SKINNY_DEVICE_RS_FAILED);
	return NULL;
}

/*!
 * \brief Send a Reject Message to Device.
 * \param current_session SCCP Session Pointer
 * \param previous_session SCCP Session Pointer
 * \param token Do we need to return a token reject or a session reject (as Boolean)
 */
static void sccp_session_crossdevice_cleanup(constSessionPtr current_session, sessionPtr previous_session, boolean_t token)
{
	if (!current_session) {
		return;
	}

	/* cleanup previous session */
	if (current_session != previous_session) {
		sccp_log(DEBUGCAT_CORE) (VERBOSE_PREFIX_2 "%s: Previous session %p needs to be cleaned up and killed!\n", current_session->designator, previous_session);

		/* remove session */
		sccp_log(DEBUGCAT_SOCKET) (VERBOSE_PREFIX_3 "%s: Remove Session %p from globals\n", current_session->designator, previous_session);
		// sccp_session_removeFromGlobals(previous_session);

		/* cleanup device */
		if (previous_session->device) {
			AUTO_RELEASE sccp_device_t * d = __sccp_session_removeDevice(previous_session);

			if (d) {
				sccp_log(DEBUGCAT_SOCKET) (VERBOSE_PREFIX_3 "%s: Running Device Cleanup\n", DEV_ID_LOG(d));
				sccp_device_setRegistrationState(d, SKINNY_DEVICE_RS_NONE);
				d->needcheckringback = 0;
				sccp_dev_clean(d, (d->realtime) ? TRUE : FALSE, 0);
			}
		}
		/* kill threads */
		sccp_log(DEBUGCAT_SOCKET) (VERBOSE_PREFIX_3 "%s: Kill Previous Session %p Thread\n", current_session->designator, previous_session);
		__sccp_session_stopthread(previous_session, SKINNY_DEVICE_RS_FAILED);
	}

	/* reject current_session and cleanup */
	sccp_log(DEBUGCAT_SOCKET) (VERBOSE_PREFIX_3 "%s: Reject New Session %p and make device come back again for another try.\n", current_session->designator, current_session);
	if (token) {
		sccp_session_tokenReject(current_session, GLOB(token_backoff_time));
	}
	sccp_session_reject(current_session, "Crossover session not allowed, come back later");			/* this gives us a little time to clean everything up */
	return;
}

/*!
 * \brief Send a Reject Message to Device.
 * \param session SCCP Session Pointer
 * \param backoff_time Time to Backoff before retrying TokenSend
 */
void sccp_session_tokenReject(constSessionPtr session, uint32_t backoff_time)
{
	sccp_msg_t *msg = NULL;

	REQ(msg, RegisterTokenReject);
	msg->data.RegisterTokenReject.lel_tokenRejWaitTime = htolel(backoff_time);
	sccp_session_send2(session, msg);
}

/*!
 * \brief Send a token acknowledgement.
 * \param session SCCP Session Pointer
 */
void sccp_session_tokenAck(constSessionPtr session)
{
	sccp_msg_t *msg = NULL;

	REQ(msg, RegisterTokenAck);
	sccp_session_send2(session, msg);
}

/*!
 * \brief Send an Reject Message to the SPCP Device.
 * \param session SCCP Session Pointer
 * \param features Phone Features as Uint32_t
 */
void sccp_session_tokenRejectSPCP(constSessionPtr session, uint32_t features)
{
	sccp_msg_t *msg = NULL;

	REQ(msg, SPCPRegisterTokenReject);
	msg->data.SPCPRegisterTokenReject.lel_features = htolel(features);
	sccp_session_send2(session, msg);
}

/*!
 * \brief Send a token acknowledgement to the SPCP Device.
 * \param session SCCP Session Pointer
 * \param features Phone Features as Uint32_t
 */
void sccp_session_tokenAckSPCP(constSessionPtr session, uint32_t features)
{
	sccp_msg_t *msg = NULL;

	REQ(msg, SPCPRegisterTokenAck);
	msg->data.SPCPRegisterTokenAck.lel_features = htolel(features);
	sccp_session_send2(session, msg);
}

/*!
 * \brief Set Session Protocol
 * \param session SCCP Session
 * \param device SCCP Device
 */
gcc_inline void sccp_session_setProtocol(constSessionPtr session, uint16_t protocolType)
{
	sccp_session_t * s = (sccp_session_t *)session;								/* discard const */

	if (s) {
		s->protocolType = protocolType;
	}
}


/*!
 * \brief Get Session Protocol
 * \param session SCCP Session
 * \param device SCCP Device
 */
gcc_inline uint16_t sccp_session_getProtocol(constSessionPtr session)
{
	if (session) {
		return session->protocolType;
	}
	return UNKNOWN_PROTOCOL;
}

/*!
 * \brief Reset Last KeepAlive
 * \param session SCCP Session
 * \param device SCCP Device
 */
gcc_inline void sccp_session_resetLastKeepAlive(constSessionPtr session)
{
	sccp_session_t * s = (sccp_session_t *)session;								/* discard const */

	if (s) {
		s->lastKeepAlive = time(0);
	}
}

gcc_inline void sccp_session_stopthread(constSessionPtr session, uint8_t newRegistrationState)
{
	sccp_session_t * s = (sccp_session_t *)session;								/* discard const */

	if (s) {
		__sccp_session_stopthread(s, newRegistrationState);
	}
}

gcc_inline const char * const sccp_session_getDesignator(constSessionPtr session)
{
	return session->designator;
}

gcc_inline boolean_t sccp_session_check_crossdevice(constSessionPtr session, constDevicePtr device)
{
	if (session && device && session->device && session->device != device) {
	//if (session && device && (session->device != device || device->session != session)) {
		pbx_log(LOG_WARNING, "Session and Device Session are of sync.\n");
		sccp_session_crossdevice_cleanup(session, device->session, FALSE);
		return TRUE;
	}
	return FALSE;
}


/*!
 * \brief Get device connected to this session
 * \note returns retained device
 */
gcc_inline sccp_device_t * const sccp_session_getDevice(constSessionPtr session, boolean_t required)
{
	if (!session) {
		return NULL;
	}
	sccp_device_t *device = (session->device) ? sccp_device_retain(session->device) : NULL;
	if (required && !device) {
		pbx_log(LOG_WARNING, "No valid Session Device available\n");
		return NULL;
	}
	if (required && sccp_session_check_crossdevice(session, device)) {
		sccp_device_release(device);							/* explicit release after error */
		return NULL;
	}
	return device;
}

boolean_t sccp_session_isValid(constSessionPtr session)
{
	if (session && session->fds[0].fd > 0 && !session->session_stop && !sccp_socket_is_any_addr(&session->ourip)) {
		return TRUE;
	}
	return FALSE;
}

/* -------------------------------------------------------------------------------------------------------SHOW SESSIONS- */
/*!
 * \brief Show Sessions
 * \param fd Fd as int
 * \param total Total number of lines as int
 * \param s AMI Session
 * \param m Message
 * \param argc Argc as int
 * \param argv[] Argv[] as char
 * \return Result as int
 *
 * \called_from_asterisk
 *
 */
int sccp_cli_show_sessions(int fd, sccp_cli_totals_t *totals, struct mansession *s, const struct message *m, int argc, char *argv[])
{
	int local_line_total = 0;
	char clientAddress[INET6_ADDRSTRLEN] = "";

#define CLI_AMI_TABLE_NAME Sessions
#define CLI_AMI_TABLE_PER_ENTRY_NAME Session
#define CLI_AMI_TABLE_LIST_ITER_HEAD &GLOB(sessions)
#define CLI_AMI_TABLE_LIST_ITER_TYPE sccp_session_t
#define CLI_AMI_TABLE_LIST_ITER_VAR session
#define CLI_AMI_TABLE_LIST_LOCK SCCP_RWLIST_RDLOCK
#define CLI_AMI_TABLE_LIST_ITERATOR SCCP_RWLIST_TRAVERSE
#define CLI_AMI_TABLE_LIST_UNLOCK SCCP_RWLIST_UNLOCK
#define CLI_AMI_TABLE_BEFORE_ITERATION 														\
		sccp_session_lock(session);													\
		sccp_copy_string(clientAddress, sccp_socket_stringify_addr(&session->sin), sizeof(clientAddress));				\
		AUTO_RELEASE sccp_device_t *d = session->device ? sccp_device_retain(session->device) : NULL;								\
		if (d || (argc == 4 && sccp_strcaseequals(argv[3],"all"))) {									\

#define CLI_AMI_TABLE_AFTER_ITERATION 														\
		}																\
		sccp_session_unlock(session);													\

#define CLI_AMI_TABLE_FIELDS 															\
		CLI_AMI_TABLE_FIELD(Socket,		"-6",		d,	6,	session->fds[0].fd)					\
		CLI_AMI_TABLE_FIELD(IP,			"40.40",	s,	40,	clientAddress)                                		\
		CLI_AMI_TABLE_FIELD(Port,		"-5",		d,	5,	sccp_socket_getPort(&session->sin) )    		\
		CLI_AMI_TABLE_FIELD(KA,			"-4",		d,	4,	(uint32_t) (time(0) - session->lastKeepAlive))		\
		CLI_AMI_TABLE_FIELD(KAI,		"-4",		d,	4,	(d) ? d->keepaliveinterval : 0)				\
		CLI_AMI_TABLE_FIELD(DeviceName,		"15",		s,	15,	(d) ? d->id : "--")					\
		CLI_AMI_TABLE_FIELD(State,		"-14.14",	s,	14,	(d) ? sccp_devicestate2str(sccp_device_getDeviceState(d)) : "--")		\
		CLI_AMI_TABLE_FIELD(Type,		"-15.15",	s,	15,	(d) ? skinny_devicetype2str(d->skinny_type) : "--")	\
		CLI_AMI_TABLE_FIELD(RegState,		"-10.10",	s,	10,	(d) ? skinny_registrationstate2str(sccp_device_getRegistrationState(d)) : "--")	\
		CLI_AMI_TABLE_FIELD(Token,		"-10.10",	s,	10,	d ? sccp_tokenstate2str(d->status.token) : "--")
#include "sccp_cli_table.h"

	if (s) {
		totals->lines = local_line_total;
		totals->tables = 1;
	}
	return RESULT_SUCCESS;
}

// kate: indent-width 8; replace-tabs off; indent-mode cstyle; auto-insert-doxygen on; line-numbers on; tab-indents on; keep-extra-spaces off; auto-brackets off;
