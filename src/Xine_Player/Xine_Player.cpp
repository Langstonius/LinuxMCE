//<-dceag-d-b->
#include "Xine_Player.h"
#include "DCE/Logger.h"
#include "PlutoUtils/FileUtils.h"
#include "PlutoUtils/StringUtils.h"
#include "PlutoUtils/Other.h"

#include <iostream>
using namespace std;
using namespace DCE;

#include "Gen_Devices/AllCommandsRequests.h"
//<-dceag-d-e->

#include "Xine_Stream_Factory.h"
#include "pluto_main/Define_Button.h"

//<-dceag-const-b->
// The primary constructor when the class is created as a stand-alone device
Xine_Player::Xine_Player(int DeviceID, string ServerAddress,bool bConnectEventHandler,bool bLocalMode,class Router *pRouter)
	: Xine_Player_Command(DeviceID, ServerAddress,bConnectEventHandler,bLocalMode,pRouter)
//<-dceag-const-e->
{
	m_pDeviceData_MediaPlugin = NULL;
	ptrFactory = new Xine_Stream_Factory(this);
	
	m_pNotificationSocket = new XineNotification_SocketListener(string("m_pNotificationSocket"));
	m_pNotificationSocket->m_bSendOnlySocket = true; // one second
}

//<-dceag-const2-b->
// The constructor when the class is created as an embedded instance within another stand-alone device
Xine_Player::Xine_Player(Command_Impl *pPrimaryDeviceCommand, DeviceData_Impl *pData, Event_Impl *pEvent, Router *pRouter)
	: Xine_Player_Command(pPrimaryDeviceCommand, pData, pEvent, pRouter)
//<-dceag-const2-e->
{
	m_pDeviceData_MediaPlugin=NULL;
}

//<-dceag-dest-b->
Xine_Player::~Xine_Player()
//<-dceag-dest-e->
{
	if (ptrFactory)
	{
		ptrFactory->ShutdownFactory();
		delete ptrFactory;
	}
	
	if (m_pNotificationSocket)
	{
		delete m_pNotificationSocket;
		m_pNotificationSocket = NULL;
	}
}

//<-dceag-getconfig-b->
bool Xine_Player::GetConfig()
{
	if( !Xine_Player_Command::GetConfig() )
		return false;
//<-dceag-getconfig-e->
	m_pDeviceData_MediaPlugin = m_pData->m_AllDevices.m_mapDeviceData_Base_FindFirstOfCategory(DEVICECATEGORY_Media_Plugins_CONST);
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::EVENT_Playback_Completed(streamID=%i)", 0);
	EVENT_Playback_Completed("",0,false);  // In case media plugin thought something was playing, let it know that there's not

	// Quick and dirty, get nbd-client working
	FILE *file = fopen("/etc/nbd-client","wb");
	if( file )
	{
		int Count=0;
		for(Map_DeviceData_Base::iterator it=m_pData->m_AllDevices.m_mapDeviceData_Base.begin();it!=m_pData->m_AllDevices.m_mapDeviceData_Base.end();++it)
		{
			DeviceData_Base *pDevice = it->second;
			DeviceData_Base *pDevice_TopMost = pDevice->GetTopMostDevice();
			if( pDevice->WithinCategory(DEVICECATEGORY_Disc_Drives_CONST) && pDevice_TopMost && pDevice_TopMost!=m_pData->GetTopMostDevice() )
			{
				fprintf(file,"NBD_DEVICE[%d]=/dev/device_%d\n",Count,pDevice->m_dwPK_Device);
				fprintf(file,"NBD_TYPE[%d]=r\n",Count);
				fprintf(file,"NBD_PORT[%d]=%d\n",Count,pDevice->m_dwPK_Device+18000);
				fprintf(file,"NBD_HOST[%d]=%s\n",Count,pDevice_TopMost->m_sIPAddress.c_str());
				Count++;
			}
		}
		fclose(file);
		system("/etc/init.d/nbd-client restart");
	}
	else
		g_pPlutoLogger->Write(LV_CRITICAL,"Cannot create nbd-client");
	
	// Put your code here to initialize the data in this class
	// The configuration parameters DATA_ are now populated
	return true;
}

//<-dceag-reg-b->
// This function will only be used if this device is loaded into the DCE Router's memory space as a plug-in.  Otherwise Connect() will be called from the main()
bool Xine_Player::Register()
//<-dceag-reg-e->
{
	return Connect(PK_DeviceTemplate_get()); 
}

/*  Since several parents can share the same child class, and each has it's own implementation, the base class in Gen_Devices
	cannot include the actual implementation.  Instead there's an extern function declared, and the actual new exists here.  You 
	can safely remove this block (put a ! after the dceag-createinst-b block) if this device is not embedded within other devices. */
//<-dceag-createinst-b->
Xine_Player_Command *Create_Xine_Player(Command_Impl *pPrimaryDeviceCommand, DeviceData_Impl *pData, Event_Impl *pEvent, Router *pRouter)
{
	return new Xine_Player(pPrimaryDeviceCommand, pData, pEvent, pRouter);
}
//<-dceag-createinst-e->

/*
	When you receive commands that are destined to one of your children,
	then if that child implements DCE then there will already be a separate class
	created for the child that will get the message.  If the child does not, then you will 
	get all	commands for your children in ReceivedCommandForChild, where 
	pDeviceData_Base is the child device.  If you handle the message, you 
	should change the sCMD_Result to OK
*/
//<-dceag-cmdch-b->
void Xine_Player::ReceivedCommandForChild(DeviceData_Impl *pDeviceData_Impl,string &sCMD_Result,Message *pMessage)
//<-dceag-cmdch-e->
{
	sCMD_Result = "UNHANDLED CHILD";
}

/*
	When you received a valid command, but it wasn't for one of your children,
	then ReceivedUnknownCommand gets called.  If you handle the message, you 
	should change the sCMD_Result to OK
*/
//<-dceag-cmduk-b->
void Xine_Player::ReceivedUnknownCommand(string &sCMD_Result,Message *pMessage)
//<-dceag-cmduk-e->
{
	sCMD_Result = "UNKNOWN DEVICE";
}

