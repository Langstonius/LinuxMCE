#ifndef Security_PluginBase_h
#define Security_PluginBase_h
#include "DeviceData_Impl.h"
#include "Message.h"
#include "Command_Impl.h"
#include "Logger.h"


namespace DCE
{
//   OUR EVENT CLASS 

class Security_Plugin_Event : public Event_Impl
{
public:
	Security_Plugin_Event(int DeviceID, string ServerAddress, bool bConnectEventHandler=true) : Event_Impl(DeviceID,33, ServerAddress, bConnectEventHandler) {};
	Security_Plugin_Event(class ClientSocket *pOCClientSocket, int DeviceID) : Event_Impl(pOCClientSocket, DeviceID) {};
	//Events
	class Event_Impl *CreateEvent( unsigned long dwPK_DeviceTemplate, ClientSocket *pOCClientSocket, unsigned long dwDevice );
	virtual void Security_Breach(int iPK_Device)
	{
		SendMessage(new Message(m_dwPK_Device, DEVICEID_EVENTMANAGER, PRIORITY_NORMAL, MESSAGETYPE_EVENT, 16,1,26,StringUtils::itos(iPK_Device).c_str()));
	}

	virtual void Fire_Alarm(int iPK_Device)
	{
		SendMessage(new Message(m_dwPK_Device, DEVICEID_EVENTMANAGER, PRIORITY_NORMAL, MESSAGETYPE_EVENT, 17,1,26,StringUtils::itos(iPK_Device).c_str()));
	}

	virtual void Reset_Alarm()
	{
		SendMessage(new Message(m_dwPK_Device, DEVICEID_EVENTMANAGER, PRIORITY_NORMAL, MESSAGETYPE_EVENT, 18,0));
	}

	virtual void Air_Quality(int iPK_Device)
	{
		SendMessage(new Message(m_dwPK_Device, DEVICEID_EVENTMANAGER, PRIORITY_NORMAL, MESSAGETYPE_EVENT, 39,1,26,StringUtils::itos(iPK_Device).c_str()));
	}

	virtual void Doorbell(int iPK_Device)
	{
		SendMessage(new Message(m_dwPK_Device, DEVICEID_EVENTMANAGER, PRIORITY_NORMAL, MESSAGETYPE_EVENT, 40,1,26,StringUtils::itos(iPK_Device).c_str()));
	}

	virtual void Monitor_Mode(int iPK_Device)
	{
		SendMessage(new Message(m_dwPK_Device, DEVICEID_EVENTMANAGER, PRIORITY_NORMAL, MESSAGETYPE_EVENT, 41,1,26,StringUtils::itos(iPK_Device).c_str()));
	}

	virtual void House_Mode_Changed(int iPK_DeviceGroup,int iPK_HouseMode)
	{
		SendMessage(new Message(m_dwPK_Device, DEVICEID_EVENTMANAGER, PRIORITY_NORMAL, MESSAGETYPE_EVENT, 43,2,38,StringUtils::itos(iPK_DeviceGroup).c_str(),39,StringUtils::itos(iPK_HouseMode).c_str()));
	}

};


//   OUR DATA CLASS 

class Security_Plugin_Data : public DeviceData_Impl
{
public:
	virtual ~Security_Plugin_Data() {};
	class DeviceData_Impl *CreateData(DeviceData_Impl *Parent,char *pDataBlock,unsigned long AllocatedSize,char *CurrentPosition);
	virtual int GetPK_DeviceList() { return 33; } ;
	virtual const char *GetDeviceDescription() { return "Security_Plugin"; } ;
	string Get_Path() { return m_mapParameters[2];}
	string Get_Mobile_Orbiter_Notification() { return m_mapParameters[34];}
	string Get_Other_Phone_Notifications() { return m_mapParameters[35];}
	string Get_Neighbors_to_Call() { return m_mapParameters[36];}
	int Get_PK_HouseMode() { return atoi(m_mapParameters[38].c_str());}
	void Set_PK_HouseMode(int Value) { SetParm(38,StringUtils::itos(Value).c_str()); }
	string Get_PK_Device() { return m_mapParameters[77];}
	string Get_Emergency_Calls() { return m_mapParameters[96];}
};



//   OUR COMMAND CLASS 

class Security_Plugin_Command : public Command_Impl
{
public:
	Security_Plugin_Command(int DeviceID, string ServerAddress,bool bConnectEventHandler=true,bool bLocalMode=false,class Router *pRouter=NULL)
	: Command_Impl(DeviceID, ServerAddress, bLocalMode, pRouter)
	{
	}
	virtual bool GetConfig()
	{
		
		m_pData=NULL;
		m_pEvent = new Security_Plugin_Event(m_dwPK_Device, m_sHostName, !m_bLocalMode);
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
					m_pEvent = new Security_Plugin_Event(m_dwPK_Device, m_sHostName, !m_bLocalMode);
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
			m_pData = new Security_Plugin_Data();
			return true;
		}
		if( (m_pEvent->m_pClientSocket->m_eLastError!=cs_err_None && m_pEvent->m_pClientSocket->m_eLastError!=cs_err_NeedReload) || m_pEvent->m_pClientSocket->m_Socket==INVALID_SOCKET )
			return false;

