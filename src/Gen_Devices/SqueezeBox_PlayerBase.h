#ifndef SqueezeBox_PlayerBase_h
#define SqueezeBox_PlayerBase_h
#include "DeviceData_Impl.h"
#include "Message.h"
#include "Command_Impl.h"
#include "Logger.h"


namespace DCE
{
//   OUR EVENT CLASS 

class SqueezeBox_Player_Event : public Event_Impl
{
public:
	SqueezeBox_Player_Event(int DeviceID, string ServerAddress, bool bConnectEventHandler=true) : Event_Impl(DeviceID,58, ServerAddress, bConnectEventHandler) {};
	SqueezeBox_Player_Event(class ClientSocket *pOCClientSocket, int DeviceID) : Event_Impl(pOCClientSocket, DeviceID) {};
	//Events
	class Event_Impl *CreateEvent( unsigned long dwPK_DeviceTemplate, ClientSocket *pOCClientSocket, unsigned long dwDevice );
};


//   OUR DATA CLASS 

class SqueezeBox_Player_Data : public DeviceData_Impl
{
public:
	virtual ~SqueezeBox_Player_Data() {};
	class DeviceData_Impl *CreateData(DeviceData_Impl *Parent,char *pDataBlock,unsigned long AllocatedSize,char *CurrentPosition);
	virtual int GetPK_DeviceList() { return 58; } ;
	virtual const char *GetDeviceDescription() { return "SqueezeBox_Player"; } ;
	string Get_COM_Port_on_PC() { return m_mapParameters[37];}
};



//   OUR COMMAND CLASS 

class SqueezeBox_Player_Command : public Command_Impl
{
public:
	SqueezeBox_Player_Command(int DeviceID, string ServerAddress,bool bConnectEventHandler=true,bool bLocalMode=false,class Router *pRouter=NULL)
	: Command_Impl(DeviceID, ServerAddress, bLocalMode, pRouter)
	{
	}
	virtual bool GetConfig()
	{
		
		m_pData=NULL;
		m_pEvent = new SqueezeBox_Player_Event(m_dwPK_Device, m_sHostName, !m_bLocalMode);
		if( m_pEvent->m_dwPK_Device )
			m_dwPK_Device = m_pEvent->m_dwPK_Device;
		if( m_sIPAddress!=m_pEvent->m_pClientSocket->m_sIPAddress )	
			m_sIPAddress=m_pEvent->m_pClientSocket->m_sIPAddress;
		m_sMacAddress=m_pEvent->m_pClientSocket->m_sMacAddress;
		if( m_pEvent->m_pClientSocket->m_eLastError!=cs_err_None )
		{
			if( m_pEvent->m_pClientSocket->m_eLastError==cs_err_BadDevice )
			{
				while( m_pEvent->m_pClientSocket->m_eLastError==cs_err_BadDevice && (m_dwPK_Device = DeviceIdInvalid())!=0 )
				{
					delete m_pEvent;
					m_pEvent = new SqueezeBox_Player_Event(m_dwPK_Device, m_sHostName, !m_bLocalMode);
					if( m_pEvent->m_dwPK_Device )
						m_dwPK_Device = m_pEvent->m_dwPK_Device;
				}
			}
			if( m_pEvent->m_pClientSocket->m_eLastError==cs_err_NeedReload )
			{
				if( RouterNeedsReload() )
				{
					string sResponse;
					Event_Impl event_Impl(DEVICEID_MESSAGESEND, 0, m_sHostName);
					event_Impl.m_pClientSocket->SendString( "RELOAD" );
					if( !event_Impl.m_pClientSocket->ReceiveString( sResponse ) || sResponse!="OK" )
					{
						CannotReloadRouter();
						g_pPlutoLogger->Write(LV_WARNING,"Reload request denied: %s",sResponse.c_str());
					}
				Sleep(10000);  // Give the router 10 seconds before we re-attempt, otherwise we'll get an error right away
				}	
			}
		}
		
		if( m_bLocalMode )
		{
			m_pData = new SqueezeBox_Player_Data();
			return true;
		}
		if( m_pEvent->m_pClientSocket->m_eLastError!=cs_err_None || m_pEvent->m_pClientSocket->m_Socket==INVALID_SOCKET )
			return false;

		int Size; char *pConfig = m_pEvent->GetConfig(Size);
		if( !pConfig )
			return false;
		m_pData = new SqueezeBox_Player_Data();
		if( Size )
			m_pData->SerializeRead(Size,pConfig);
		delete[] pConfig;
		pConfig = m_pEvent->GetDeviceList(Size);
		m_pData->m_AllDevices.SerializeRead(Size,pConfig);
		delete[] pConfig;
		m_pData->m_pEvent_Impl = m_pEvent;
		m_pcRequestSocket = new Event_Impl(m_dwPK_Device, 58,m_sHostName);
		if( m_iInstanceID )
		{
			m_pEvent->m_pClientSocket->SendString("INSTANCE " + StringUtils::itos(m_iInstanceID));
			m_pcRequestSocket->m_pClientSocket->SendString("INSTANCE " + StringUtils::itos(m_iInstanceID));
		}
		return true;
	};
	SqueezeBox_Player_Command(Command_Impl *pPrimaryDeviceCommand, DeviceData_Impl *pData, Event_Impl *pEvent, Router *pRouter) : Command_Impl(pPrimaryDeviceCommand, pData, pEvent, pRouter) {};
	virtual ~SqueezeBox_Player_Command() {};
	SqueezeBox_Player_Event *GetEvents() { return (SqueezeBox_Player_Event *) m_pEvent; };
	SqueezeBox_Player_Data *GetData() { return (SqueezeBox_Player_Data *) m_pData; };
	const char *GetClassName() { return "SqueezeBox_Player_Command"; };
	virtual int PK_DeviceTemplate_get() { return 58; };
	static int PK_DeviceTemplate_get_static() { return 58; };
	virtual void ReceivedCommandForChild(DeviceData_Impl *pDeviceData_Impl,string &sCMD_Result,Message *pMessage) { };
	virtual void ReceivedUnknownCommand(string &sCMD_Result,Message *pMessage) { };
	Command_Impl *CreateCommand(int PK_DeviceTemplate, Command_Impl *pPrimaryDeviceCommand, DeviceData_Impl *pData, Event_Impl *pEvent);
	//Data accessors
	string DATA_Get_COM_Port_on_PC() { return GetData()->Get_COM_Port_on_PC(); }
	//Event accessors
	//Commands - Override these to handle commands from the server
	virtual void CMD_Play_Media(string sFilename,int iPK_MediaType,int iStreamID,string sMediaPosition,string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Stop_Media(int iStreamID,string *sMediaPosition,string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Pause_Media(int iStreamID,string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Restart_Media(int iStreamID,string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Change_Playback_Speed(int iStreamID,int iMediaPlaybackSpeed,string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Jump_to_Position_in_Stream(string sValue_To_Assign,int iStreamID,string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Jump_Position_In_Playlist(string sValue_To_Assign,string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Report_Playback_Position(int iStreamID,string *sText,string *sMediaPosition,string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Set_Media_Position(int iStreamID,string sMediaPosition,string &sCMD_Result,class Message *pMessage) {};

	//This distributes a received message to your handler.
	virtual bool ReceivedMessage(class Message *pMessageOriginal)
	{
		map<long, string>::iterator itRepeat;
		if( Command_Impl::ReceivedMessage(pMessageOriginal) )
			return true;
		int iHandled=0;
		for(int s=-1;s<(int) pMessageOriginal->m_vectExtraMessages.size(); ++s)
		{
			Message *pMessage = s>=0 ? pMessageOriginal->m_vectExtraMessages[s] : pMessageOriginal;
			if (pMessage->m_dwPK_Device_To==m_dwPK_Device && pMessage->m_dwMessage_Type == MESSAGETYPE_COMMAND)
			{
				switch(pMessage->m_dwID)
				{
				case 37:
					{
						string sCMD_Result="OK";
					string sFilename=pMessage->m_mapParameters[13];
					int iPK_MediaType=atoi(pMessage->m_mapParameters[29].c_str());
					int iStreamID=atoi(pMessage->m_mapParameters[41].c_str());
					string sMediaPosition=pMessage->m_mapParameters[42];
						CMD_Play_Media(sFilename.c_str(),iPK_MediaType,iStreamID,sMediaPosition.c_str(),sCMD_Result,pMessage);
						if( pMessage->m_eExpectedResponse==ER_ReplyMessage && !pMessage->m_bRespondedToMessage )
						{
							pMessage->m_bRespondedToMessage=true;
							Message *pMessageOut=new Message(m_dwPK_Device,pMessage->m_dwPK_Device_From,PRIORITY_NORMAL,MESSAGETYPE_REPLY,0,0);
							pMessageOut->m_mapParameters[0]=sCMD_Result;
							SendMessage(pMessageOut);
						}
						else if( (pMessage->m_eExpectedResponse==ER_DeliveryConfirmation || pMessage->m_eExpectedResponse==ER_ReplyString) && !pMessage->m_bRespondedToMessage )
						{
							pMessage->m_bRespondedToMessage=true;
							SendString(sCMD_Result);
						}
						if( (itRepeat=pMessage->m_mapParameters.find(72))!=pMessage->m_mapParameters.end() )
						{
							int iRepeat=atoi(pMessage->m_mapParameters[72].c_str());
							for(int i=2;i<=iRepeat;++i)
								CMD_Play_Media(sFilename.c_str(),iPK_MediaType,iStreamID,sMediaPosition.c_str(),sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 38:
					{
						string sCMD_Result="OK";
					int iStreamID=atoi(pMessage->m_mapParameters[41].c_str());
					string sMediaPosition=pMessage->m_mapParameters[42];
						CMD_Stop_Media(iStreamID,&sMediaPosition,sCMD_Result,pMessage);
						if( pMessage->m_eExpectedResponse==ER_ReplyMessage && !pMessage->m_bRespondedToMessage )
						{
							pMessage->m_bRespondedToMessage=true;
							Message *pMessageOut=new Message(m_dwPK_Device,pMessage->m_dwPK_Device_From,PRIORITY_NORMAL,MESSAGETYPE_REPLY,0,0);
						pMessageOut->m_mapParameters[42]=sMediaPosition;
							pMessageOut->m_mapParameters[0]=sCMD_Result;
							SendMessage(pMessageOut);
						}
						else if( (pMessage->m_eExpectedResponse==ER_DeliveryConfirmation || pMessage->m_eExpectedResponse==ER_ReplyString) && !pMessage->m_bRespondedToMessage )
						{
							pMessage->m_bRespondedToMessage=true;
							SendString(sCMD_Result);
						}
						if( (itRepeat=pMessage->m_mapParameters.find(72))!=pMessage->m_mapParameters.end() )
						{
							int iRepeat=atoi(pMessage->m_mapParameters[72].c_str());
							for(int i=2;i<=iRepeat;++i)
								CMD_Stop_Media(iStreamID,&sMediaPosition,sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 39:
					{
						string sCMD_Result="OK";
					int iStreamID=atoi(pMessage->m_mapParameters[41].c_str());
						CMD_Pause_Media(iStreamID,sCMD_Result,pMessage);
						if( pMessage->m_eExpectedResponse==ER_ReplyMessage && !pMessage->m_bRespondedToMessage )
						{
							pMessage->m_bRespondedToMessage=true;
							Message *pMessageOut=new Message(m_dwPK_Device,pMessage->m_dwPK_Device_From,PRIORITY_NORMAL,MESSAGETYPE_REPLY,0,0);
							pMessageOut->m_mapParameters[0]=sCMD_Result;
							SendMessage(pMessageOut);
						}
						else if( (pMessage->m_eExpectedResponse==ER_DeliveryConfirmation || pMessage->m_eExpectedResponse==ER_ReplyString) && !pMessage->m_bRespondedToMessage )
						{
							pMessage->m_bRespondedToMessage=true;
							SendString(sCMD_Result);
						}
						if( (itRepeat=pMessage->m_mapParameters.find(72))!=pMessage->m_mapParameters.end() )
						{
							int iRepeat=atoi(pMessage->m_mapParameters[72].c_str());
							for(int i=2;i<=iRepeat;++i)
								CMD_Pause_Media(iStreamID,sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 40:
					{
						string sCMD_Result="OK";
					int iStreamID=atoi(pMessage->m_mapParameters[41].c_str());
						CMD_Restart_Media(iStreamID,sCMD_Result,pMessage);
						if( pMessage->m_eExpectedResponse==ER_ReplyMessage && !pMessage->m_bRespondedToMessage )
						{
							pMessage->m_bRespondedToMessage=true;
							Message *pMessageOut=new Message(m_dwPK_Device,pMessage->m_dwPK_Device_From,PRIORITY_NORMAL,MESSAGETYPE_REPLY,0,0);
							pMessageOut->m_mapParameters[0]=sCMD_Result;
							SendMessage(pMessageOut);
						}
						else if( (pMessage->m_eExpectedResponse==ER_DeliveryConfirmation || pMessage->m_eExpectedResponse==ER_ReplyString) && !pMessage->m_bRespondedToMessage )
						{
							pMessage->m_bRespondedToMessage=true;
							SendString(sCMD_Result);
						}
						if( (itRepeat=pMessage->m_mapParameters.find(72))!=pMessage->m_mapParameters.end() )
						{
							int iRepeat=atoi(pMessage->m_mapParameters[72].c_str());
							for(int i=2;i<=iRepeat;++i)
								CMD_Restart_Media(iStreamID,sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 41:
					{
						string sCMD_Result="OK";
					int iStreamID=atoi(pMessage->m_mapParameters[41].c_str());
					int iMediaPlaybackSpeed=atoi(pMessage->m_mapParameters[43].c_str());
						CMD_Change_Playback_Speed(iStreamID,iMediaPlaybackSpeed,sCMD_Result,pMessage);
						if( pMessage->m_eExpectedResponse==ER_ReplyMessage && !pMessage->m_bRespondedToMessage )
						{
							pMessage->m_bRespondedToMessage=true;
							Message *pMessageOut=new Message(m_dwPK_Device,pMessage->m_dwPK_Device_From,PRIORITY_NORMAL,MESSAGETYPE_REPLY,0,0);
							pMessageOut->m_mapParameters[0]=sCMD_Result;
							SendMessage(pMessageOut);
						}
						else if( (pMessage->m_eExpectedResponse==ER_DeliveryConfirmation || pMessage->m_eExpectedResponse==ER_ReplyString) && !pMessage->m_bRespondedToMessage )
						{
							pMessage->m_bRespondedToMessage=true;
							SendString(sCMD_Result);
						}
						if( (itRepeat=pMessage->m_mapParameters.find(72))!=pMessage->m_mapParameters.end() )
						{
							int iRepeat=atoi(pMessage->m_mapParameters[72].c_str());
							for(int i=2;i<=iRepeat;++i)
								CMD_Change_Playback_Speed(iStreamID,iMediaPlaybackSpeed,sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 42:
					{
						string sCMD_Result="OK";
					string sValue_To_Assign=pMessage->m_mapParameters[5];
					int iStreamID=atoi(pMessage->m_mapParameters[41].c_str());
						CMD_Jump_to_Position_in_Stream(sValue_To_Assign.c_str(),iStreamID,sCMD_Result,pMessage);
						if( pMessage->m_eExpectedResponse==ER_ReplyMessage && !pMessage->m_bRespondedToMessage )
						{
							pMessage->m_bRespondedToMessage=true;
							Message *pMessageOut=new Message(m_dwPK_Device,pMessage->m_dwPK_Device_From,PRIORITY_NORMAL,MESSAGETYPE_REPLY,0,0);
							pMessageOut->m_mapParameters[0]=sCMD_Result;
							SendMessage(pMessageOut);
						}
						else if( (pMessage->m_eExpectedResponse==ER_DeliveryConfirmation || pMessage->m_eExpectedResponse==ER_ReplyString) && !pMessage->m_bRespondedToMessage )
						{
							pMessage->m_bRespondedToMessage=true;
							SendString(sCMD_Result);
						}
						if( (itRepeat=pMessage->m_mapParameters.find(72))!=pMessage->m_mapParameters.end() )
						{
							int iRepeat=atoi(pMessage->m_mapParameters[72].c_str());
							for(int i=2;i<=iRepeat;++i)
								CMD_Jump_to_Position_in_Stream(sValue_To_Assign.c_str(),iStreamID,sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 65:
					{
						string sCMD_Result="OK";
					string sValue_To_Assign=pMessage->m_mapParameters[5];
						CMD_Jump_Position_In_Playlist(sValue_To_Assign.c_str(),sCMD_Result,pMessage);
						if( pMessage->m_eExpectedResponse==ER_ReplyMessage && !pMessage->m_bRespondedToMessage )
						{
							pMessage->m_bRespondedToMessage=true;
							Message *pMessageOut=new Message(m_dwPK_Device,pMessage->m_dwPK_Device_From,PRIORITY_NORMAL,MESSAGETYPE_REPLY,0,0);
							pMessageOut->m_mapParameters[0]=sCMD_Result;
							SendMessage(pMessageOut);
						}
						else if( (pMessage->m_eExpectedResponse==ER_DeliveryConfirmation || pMessage->m_eExpectedResponse==ER_ReplyString) && !pMessage->m_bRespondedToMessage )
						{
							pMessage->m_bRespondedToMessage=true;
							SendString(sCMD_Result);
						}
						if( (itRepeat=pMessage->m_mapParameters.find(72))!=pMessage->m_mapParameters.end() )
						{
							int iRepeat=atoi(pMessage->m_mapParameters[72].c_str());
							for(int i=2;i<=iRepeat;++i)
								CMD_Jump_Position_In_Playlist(sValue_To_Assign.c_str(),sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 259:
					{
						string sCMD_Result="OK";
					int iStreamID=atoi(pMessage->m_mapParameters[41].c_str());
					string sText=pMessage->m_mapParameters[9];
					string sMediaPosition=pMessage->m_mapParameters[42];
						CMD_Report_Playback_Position(iStreamID,&sText,&sMediaPosition,sCMD_Result,pMessage);
						if( pMessage->m_eExpectedResponse==ER_ReplyMessage && !pMessage->m_bRespondedToMessage )
						{
							pMessage->m_bRespondedToMessage=true;
							Message *pMessageOut=new Message(m_dwPK_Device,pMessage->m_dwPK_Device_From,PRIORITY_NORMAL,MESSAGETYPE_REPLY,0,0);
						pMessageOut->m_mapParameters[9]=sText;
						pMessageOut->m_mapParameters[42]=sMediaPosition;
							pMessageOut->m_mapParameters[0]=sCMD_Result;
							SendMessage(pMessageOut);
						}
						else if( (pMessage->m_eExpectedResponse==ER_DeliveryConfirmation || pMessage->m_eExpectedResponse==ER_ReplyString) && !pMessage->m_bRespondedToMessage )
						{
							pMessage->m_bRespondedToMessage=true;
							SendString(sCMD_Result);
						}
						if( (itRepeat=pMessage->m_mapParameters.find(72))!=pMessage->m_mapParameters.end() )
						{
							int iRepeat=atoi(pMessage->m_mapParameters[72].c_str());
							for(int i=2;i<=iRepeat;++i)
								CMD_Report_Playback_Position(iStreamID,&sText,&sMediaPosition,sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 412:
					{
						string sCMD_Result="OK";
					int iStreamID=atoi(pMessage->m_mapParameters[41].c_str());
					string sMediaPosition=pMessage->m_mapParameters[42];
						CMD_Set_Media_Position(iStreamID,sMediaPosition.c_str(),sCMD_Result,pMessage);
						if( pMessage->m_eExpectedResponse==ER_ReplyMessage && !pMessage->m_bRespondedToMessage )
						{
							pMessage->m_bRespondedToMessage=true;
							Message *pMessageOut=new Message(m_dwPK_Device,pMessage->m_dwPK_Device_From,PRIORITY_NORMAL,MESSAGETYPE_REPLY,0,0);
							pMessageOut->m_mapParameters[0]=sCMD_Result;
							SendMessage(pMessageOut);
						}
						else if( (pMessage->m_eExpectedResponse==ER_DeliveryConfirmation || pMessage->m_eExpectedResponse==ER_ReplyString) && !pMessage->m_bRespondedToMessage )
						{
							pMessage->m_bRespondedToMessage=true;
							SendString(sCMD_Result);
						}
						if( (itRepeat=pMessage->m_mapParameters.find(72))!=pMessage->m_mapParameters.end() )
						{
							int iRepeat=atoi(pMessage->m_mapParameters[72].c_str());
							for(int i=2;i<=iRepeat;++i)
								CMD_Set_Media_Position(iStreamID,sMediaPosition.c_str(),sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				}
				iHandled += Command_Impl::ReceivedMessage(pMessage);
			}
			else if( pMessage->m_dwMessage_Type == MESSAGETYPE_COMMAND )
			{
				MapCommand_Impl::iterator it = m_mapCommandImpl_Children.find(pMessage->m_dwPK_Device_To);
				if( it!=m_mapCommandImpl_Children.end() && !(*it).second->m_bGeneric )
				{
					Command_Impl *pCommand_Impl = (*it).second;
					iHandled += pCommand_Impl->ReceivedMessage(pMessage);
			}
			else
			{
				DeviceData_Impl *pDeviceData_Impl = m_pData->FindChild(pMessage->m_dwPK_Device_To);
				string sCMD_Result="UNHANDLED";
				if( pDeviceData_Impl )
					ReceivedCommandForChild(pDeviceData_Impl,sCMD_Result,pMessage);
				else
					ReceivedUnknownCommand(sCMD_Result,pMessage);
					if( pMessage->m_eExpectedResponse==ER_ReplyMessage && !pMessage->m_bRespondedToMessage )
					{
							pMessage->m_bRespondedToMessage=true;
						Message *pMessageOut=new Message(m_dwPK_Device,pMessage->m_dwPK_Device_From,PRIORITY_NORMAL,MESSAGETYPE_REPLY,0,0);
						pMessageOut->m_mapParameters[0]=sCMD_Result;
						SendMessage(pMessageOut);
					}
					else if( (pMessage->m_eExpectedResponse==ER_DeliveryConfirmation || pMessage->m_eExpectedResponse==ER_ReplyString) && !pMessage->m_bRespondedToMessage )
						{
							pMessage->m_bRespondedToMessage=true;
						SendString(sCMD_Result);
						}
					if( sCMD_Result!="UNHANDLED" && sCMD_Result!="UNKNOWN DEVICE" )
						iHandled++;
				}
			}
			if( iHandled==0 && !pMessage->m_bRespondedToMessage &&
			(pMessage->m_eExpectedResponse==ER_ReplyMessage || pMessage->m_eExpectedResponse==ER_ReplyString) )
			{
				pMessage->m_bRespondedToMessage=true;
				if( pMessage->m_eExpectedResponse==ER_ReplyMessage )
				{
					Message *pMessageOut=new Message(m_dwPK_Device,pMessage->m_dwPK_Device_From,PRIORITY_NORMAL,MESSAGETYPE_REPLY,0,0);
					pMessageOut->m_mapParameters[0]="UNHANDLED";
					SendMessage(pMessageOut);
				}
				else
					SendString("UNHANDLED");
			}
		}
		return iHandled!=0;
	}
}; // end class


}
#endif