//<-dceag-sample-b->!

/*

	COMMANDS TO IMPLEMENT

*/

//<-dceag-c28-b->

	/** @brief COMMAND: #28 - Simulate Keypress */
	/** Send a key to the device's OSD, or simulate keypresses on the device's panel */
		/** @param #26 PK_Button */
			/** What key to simulate being pressed.  If 2 numbers are specified, separated by a comma, the second will be used if the Shift key is specified. */
		/** @param #50 Name */
			/** The application to send the keypress to. If not specified, it goes to the DCE device. */

void Xine_Player::CMD_Simulate_Keypress(string sPK_Button,string sName,string &sCMD_Result,Message *pMessage)
//<-dceag-c28-e->
{
	int PK_Button = atoi(sPK_Button.c_str());
	g_pPlutoLogger->Write(LV_CRITICAL,"Xine_Player::CMD_Simulate_Keypress %d",PK_Button);
	switch(PK_Button)
	{
		case BUTTON_Up_Arrow_CONST:
			CMD_Move_Up();
			break;
		case BUTTON_Down_Arrow_CONST:
			CMD_Move_Down();
			break;
		case BUTTON_Left_Arrow_CONST:
			CMD_Move_Left();
			break;
		case BUTTON_Right_Arrow_CONST:
			CMD_Move_Right();
			break;
		case BUTTON_Enter_CONST:
			CMD_EnterGo();
			break;
		default:
			g_pPlutoLogger->Write(LV_CRITICAL,"Xine_Player::CMD_Simulate_Keypress -- Can't handle %d",PK_Button);
			break;
	}
}

//<-dceag-c29-b->

	/** @brief COMMAND: #29 - Simulate Mouse Click */
	/** Simlate a mouse click at a certain position on the screen */
		/** @param #11 Position X */
			/**  */
		/** @param #12 Position Y */
			/**  */

void Xine_Player::CMD_Simulate_Mouse_Click(int iPosition_X,int iPosition_Y,string &sCMD_Result,Message *pMessage)
//<-dceag-c29-e->
{
	//HACK we have to know stream ID here
	Xine_Stream *pStream =  ptrFactory->GetStream( 0 );
	
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Simulate_Mouse_Click() with corresponding stream %p.", pStream);
	
	if (pStream==NULL)
	{
		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Simulate_Mouse_Click() stream is NULL, aborting - init failure?");
		return;
	}
	else
		pStream->simulateMouseClick( iPosition_X, iPosition_Y );
}

//<-dceag-c37-b->

	/** @brief COMMAND: #37 - Play Media */
	/** This command will instruct a Media Player to play a media stream identified by a media descriptor created by the "Create Media" command. */
		/** @param #29 PK_MediaType */
			/** The type of media */
		/** @param #41 StreamID */
			/** The media that we need to play. */
		/** @param #42 MediaPosition */
			/** The position at which we need to start playing. */
		/** @param #59 MediaURL */
			/** The file to play, or other media id.  The format is specific on the media type and the media player. */