		int Size; char *pConfig = m_pEvent->GetConfig(Size);
		if( !pConfig )
			return false;
		m_pData = new Security_Plugin_Data();
		if( Size )
			m_pData->SerializeRead(Size,pConfig);
		delete[] pConfig;
		pConfig = m_pEvent->GetDeviceList(Size);
		m_pData->m_AllDevices.SerializeRead(Size,pConfig);
		delete[] pConfig;
		m_pData->m_pEvent_Impl = m_pEvent;
		m_pcRequestSocket = new Event_Impl(m_dwPK_Device, 33,m_sHostName);
		if( m_iInstanceID )
		{
			m_pEvent->m_pClientSocket->SendString("INSTANCE " + StringUtils::itos(m_iInstanceID));
			m_pcRequestSocket->m_pClientSocket->SendString("INSTANCE " + StringUtils::itos(m_iInstanceID));
		}
		return true;
	};
	Security_Plugin_Command(Command_Impl *pPrimaryDeviceCommand, DeviceData_Impl *pData, Event_Impl *pEvent, Router *pRouter) : Command_Impl(pPrimaryDeviceCommand, pData, pEvent, pRouter) {};
	virtual ~Security_Plugin_Command() {};
	Security_Plugin_Event *GetEvents() { return (Security_Plugin_Event *) m_pEvent; };
	Security_Plugin_Data *GetData() { return (Security_Plugin_Data *) m_pData; };
	const char *GetClassName() { return "Security_Plugin_Command"; };
	virtual int PK_DeviceTemplate_get() { return 33; };
	static int PK_DeviceTemplate_get_static() { return 33; };
	virtual void ReceivedCommandForChild(DeviceData_Impl *pDeviceData_Impl,string &sCMD_Result,Message *pMessage) { };
	virtual void ReceivedUnknownCommand(string &sCMD_Result,Message *pMessage) { };
	Command_Impl *CreateCommand(int PK_DeviceTemplate, Command_Impl *pPrimaryDeviceCommand, DeviceData_Impl *pData, Event_Impl *pEvent);
	//Data accessors
	string DATA_Get_Path() { return GetData()->Get_Path(); }
	string DATA_Get_Mobile_Orbiter_Notification() { return GetData()->Get_Mobile_Orbiter_Notification(); }
	string DATA_Get_Other_Phone_Notifications() { return GetData()->Get_Other_Phone_Notifications(); }
	string DATA_Get_Neighbors_to_Call() { return GetData()->Get_Neighbors_to_Call(); }
	int DATA_Get_PK_HouseMode() { return GetData()->Get_PK_HouseMode(); }
	void DATA_Set_PK_HouseMode(int Value,bool bUpdateDatabase=false) { GetData()->Set_PK_HouseMode(Value); if( bUpdateDatabase ) SetDeviceDataInDB(m_dwPK_Device,38,Value); }
	string DATA_Get_PK_Device() { return GetData()->Get_PK_Device(); }
	string DATA_Get_Emergency_Calls() { return GetData()->Get_Emergency_Calls(); }
	//Event accessors
	void EVENT_Security_Breach(int iPK_Device) { GetEvents()->Security_Breach(iPK_Device); }
	void EVENT_Fire_Alarm(int iPK_Device) { GetEvents()->Fire_Alarm(iPK_Device); }
	void EVENT_Reset_Alarm() { GetEvents()->Reset_Alarm(); }
	void EVENT_Air_Quality(int iPK_Device) { GetEvents()->Air_Quality(iPK_Device); }
	void EVENT_Doorbell(int iPK_Device) { GetEvents()->Doorbell(iPK_Device); }
	void EVENT_Monitor_Mode(int iPK_Device) { GetEvents()->Monitor_Mode(iPK_Device); }
	void EVENT_House_Mode_Changed(int iPK_DeviceGroup,int iPK_HouseMode) { GetEvents()->House_Mode_Changed(iPK_DeviceGroup,iPK_HouseMode); }
	//Commands - Override these to handle commands from the server
	virtual void CMD_Set_House_Mode(string sValue_To_Assign,int iPK_Users,string sPassword,int iPK_DeviceGroup,string sHandling_Instructions,string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Verify_PIN(int iPK_Users,string sPassword,bool *bIsSuccessful,string &sCMD_Result,class Message *pMessage) {};

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
				case 19:
					{
						string sCMD_Result="OK";
					string sValue_To_Assign=pMessage->m_mapParameters[5];
					int iPK_Users=atoi(pMessage->m_mapParameters[17].c_str());
					string sPassword=pMessage->m_mapParameters[99];
					int iPK_DeviceGroup=atoi(pMessage->m_mapParameters[100].c_str());
					string sHandling_Instructions=pMessage->m_mapParameters[101];
						CMD_Set_House_Mode(sValue_To_Assign.c_str(),iPK_Users,sPassword.c_str(),iPK_DeviceGroup,sHandling_Instructions.c_str(),sCMD_Result,pMessage);
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
								CMD_Set_House_Mode(sValue_To_Assign.c_str(),iPK_Users,sPassword.c_str(),iPK_DeviceGroup,sHandling_Instructions.c_str(),sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 387:
					{
						string sCMD_Result="OK";
					int iPK_Users=atoi(pMessage->m_mapParameters[17].c_str());
					string sPassword=pMessage->m_mapParameters[99];
					bool bIsSuccessful=(pMessage->m_mapParameters[40]=="1" ? true : false);
						CMD_Verify_PIN(iPK_Users,sPassword.c_str(),&bIsSuccessful,sCMD_Result,pMessage);
						if( pMessage->m_eExpectedResponse==ER_ReplyMessage && !pMessage->m_bRespondedToMessage )
						{
							pMessage->m_bRespondedToMessage=true;
							Message *pMessageOut=new Message(m_dwPK_Device,pMessage->m_dwPK_Device_From,PRIORITY_NORMAL,MESSAGETYPE_REPLY,0,0);
						pMessageOut->m_mapParameters[40]=(bIsSuccessful ? "1" : "0");
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
								CMD_Verify_PIN(iPK_Users,sPassword.c_str(),&bIsSuccessful,sCMD_Result,pMessage);
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
