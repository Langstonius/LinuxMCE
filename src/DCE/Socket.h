/**
 *
 * @file Socket.h
 * @brief header file for the Socket class
 * @author
 * @todo notcommented
 *
 */


#ifndef SOCKET_H
#define SOCKET_H

#include "PlutoUtils/MultiThreadIncludes.h"

#define INSOCKBUFFER_SIZE 8192

#define SOCKET_TIMEOUT		30

namespace DCE
{
	
	class Message; /** < to be able to use it in declarations; we include it's header in the cpp file */

	/**
	 * @brief defines a socket that works under different architectures
	 * @todo ask !!!
	 */
	class Socket
	{
		
	public:
		enum SocketType { st_Unknown, st_ServerCommand, st_ServerEvent, st_ClientCommand, st_ClientEvent } m_eSocketType;

		/** < If true,  the framework will send a ping every 10 seconds or so to be sure the network
		is ok.  This is useful for poor wireless connections */
		bool m_bUsePingToKeepAlive;
		bool m_bUsePlainText; /** < If true, messages will be sent in plain text format, just like a MessageSend string */

		SOCKET m_Socket; /** < the actual socket @todo ask */
		
		string m_sName; /** < a name for the socket */
		string m_sIPAddress,m_sHostName; /** < The hostname is the original name for the server, which may be a name--not an IP, IP is always the IP of the server */
		string m_sMyIPAddress; /** < Only used on a client side, this is the IP address the server reported for the incoming connection */
		string m_sMacAddress; /** < On both client/server sockets, the client's mac address */
		
		int m_iSocketCounter; /** < counts the sockets actually created */

		char *m_pcSockLogFile; /** < name of log file for the socket */
		char *m_pcSockLogErrorFile; /** < name of error log file for the socket */	
		pluto_pthread_mutex_t m_SocketMutex; /** < for synchronisation between threads @todo ask */
		pthread_mutexattr_t m_SocketMutexAttr;  /** < for synchronisation between threads @todo ask */

		pthread_t m_pthread_pingloop_id; /** < the thread id for the tread that's treating the messages from the message queue */
		pthread_cond_t m_PingLoopCond; /** < condition for the messages in the queue */

		char *m_pcInSockBuffer; /** < input buffer for the socket @todo ask */
		char *m_pcCurInsockBuffer; /** < input buffer current position @todo ask */
		
		int m_iSockBufBytesLeft; /** < bytes left in the socket buffer @todo ask */
		int m_iReceiveTimeout; /** < the interval after witch the sockets stops expecting for an answer in seconds */
		bool m_bQuit; /** < set when the socket should terminate */
        bool m_bCancelSocketOp; /** < cancel any operations with the socket right away */

		/** < If non null, then any socket failures will be sent to the
		m_pSocket_PingFailure rather than handled locally */
		Socket *m_pSocket_PingFailure;
		/**
		 * @brief creates a new socket objest with the specified name, and it also writes a log
		 */
		Socket( string sName,string sIPAddress="",string sMacAddress="" );
		
		/**
		 * @brief frees the allocated memory and closes the base socket, and it also writes a log
		 */
		virtual ~Socket();
		
		/** @todo check message */
		//	void DCESocketLock();
		//	void DCESocketUnlock();

		/**
		 * @brief use to set the member
		 */
		void SetReceiveTimeout( int TimeoutSeconds ) { m_iReceiveTimeout = TimeoutSeconds; };
		
		/**
		 * @brief sends the message data imediatlly and deletes the message object if required
		 * @param bDeleteMessage specifies if the message should be deleted after use
		 * @return the return of SendData (witch it uses)
		 * @see SendData
		 */
		bool SendMessage( Message *pMessage, bool bDeleteMessage = true );


		/**
		 * @brief it reads the data from the socket then parses it into a new Message
		 * this gets called in response to a MESSAGE <Length> string
		 * If bText=true, the message is assumed to be in text, not binary format
		 */
		virtual Message *ReceiveMessage( int iLength, bool bText=false );

		/**
		 * @brief just sends raw data to the socket.
		 */
		virtual bool SendData( int iSize, const char *pcData );
		
		/**
		 * @brief just reads raw data from the socket
		 */
		virtual bool ReceiveData( int iSize, char *pcData );

		/**
		 * @brief sends a string to the socket.
		 * @return false on errors
		 */
		virtual bool SendString( string sToSend );
		
		/**
		 * @brief reads a string from the socket.
		 * @return false on errors
		 */
		virtual bool ReceiveString( string &sResult );

		/**
		 * @brief sends a string to the socket and retrives the answer
		 * @return false on errors
		 */
		virtual string SendReceiveString( string sToSend );

		/**
		 * @brief sends a message to the socket and retrives the answer
		 * @return false on errors
		 */
		virtual Message *SendReceiveMessage( Message *pMessage );

		/**
		 * @brief Close the socket
		 */
		virtual void Close();

		/**
		 * @brief Start monitoring the socket to be sure it stays alive
		 */
		virtual void StartPingLoop();

		/**
		 * @brief Called when a ping test failed
		 */
		virtual void PingFailed();
		static class SocketInfo *g_mapSocketInfo_Find(int iSocketCounter,string sName,Socket *pSocket);
	};

    extern pluto_pthread_mutex_t *g_pSocketInfoMutex;
	class SocketInfo
	{
	public:
		Socket *m_pSocket;
		int m_iSocketCounter;
		string m_sName,m_sDevice,m_sComment,m_sLogFile;
		string m_sLastStringOut;
		string m_sLastStringIn;
		time_t m_tCreated,m_tDestroyed;

		SocketInfo(int iSocketCounter,string sName,Socket *pSocket) { m_iSocketCounter=iSocketCounter; m_sName=sName; m_pSocket=pSocket; m_tCreated=m_tDestroyed=0; }
	};
	extern map<int,SocketInfo *> g_mapSocketInfo;
}

#endif