void Xine_Player::CMD_Play_Media(int iPK_MediaType,int iStreamID,string sMediaPosition,string sMediaURL,string &sCMD_Result,Message *pMessage)
//<-dceag-c37-e->
{
	Xine_Stream *prevStream =  ptrFactory->GetStream( iStreamID );
	if ( (prevStream != NULL) && (prevStream->m_bIsRendering) && (iStreamID!=0) )
	{
		string mPosition;
		CMD_Stop_Media( iStreamID, &mPosition);
		if (prevStream->m_iMenuButtons!=0)
			prevStream->FireMenuOnScreen( 0 );
	}
	

	Xine_Stream *pStream =  ptrFactory->GetStream( iStreamID, true, pMessage?pMessage->m_dwPK_Device_From:0);
	
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Play_Media() called for filename: %s (%s) with corresponding stream %p.", sMediaURL.c_str(),sMediaPosition.c_str(),pStream);
	
	if (pStream==NULL)
	{
		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Play_Media() stream is NULL, aborting - init failure?");
		return;
	}
	
	// hiding menu if it was on screen
	if (pStream->m_iMenuButtons!=0)
		pStream->FireMenuOnScreen( 0 );
	
	
	if( sMediaURL.size()==0 && sMediaPosition.size()==0)
	{
		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Play_Media() with empty filename and position");
		pStream->changePlaybackSpeed(Xine_Stream::PLAYBACK_NORMAL);
		return;
	}
	
	if( sMediaURL.size()==0 && pStream->m_bIsRendering)
	{
		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Play_Media() with empty filename");
		pStream->changePlaybackSpeed(Xine_Stream::PLAYBACK_STOP);
		pStream->playStream( sMediaPosition);
		return;
	}
	
	pStream->m_sCurrentFile=sMediaURL;

	
	string sMediaInfo;
	
	if (pStream->OpenMedia( sMediaURL, sMediaInfo,  sMediaPosition))
	{
		if (pStream->playStream( sMediaPosition))
		{
			g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::EVENT_Playback_Started(streamID=%i)", iStreamID);
			EVENT_Playback_Started(sMediaURL,iStreamID,sMediaInfo,pStream->m_sAudioInfo,pStream->m_sVideoInfo);
			
			// telling to all slaves to start playback
			if (pStream->m_bBroadcaster && pStream->m_iBroadcastPort!=0)
			{
				string sSlaveMediaURL;
//				sSlaveMediaURL = sSlaveMediaURL + "slave://" + "localhost"  + ":" + StringUtils::itos(pStream->m_iBroadcastPort);
				sSlaveMediaURL = sSlaveMediaURL + "slave://" + m_sIPofMD + ":" + StringUtils::itos(pStream->m_iBroadcastPort);
				g_pPlutoLogger->Write(LV_WARNING, "Slave URL: %s", sSlaveMediaURL.c_str());
				string streamingTargets = pStream->m_sBroadcastTargets;
				string::size_type tokenPos=0;
				string curTarget = StringUtils::Tokenize(streamingTargets, string(","), tokenPos);
				while (curTarget!="")
				{
					int targetDevice = atoi(curTarget.c_str());
					if (targetDevice)
					{
						// if local instance, adding 1000 to streamID
						DCE::CMD_Play_Media cmd(m_dwPK_Device, targetDevice, iPK_MediaType, (targetDevice==m_dwPK_Device)?(iStreamID+1000):iStreamID, 
							sMediaPosition, sSlaveMediaURL);
						SendCommandNoResponse(cmd);
						g_pPlutoLogger->Write(LV_WARNING, "Starting playback on device %i %s, stream ID=%i", targetDevice, (targetDevice==m_dwPK_Device)?"[local]":"[remote]",
							(targetDevice==m_dwPK_Device)?(iStreamID+1000):iStreamID );
					}
						
					curTarget = StringUtils::Tokenize(streamingTargets, string(","), tokenPos);
				}
			}
		}
		else
		{
			g_pPlutoLogger->Write(LV_CRITICAL, "Xine_Player::CMD_Play_Media() failed to play -- retrying without starting position info");
			if (pStream->playStream( sMediaPosition))
			{
				g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::EVENT_Playback_Started(streamID=%i)", iStreamID);
				EVENT_Playback_Started(sMediaURL,iStreamID,sMediaInfo,pStream->m_sAudioInfo,pStream->m_sVideoInfo);
			
				// telling to all slaves to start playback
				if (pStream->m_bBroadcaster && pStream->m_iBroadcastPort!=0)
				{
					string sSlaveMediaURL;
					sSlaveMediaURL = sSlaveMediaURL + "slave://" + m_sIPofMD + ":" + StringUtils::itos(pStream->m_iBroadcastPort);
					g_pPlutoLogger->Write(LV_WARNING, "Slave URL: %s", sSlaveMediaURL.c_str());
					string streamingTargets = pStream->m_sBroadcastTargets;
					string::size_type tokenPos=0;
					string curTarget = StringUtils::Tokenize(streamingTargets, string(","), tokenPos);
					while (curTarget!="")
					{
						int targetDevice = atoi(curTarget.c_str());
						if (targetDevice)
						{
							// if local instance, adding 1000 to streamID
							DCE::CMD_Play_Media cmd(m_dwPK_Device, targetDevice, iPK_MediaType, (targetDevice==m_dwPK_Device)?(iStreamID+1000):iStreamID, 
								sMediaPosition, sSlaveMediaURL);
							SendCommandNoResponse(cmd);
							g_pPlutoLogger->Write(LV_WARNING, "Starting playback on device %i %s, stream ID=%i", targetDevice, (targetDevice==m_dwPK_Device)?"[local]":"[remote]",
								(targetDevice==m_dwPK_Device)?(iStreamID+1000):iStreamID );
						}
							
						curTarget = StringUtils::Tokenize(streamingTargets, string(","), tokenPos);
					}
				}
			}
			else
			{
				g_pPlutoLogger->Write(LV_CRITICAL, "Xine_Player::CMD_Play_Media() failed to play %s without position info",sMediaURL.c_str());
				g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::EVENT_Playback_Completed(streamID=%i)", iStreamID);
				EVENT_Playback_Completed(sMediaURL,iStreamID,true);  // true = there was an error, don't keep repeating
			}
		}
	}
	else
	{
		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Play_Media() Failed to open media");
	}
	
	// storing streamID for reuse
	if (!pStream->m_bBroadcaster)
		ptrFactory->m_iLastRenderingStream = iStreamID;
	
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Play_Media() ended for filename: %s with stream %p.", sMediaURL.c_str(), pStream);
	
}

//<-dceag-c38-b->

	/** @brief COMMAND: #38 - Stop Media */
	/** This will instruct the media player to stop the playback of a media started with the "Play Media" Command */
		/** @param #41 StreamID */
			/** The media needing to be stopped. */
		/** @param #42 MediaPosition */
			/** The position at which this stream was last played. */

void Xine_Player::CMD_Stop_Media(int iStreamID,string *sMediaPosition,string &sCMD_Result,Message *pMessage)
//<-dceag-c38-e->
{
	Xine_Stream *pStream =  ptrFactory->GetStream( iStreamID );
	
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Stop_Media() with corresponding stream %p.", pStream);
	
	if (pStream==NULL)
	{
		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Stop_Media() stream is NULL, aborting - init failure?");
		return;
	}
	
	pStream->pauseMediaStream();
	*sMediaPosition = pStream->GetPosition();
	
	
	// telling to all slaves to start playback
	if (pStream->m_bBroadcaster && pStream->m_iBroadcastPort!=0)
	{
		string streamingTargets = pStream->m_sBroadcastTargets;
		string::size_type tokenPos=0;
		string curTarget = StringUtils::Tokenize(streamingTargets, string(","), tokenPos);
		while (curTarget!="")
		{
			int targetDevice = atoi(curTarget.c_str());
			if (targetDevice)
			{
				// if local instance, adding 1000 to streamID
				string sPos;
				DCE::CMD_Stop_Media cmd(m_dwPK_Device, targetDevice, (targetDevice==m_dwPK_Device)?(iStreamID+1000):iStreamID, &sPos );
				SendCommandNoResponse(cmd);
				g_pPlutoLogger->Write(LV_WARNING, "Stopping playback on device %i %s, stream ID=%i", targetDevice, (targetDevice==m_dwPK_Device)?"[local]":"[remote]",
					(targetDevice==m_dwPK_Device)?(iStreamID+1000):iStreamID );
			}
				
			curTarget = StringUtils::Tokenize(streamingTargets, string(","), tokenPos);
		}
	}


	pStream->CloseMedia();
	
	ptrFactory->CloseStreamAV( iStreamID );
	//ptrFactory->HideStreamWindows( 1 );
}

