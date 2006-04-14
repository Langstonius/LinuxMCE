#include "BD/BDCommandProcessor_Symbian_Bluetooth.h"

#include <bt_sock.h>

const TInt KPort = 19; //3460;

void BDCommandProcessor_Symbian_Bluetooth::Start() 
{
	User::LeaveIfError(iSocketServ.Connect());
    User::LeaveIfError(iSdpSession.Connect());
    User::LeaveIfError(iSdpDatabase.Open(iSdpSession));

	// Begin listening for incoming pluto connections
	User::LeaveIfError( iListeningSocket.Open( iSocketServ, KServerTransportName ) );
	User::LeaveIfError( iSocket.Open( iSocketServ ) );

	TBTSockAddr listeningAddress;
	listeningAddress.SetPort( KPlutoMOPort );

	User::LeaveIfError( iListeningSocket.Bind( listeningAddress ) );
}