//<-dceag-c39-b->

	/** @brief COMMAND: #39 - Pause Media */
	/** This will stop a media that is currently played. This method should be paired with the "Restart Media" and used when the playback will be stopped and restarted on the same display device. */
		/** @param #41 StreamID */
			/** The media stream for which we need to pause playback. */

void Xine_Player::CMD_Pause_Media(int iStreamID,string &sCMD_Result,Message *pMessage)
//<-dceag-c39-e->
{
	Xine_Stream *pStream =  ptrFactory->GetStream( iStreamID );
	
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Pause_Media() with corresponding stream %p.", pStream);
	
	if (pStream==NULL)
	{
		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Pause_Media() stream is NULL, aborting - init failure?");
		return;
	}
	else
		pStream->pauseMediaStream();
}

//<-dceag-c40-b->

	/** @brief COMMAND: #40 - Restart Media */
	/** This will restart a media was paused with the above command */
		/** @param #41 StreamID */
			/** The media stream that we need to restart playback for. */

void Xine_Player::CMD_Restart_Media(int iStreamID,string &sCMD_Result,Message *pMessage)
//<-dceag-c40-e->
{
	Xine_Stream *pStream =  ptrFactory->GetStream( iStreamID );
	
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Restart_Media() with corresponding stream %p.", pStream);
	
	if (pStream==NULL)
	{
		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Restart_Media() stream is NULL, aborting - init failure?");
		return;
	}
	else
		pStream->restartMediaStream();
}

//<-dceag-c41-b->

	/** @brief COMMAND: #41 - Change Playback Speed */
	/** Will make the playback to FF with a configurable amount of speed. */
		/** @param #41 StreamID */
			/** The media needing the playback speed change. */
		/** @param #43 MediaPlaybackSpeed */
			/** The requested media playback speed * 1000.  -1000 = rev, 4000 = 4x fwd, -500 = rev 1/2.  Less than 10 = relative.  +2 = double, -1 = reverse.   See Media_Plugin::ReceivedMessage */
		/** @param #220 Report */
			/** If true, report this speed to the user on the OSD */

void Xine_Player::CMD_Change_Playback_Speed(int iStreamID,int iMediaPlaybackSpeed,bool bReport,string &sCMD_Result,Message *pMessage)
//<-dceag-c41-e->
{
	Xine_Stream *pStream =  ptrFactory->GetStream( iStreamID );
	
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Change_Playback_Speed() with corresponding stream %p.", pStream);
	
	if (pStream==NULL)
	{
		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Change_Playback_Speed() stream is NULL, aborting - init failure?");
		return;
	}
	else
		pStream->changePlaybackSpeed((Xine_Stream::PlayBackSpeedType)iMediaPlaybackSpeed);
}

//<-dceag-c42-b->

	/** @brief COMMAND: #42 - Jump to Position in Stream */
	/** Jump to a position in the stream, specified in seconds. */
		/** @param #5 Value To Assign */
			/** The number of seconds.  A number is considered an absolute.  "+2" means forward 2, "-1" means back 1.  A simpler command than Set Media Position */
		/** @param #41 StreamID */
			/** The stream */

void Xine_Player::CMD_Jump_to_Position_in_Stream(string sValue_To_Assign,int iStreamID,string &sCMD_Result,Message *pMessage)
//<-dceag-c42-e->
{
	Xine_Stream *pStream =  ptrFactory->GetStream( iStreamID );
	if (pStream==NULL)
	{
		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Jump_to_Position_in_Stream() stream is NULL, aborting - init failure?");
		return;
	}
	else
	{
		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Change_Playback_Speed() with corresponding stream %p and value %s", pStream, sValue_To_Assign.c_str());
		if (sValue_To_Assign!="")
		{
				int currentTime, totalTime;
				int iMediaPosition = pStream->getStreamPlaybackPosition(currentTime, totalTime);
				switch (sValue_To_Assign[0])
				{
					case '+':
					case '-':
						iMediaPosition += atoi(sValue_To_Assign.c_str())*1000;
						break;
					default:
						iMediaPosition = atoi(sValue_To_Assign.c_str())*1000;
						break;
				}
				
				if (iMediaPosition>=totalTime)
					iMediaPosition = totalTime-1*1000;
				if (iMediaPosition<=0)
					iMediaPosition = 0;
				
				g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Change_Playback_Speed() jumping to position %i", iMediaPosition);
				pStream->Seek( iMediaPosition, 0 );
				pStream->changePlaybackSpeed(Xine_Stream::PLAYBACK_NORMAL);
		}
		else
		{
			g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Jump_to_Position_in_Stream() - empty argument, skipping");
		}
	}
}

//<-dceag-c63-b->

	/** @brief COMMAND: #63 - Skip Fwd - Channel/Track Greater */
	/** Raise  the channel, track, station, etc. by 1.  Same as Jump to Pos in Playlist with value +1 for a smart media player */

void Xine_Player::CMD_Skip_Fwd_ChannelTrack_Greater(string &sCMD_Result,Message *pMessage)
//<-dceag-c63-e->
{
	//HACK we need to know stream ID here
	Xine_Stream *pStream =  ptrFactory->GetStream( 0 );
	
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Skip_Fwd_ChannelTrack_Greater() with corresponding stream %p.", pStream);
	
	if (pStream==NULL)
	{
		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Skip_Fwd_ChannelTrack_Greater() stream is NULL, aborting - init failure?");
		return;
	}
	else
	{
		pStream->changePlaybackSpeed( Xine_Stream::PLAYBACK_NORMAL );
		pStream->sendInputEvent( XINE_EVENT_INPUT_NEXT );
	}
}

//<-dceag-c64-b->

	/** @brief COMMAND: #64 - Skip Back - Channel/Track Lower */
	/** Lower the channel, track, station, etc. by 1.  Same as Jump to Pos in Playlist with value -1 for a smart media player */

void Xine_Player::CMD_Skip_Back_ChannelTrack_Lower(string &sCMD_Result,Message *pMessage)
//<-dceag-c64-e->
{
	//HACK we need to know stream ID here
	Xine_Stream *pStream =  ptrFactory->GetStream( 0 );
	
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Skip_Fwd_ChannelTrack_Lower() with corresponding stream %p.", pStream);
	
	if (pStream==NULL)
	{
		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Skip_Fwd_ChannelTrack_Lower() stream is NULL, aborting - init failure?");
		return;
	}
	else
	{
		pStream->changePlaybackSpeed( Xine_Stream::PLAYBACK_NORMAL );
		pStream->sendInputEvent( XINE_EVENT_INPUT_PREVIOUS );
	}
}

//<-dceag-c65-b->

	/** @brief COMMAND: #65 - Jump Position In Playlist */
	/** Jump to a specific position in the playlist, or a track, or a chapter.  Smart media players should also understand the skip fwd/skip back (which non-DCE media players use) to be the same thing as a jump +1 or -1 */
		/** @param #5 Value To Assign */
			/** The track to go to.  A number is considered an absolute.  "+2" means forward 2, "-1" means back 1. */

void Xine_Player::CMD_Jump_Position_In_Playlist(string sValue_To_Assign,string &sCMD_Result,Message *pMessage)
//<-dceag-c65-e->
{
	if( sValue_To_Assign.size()==0 )
		return;  // Nothing to do
	
	//HACK we need to know stream ID here
	Xine_Stream *pStream =  ptrFactory->GetStream( 0 );
	
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Jump_Position_In_Playlist() with corresponding stream %p.", pStream);
	
	if (pStream==NULL)
	{
		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Jump_Position_In_Playlist() stream is NULL, aborting - init failure?");
		return;
	}
	else
	{
		pStream->changePlaybackSpeed( Xine_Stream::PLAYBACK_NORMAL);
		
		int ChaptersToSkip;
		if( sValue_To_Assign[0]=='-' || sValue_To_Assign[0]=='+' )
			ChaptersToSkip = atoi(sValue_To_Assign.c_str());
		else if( pStream->m_iChapter==-1 )
			ChaptersToSkip=1;
		else
			ChaptersToSkip = atoi(sValue_To_Assign.c_str())-pStream->m_iChapter;

		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::Skipping %d chapters",ChaptersToSkip);

		if( ChaptersToSkip<0 )
			for(int i=0;i>ChaptersToSkip;i--)
				pStream->sendInputEvent(XINE_EVENT_INPUT_PREVIOUS);
		else
			for(int i=0;i<ChaptersToSkip;i++)
				pStream->sendInputEvent(XINE_EVENT_INPUT_NEXT);
	}
	
}

//<-dceag-c81-b->

	/** @brief COMMAND: #81 - Navigate Next */
	/** Nagivate to the next possible navigable area. (The actual outcome depends on the specifc device) */
		/** @param #41 StreamID */
			/** The stream on which to do the navigation. */

void Xine_Player::CMD_Navigate_Next(int iStreamID,string &sCMD_Result,Message *pMessage)
//<-dceag-c81-e->
{
	Xine_Stream *pStream =  ptrFactory->GetStream( iStreamID );
	
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Navigate_Next() with corresponding stream %p.", pStream);
	
	if (pStream==NULL)
	{
		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Navigate_Next() stream is NULL, aborting - init failure?");
		return;
	}
	else
		pStream->selectNextButton();
}

//<-dceag-c82-b->

	/** @brief COMMAND: #82 - Navigate Prev */
	/** Nagivate the previous possible navigable area. (The actual outcome depends on the specific device). */
		/** @param #41 StreamID */
			/** The stream on which to do the navigation. */

void Xine_Player::CMD_Navigate_Prev(int iStreamID,string &sCMD_Result,Message *pMessage)
//<-dceag-c82-e->
{
	Xine_Stream *pStream =  ptrFactory->GetStream( iStreamID );
	
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Navigate_Prev() with corresponding stream %p.", pStream);
	
	if (pStream==NULL)
	{
		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Navigate_Prev() stream is NULL, aborting - init failure?");
		return;
	}
	else
		pStream->selectPrevButton();
}

//<-dceag-c84-b->

	/** @brief COMMAND: #84 - Get Video Frame */
	/** Get's the current video frame from the media player. */
		/** @param #19 Data */
			/** The video frame */
		/** @param #20 Format */
			/** Format of the frame */
		/** @param #23 Disable Aspect Lock */
			/** Disable Aspect Ratio */
		/** @param #41 StreamID */
			/** The ID of the stream */
		/** @param #60 Width */
			/** Frame width */
		/** @param #61 Height */
			/** Frame height */

void Xine_Player::CMD_Get_Video_Frame(string sDisable_Aspect_Lock,int iStreamID,int iWidth,int iHeight,char **pData,int *iData_Size,string *sFormat,string &sCMD_Result,Message *pMessage)
//<-dceag-c84-e->
{
	Xine_Stream *pStream =  ptrFactory->GetStream( iStreamID );
	
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Get_Video_Frame() with corresponding stream %p.", pStream);
	
	if (pStream==NULL)
	{
		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Get_Video_Frame() stream is NULL, aborting - init failure?");
		return;
	}
	else
		pStream->getScreenShot(iWidth, iHeight, *pData, *iData_Size, *sFormat, sCMD_Result);
}

//<-dceag-c87-b->

	/** @brief COMMAND: #87 - Goto Media Menu */
	/** Goto to the current media Root Menu. */
		/** @param #41 StreamID */
			/** The stream ID */
		/** @param #64 MenuType */
			/** The type of menu that the user want to jump to.
(For DVD handlers usually this applies)
0 - Root menu 
1 - Title menu
2 - Media menu */

void Xine_Player::CMD_Goto_Media_Menu(int iStreamID,int iMenuType,string &sCMD_Result,Message *pMessage)
//<-dceag-c87-e->
{
	Xine_Stream *pStream =  ptrFactory->GetStream( iStreamID );
	
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Goto_Media_Menu() with corresponding stream %p.", pStream);
	
	if (pStream==NULL)
	{
		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Goto_Media_Menu() stream is NULL, aborting - init failure?");
		return;
	}
	else
	{
		if  ( iMenuType != 0 && iMenuType != 1 && iMenuType != 2 )
			iMenuType = 0;

		pStream->selectMenu( iMenuType );
	}
}


//<-dceag-c92-b->

	/** @brief COMMAND: #92 - Pause */
	/** Pause the media */

void Xine_Player::CMD_Pause(string &sCMD_Result,Message *pMessage)
//<-dceag-c92-e->
{
	CMD_Pause_Media(0);
}

//<-dceag-c95-b->

	/** @brief COMMAND: #95 - Stop */
	/** Stop the media */
		/** @param #203 Eject */
			/** If true, the drive will be ejected if there is no media currently playing, so a remote's stop button acts as stop/eject. */

void Xine_Player::CMD_Stop(bool bEject,string &sCMD_Result,Message *pMessage)
//<-dceag-c95-e->
{
	string sMediaPosition;
	CMD_Stop_Media(0, &sMediaPosition);
 }

//<-dceag-c139-b->

	/** @brief COMMAND: #139 - Play */
	/** Play the media */

void Xine_Player::CMD_Play(string &sCMD_Result,Message *pMessage)
//<-dceag-c139-e->
{
	CMD_Play_Media( 0, 1, "", "");
}

//<-dceag-c140-b->

	/** @brief COMMAND: #140 - Audio Track */
	/** Go to an audio track */
		/** @param #5 Value To Assign */
			/** The audio track to go to.  Simple A/V equipment ignores this and just toggles. */

void Xine_Player::CMD_Audio_Track(string sValue_To_Assign,string &sCMD_Result,Message *pMessage)
//<-dceag-c140-e->
{
	//HACK we need to know stream ID here
	Xine_Stream *pStream =  ptrFactory->GetStream( 0 );
	
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Audio_Track() with corresponding stream %p.", pStream);
	
	if (pStream==NULL)
	{
		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Audio_Track() stream is NULL, aborting - init failure?");
		return;
	}
	else
		pStream->setAudio( atoi(sValue_To_Assign.c_str()) );
}

//<-dceag-c141-b->

	/** @brief COMMAND: #141 - Subtitle */
	/** Go to a subtitle */
		/** @param #5 Value To Assign */
			/** The subtitle to go to.  Simple A/V equipment ignores this and just toggles. */

void Xine_Player::CMD_Subtitle(string sValue_To_Assign,string &sCMD_Result,Message *pMessage)
//<-dceag-c141-e->
{
	//HACK we need to know stream ID here
	Xine_Stream *pStream =  ptrFactory->GetStream( 0 );
	
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Subtitle() with corresponding stream %p.", pStream);
	
	if (pStream==NULL)
	{
		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Audio_Track() stream is NULL, aborting - init failure?");
		return;
	}
	else
		pStream->setSubtitle( atoi(sValue_To_Assign.c_str()) );
}

//<-dceag-c142-b->

	/** @brief COMMAND: #142 - Angle */
	/** Go to an angle */
		/** @param #5 Value To Assign */
			/** The angle to go to.  Simple A/V equipment ignores this and just toggles. */

void Xine_Player::CMD_Angle(string sValue_To_Assign,string &sCMD_Result,Message *pMessage)
//<-dceag-c142-e->
{
	//HACK we need to know stream ID here
	Xine_Stream *pStream =  ptrFactory->GetStream( 0 );
	
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Angle() with corresponding stream %p.", pStream);
	
	if (pStream==NULL)
	{
		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Angle() stream is NULL, aborting - init failure?");
		return;
	}
	else
		pStream->setAudio( atoi(sValue_To_Assign.c_str()) );
}

//<-dceag-c190-b->

	/** @brief COMMAND: #190 - Enter/Go */
	/** Select the currently highlighted menu item */

void Xine_Player::CMD_EnterGo(string &sCMD_Result,Message *pMessage)
//<-dceag-c190-e->
{
	//HACK we need to know stream ID here
	Xine_Stream *pStream =  ptrFactory->GetStream( 0 );
	
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_EnterGo() with corresponding stream %p.", pStream);
	
	if (pStream==NULL)
	{
		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_EnterGo() stream is NULL, aborting - init failure?");
		return;
	}
	else
		pStream->sendInputEvent( XINE_EVENT_INPUT_SELECT );
}

//<-dceag-c200-b->

	/** @brief COMMAND: #200 - Move Up */
	/** Move the highlighter */

void Xine_Player::CMD_Move_Up(string &sCMD_Result,Message *pMessage)
//<-dceag-c200-e->
{
	//HACK we need to know stream ID here
	Xine_Stream *pStream =  ptrFactory->GetStream( 0 );
	
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Move_Up() with corresponding stream %p.", pStream);
	
	if (pStream==NULL)
	{
		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Move_Up() stream is NULL, aborting - init failure?");
		return;
	}
	else
		pStream->sendInputEvent( XINE_EVENT_INPUT_UP );
}

//<-dceag-c201-b->

	/** @brief COMMAND: #201 - Move Down */
	/** Move the highlighter */

void Xine_Player::CMD_Move_Down(string &sCMD_Result,Message *pMessage)
//<-dceag-c201-e->
{
	//HACK we need to know stream ID here
	Xine_Stream *pStream =  ptrFactory->GetStream( 0 );
	
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Move_Down() with corresponding stream %p.", pStream);
	
	if (pStream==NULL)
	{
		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Move_Down() stream is NULL, aborting - init failure?");
		return;
	}
	else
		pStream->sendInputEvent( XINE_EVENT_INPUT_DOWN );
}

//<-dceag-c202-b->

	/** @brief COMMAND: #202 - Move Left */
	/** Move the highlighter */

void Xine_Player::CMD_Move_Left(string &sCMD_Result,Message *pMessage)
//<-dceag-c202-e->
{
	//HACK we need to know stream ID here
	Xine_Stream *pStream =  ptrFactory->GetStream( 0 );
	
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Move_Left() with corresponding stream %p.", pStream);
	
	if (pStream==NULL)
	{
		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Move_Left() stream is NULL, aborting - init failure?");
		return;
	}
	else
		pStream->sendInputEvent( XINE_EVENT_INPUT_LEFT );
}

//<-dceag-c203-b->

	/** @brief COMMAND: #203 - Move Right */
	/** Move the highlighter */

void Xine_Player::CMD_Move_Right(string &sCMD_Result,Message *pMessage)
//<-dceag-c203-e->
{
	//HACK we need to know stream ID here
	Xine_Stream *pStream =  ptrFactory->GetStream( 0 );
	
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Move_Right() with corresponding stream %p.", pStream);
	
	if (pStream==NULL)
	{
		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Move_Right() stream is NULL, aborting - init failure?");
		return;
	}
	else
		pStream->sendInputEvent( XINE_EVENT_INPUT_RIGHT );
}

//<-dceag-c204-b->

	/** @brief COMMAND: #204 - 0 */
	/** 0 */

void Xine_Player::CMD_0(string &sCMD_Result,Message *pMessage)
//<-dceag-c204-e->
{
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_0() Not implemented!");
}

//<-dceag-c205-b->

	/** @brief COMMAND: #205 - 1 */
	/** 1 */

void Xine_Player::CMD_1(string &sCMD_Result,Message *pMessage)
//<-dceag-c205-e->
{
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_1() Not implemented!");
}

//<-dceag-c206-b->

	/** @brief COMMAND: #206 - 2 */
	/** 2 */

void Xine_Player::CMD_2(string &sCMD_Result,Message *pMessage)
//<-dceag-c206-e->
{
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_2() Not implemented!");
}

//<-dceag-c207-b->

	/** @brief COMMAND: #207 - 3 */
	/** 3 */

void Xine_Player::CMD_3(string &sCMD_Result,Message *pMessage)
//<-dceag-c207-e->
{
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_3() Not implemented!");
}

//<-dceag-c208-b->

	/** @brief COMMAND: #208 - 4 */
	/** 4 */

void Xine_Player::CMD_4(string &sCMD_Result,Message *pMessage)
//<-dceag-c208-e->
{
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_4() Not implemented!");
}

//<-dceag-c209-b->

	/** @brief COMMAND: #209 - 5 */
	/** 5 */

void Xine_Player::CMD_5(string &sCMD_Result,Message *pMessage)
//<-dceag-c209-e->
{
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_5() Not implemented!");
}

//<-dceag-c210-b->

	/** @brief COMMAND: #210 - 6 */
	/** 6 */

void Xine_Player::CMD_6(string &sCMD_Result,Message *pMessage)
//<-dceag-c210-e->
{
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_6() Not implemented!");
}

//<-dceag-c211-b->

	/** @brief COMMAND: #211 - 7 */
	/** 7 */

void Xine_Player::CMD_7(string &sCMD_Result,Message *pMessage)
//<-dceag-c211-e->
{
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_7() Not implemented!");
}

//<-dceag-c212-b->

	/** @brief COMMAND: #212 - 8 */
	/** 8 */

void Xine_Player::CMD_8(string &sCMD_Result,Message *pMessage)
//<-dceag-c212-e->
{
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_8() Not implemented!");
}

//<-dceag-c213-b->

	/** @brief COMMAND: #213 - 9 */
	/** 9 */

void Xine_Player::CMD_9(string &sCMD_Result,Message *pMessage)
//<-dceag-c213-e->
{
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_9() Not implemented!");
}

//<-dceag-c240-b->

	/** @brief COMMAND: #240 - Back / Prior Menu */
	/** Navigate back .. ( Escape ) */

void Xine_Player::CMD_Back_Prior_Menu(string &sCMD_Result,Message *pMessage)
//<-dceag-c240-e->
{
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Back_Prior_Menu() Not implemented!");
}

//<-dceag-c259-b->

	/** @brief COMMAND: #259 - Report Playback Position */
	/** This will report the playback position of the current stream. */
		/** @param #9 Text */
			/** A human readable representation of the current position */
		/** @param #41 StreamID */
			/** The stream ID on which to report the position. */
		/** @param #42 MediaPosition */
			/** A media player readable representation of the current position including all options */

void Xine_Player::CMD_Report_Playback_Position(int iStreamID,string *sText,string *sMediaPosition,string &sCMD_Result,Message *pMessage)
//<-dceag-c259-e->
{
	Xine_Stream *pStream =  ptrFactory->GetStream( iStreamID );
	
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Report_Playback_Position() with corresponding stream %p.", pStream);
	
	if (pStream==NULL)
	{
		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Report_Playback_Position() stream is NULL, aborting - init failure?");
		return;
	}
	else
		*sMediaPosition = pStream->GetPosition();		
}

//<-dceag-c412-b->

	/** @brief COMMAND: #412 - Set Media Position */
	/** Jump to a certain media position */
		/** @param #41 StreamID */
			/** The stream to set */
		/** @param #42 MediaPosition */
			/** The media position.  When MediaPlugin gets this, it will be a bookmark ID, when a media player gets it, the string */

void Xine_Player::CMD_Set_Media_Position(int iStreamID,string sMediaPosition,string &sCMD_Result,Message *pMessage)
//<-dceag-c412-e->
{
	Xine_Stream *pStream =  ptrFactory->GetStream( iStreamID );	
	
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Set_Media_Position() called for filename: %s (%s) with stream %p.", 
			      (pStream!=NULL)?(pStream->m_sCurrentFile.c_str()):"",sMediaPosition.c_str(),pStream);
	
	if(	sMediaPosition.substr(0,5)==" POS:" && sMediaPosition.size()<15 ) // See if there's only a POS and nothing else.  If so, just do a seek
	{
		int Position = atoi(sMediaPosition.substr(5).c_str());
	
		if (pStream==NULL)
		{
			g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Set_Media_Position() stream is NULL, aborting - init failure?");
			return;
		}
		else
		{
			g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Set_Media_Position() just a seek within current called for filename: %s (%s) with stream %p. to pos %d", pStream->m_sCurrentFile.c_str(),sMediaPosition.c_str(),pStream,Position);
			pStream->changePlaybackSpeed( Xine_Stream::PLAYBACK_NORMAL );
			pStream->Seek( Position,0 );
			pStream->DisplaySpeedAndTimeCode();
			return;
		}
	}
	if (pStream)
	{
		// as we are changing position, then do not report about
		pStream->m_bDontReportCompletion = true;
		CMD_Play_Media(0,iStreamID,sMediaPosition,pStream->m_sCurrentFile,sCMD_Result,pMessage);
		pStream->m_bDontReportCompletion = false;		
	}
}

//<-dceag-c548-b->

	/** @brief COMMAND: #548 - Menu */
	/** Show a menu associated with this media */
		/** @param #9 Text */
			/** A string indicating which menu should appear.  The parameter is only used for smart media devices */

void Xine_Player::CMD_Menu(string sText,string &sCMD_Result,Message *pMessage)
//<-dceag-c548-e->
{
	CMD_Goto_Media_Menu(0,0);
}

void Xine_Player::ReportTimecodeViaIP(int iStreamID, int Speed)
{
	Xine_Stream *pStream =  ptrFactory->GetStream( iStreamID, false );	
	if (pStream == NULL)
	{
		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::ReportTimecodeViaIP() stream is NULL");
		return;
	}

	if( !m_pDeviceData_MediaPlugin )
		return;

	g_pPlutoLogger->Write(LV_WARNING,"reporting timecode");
	int currentTime, totalTime;	
	int iMediaPosition = pStream->getStreamPlaybackPosition( currentTime, totalTime);

	// IP speed an position notification
	char buffer_current[32];
	snprintf(buffer_current, 32, "%02i:%02i:%02i.%03i", 
		iMediaPosition / 1000 / 3600, iMediaPosition / 1000 % 3600 / 60, iMediaPosition / 1000 % 3600 % 60, iMediaPosition % 1000);

	char buffer_total[32];
	snprintf(buffer_total, 32, "%02i:%02i:%02i.%03i", 
		totalTime / 1000 / 3600, totalTime / 1000 % 3600 / 60, totalTime / 1000 % 3600 % 60, totalTime % 1000);

	string sIPTimeCodeInfo = StringUtils::itos(Speed) + "," + buffer_current + "," + buffer_total + "," + StringUtils::itos(iStreamID)
		+ "," + StringUtils::itos(pStream->m_iTitle) + "," + StringUtils::itos(pStream->m_iChapter);
	
	m_pNotificationSocket->SendStringToAll( sIPTimeCodeInfo );
}


bool Xine_Player::Connect(int iPK_DeviceTemplate )
{
    if ( ! Command_Impl::Connect(iPK_DeviceTemplate) )
		return false;

	DeviceData_Base *pDevice = m_pData->GetTopMostDevice();
	m_sIPofMD = pDevice->m_sIPAddress;

	if (!ptrFactory->StartupFactory())
		return false;

	int iPort = DATA_Get_Port();
	g_pPlutoLogger->Write(LV_STATUS, "Configured port for time/speed notification is: %i, IP is %s", iPort, m_sIPofMD.c_str());

	m_pNotificationSocket->StartListening (iPort);

	return true;
}

void Xine_Player::FireMenuOnScreen(int iDestinationDevice, int iStreamID, bool bOnOff)
{
    g_pPlutoLogger->Write(LV_STATUS, "Sending Menu on screen event %s for stream %d", bOnOff ? "on" : "off", iStreamID);
    g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::EVENT_Menu_Onscreen(streamID=%i)", iStreamID);
    EVENT_Menu_Onscreen(iStreamID, bOnOff);
}

string Xine_Player::Get_MD_AudioSettings()
{
	// M/D where we are attached to
	int PK_MD = m_pData->m_dwPK_Device_MD;

	return m_pData->m_pEvent_Impl->GetDeviceDataFromDatabase(PK_MD,DEVICEDATA_Audio_settings_CONST);
}
//<-dceag-c249-b->

	/** @brief COMMAND: #249 - Start Streaming */
	/** Like play media, but it means the destination device is not the same as the source */
		/** @param #29 PK_MediaType */
			/** The type of media */
		/** @param #41 StreamID */
			/** Identifier for this streaming session. */
		/** @param #42 MediaPosition */
			/** Where to start playing from */
		/** @param #59 MediaURL */
			/** The url to use to play this stream. */
		/** @param #105 StreamingTargets */
			/** Target destinations for streaming. Semantics dependent on the target device. */

void Xine_Player::CMD_Start_Streaming(int iPK_MediaType,int iStreamID,string sMediaPosition,string sMediaURL,string sStreamingTargets,string &sCMD_Result,Message *pMessage)
//<-dceag-c249-e->
{
	// streaming logic:
	// locally the broadcasting stream is created, with performing output to devnull (plus to socket)
	// CMD_Play_Media is sent to each streaming target
	// 
	// as all playback control commands will be sent only to the local xine, 
	// then we need to control "stream as a whole"
	
	Xine_Stream *pStream =  ptrFactory->GetStream( iStreamID, false );
	// if stream already exists - we must stop it and all corresponding targets
	if (pStream!=NULL)
	{
		string sPos;
		CMD_Stop_Media( iStreamID, &sPos);
		
		// if current instance of stream is not broadcaster, we must kill it and re-create
		if (!pStream->m_bBroadcaster)
		{
			ptrFactory->DestroyStream(iStreamID);
		}
	}
	
	pStream =  ptrFactory->GetStream( iStreamID, true, pMessage->m_dwPK_Device_From, true);
		
	if (pStream==NULL)
	{
		g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::CMD_Start_Streaming() stream is NULL, aborting - init failure?");
		return;
	}
	else
	{
		pStream->m_sBroadcastTargets = sStreamingTargets;
		CMD_Play_Media( iPK_MediaType, iStreamID, sMediaPosition, sMediaURL);
	}
	
}
