#ifndef Orbiter_PluginBase_h
#define Orbiter_PluginBase_h
#include "DeviceData_Impl.h"
#include "Message.h"
#include "Command_Impl.h"
#include "Logger.h"


namespace DCE
{
//   OUR EVENT CLASS 

class Orbiter_Plugin_Event : public Event_Impl
{
public:
	Orbiter_Plugin_Event(int DeviceID, string ServerAddress, bool bConnectEventHandler=true) : Event_Impl(DeviceID,12, ServerAddress, bConnectEventHandler) {};
	Orbiter_Plugin_Event(class ClientSocket *pOCClientSocket, int DeviceID) : Event_Impl(pOCClientSocket, DeviceID) {};
	//Events
	class Event_Impl *CreateEvent( unsigned long dwPK_DeviceTemplate, ClientSocket *pOCClientSocket, unsigned long dwDevice );
	virtual void Follow_Me_Lighting(int iPK_Orbiter,int iPK_Room,int iPK_Users,int iPK_Room_Left)
	{
		SendMessage(new Message(m_dwPK_Device, DEVICEID_EVENTMANAGER, PRIORITY_NORMAL, MESSAGETYPE_EVENT, 30,4,14,StringUtils::itos(iPK_Orbiter).c_str(),27,StringUtils::itos(iPK_Room).c_str(),31,StringUtils::itos(iPK_Users).c_str(),32,StringUtils::itos(iPK_Room_Left).c_str()));
	}

	virtual void Follow_Me_Climate(int iPK_Orbiter,int iPK_Room,int iPK_Users,int iPK_Room_Left)
	{
		SendMessage(new Message(m_dwPK_Device, DEVICEID_EVENTMANAGER, PRIORITY_NORMAL, MESSAGETYPE_EVENT, 31,4,14,StringUtils::itos(iPK_Orbiter).c_str(),27,StringUtils::itos(iPK_Room).c_str(),31,StringUtils::itos(iPK_Users).c_str(),32,StringUtils::itos(iPK_Room_Left).c_str()));
	}

	virtual void Follow_Me_Media(int iPK_Orbiter,int iPK_Users,int iPK_EntArea,int iPK_EntArea_Left)
	{
		SendMessage(new Message(m_dwPK_Device, DEVICEID_EVENTMANAGER, PRIORITY_NORMAL, MESSAGETYPE_EVENT, 32,4,14,StringUtils::itos(iPK_Orbiter).c_str(),31,StringUtils::itos(iPK_Users).c_str(),33,StringUtils::itos(iPK_EntArea).c_str(),34,StringUtils::itos(iPK_EntArea_Left).c_str()));
	}

	virtual void Follow_Me_Telecom(int iPK_Orbiter,int iPK_Room,int iPK_Users,int iPK_Room_Left)
	{
		SendMessage(new Message(m_dwPK_Device, DEVICEID_EVENTMANAGER, PRIORITY_NORMAL, MESSAGETYPE_EVENT, 33,4,14,StringUtils::itos(iPK_Orbiter).c_str(),27,StringUtils::itos(iPK_Room).c_str(),31,StringUtils::itos(iPK_Users).c_str(),32,StringUtils::itos(iPK_Room_Left).c_str()));
	}

	virtual void Follow_Me_Security(int iPK_Orbiter,int iPK_Room,int iPK_Users,int iPK_Room_Left)
	{
		SendMessage(new Message(m_dwPK_Device, DEVICEID_EVENTMANAGER, PRIORITY_NORMAL, MESSAGETYPE_EVENT, 34,4,14,StringUtils::itos(iPK_Orbiter).c_str(),27,StringUtils::itos(iPK_Room).c_str(),31,StringUtils::itos(iPK_Users).c_str(),32,StringUtils::itos(iPK_Room_Left).c_str()));
	}

};


//   OUR DATA CLASS 

class Orbiter_Plugin_Data : public DeviceData_Impl
{
public:
	virtual ~Orbiter_Plugin_Data() {};
	class DeviceData_Impl *CreateData(DeviceData_Impl *Parent,char *pDataBlock,unsigned long AllocatedSize,char *CurrentPosition);
	virtual int GetPK_DeviceList() { return 12; } ;
	virtual const char *GetDeviceDescription() { return "Orbiter_Plugin"; } ;
	int Get_ThreshHold() { return atoi(m_mapParameters[61].c_str());}
	string Get_Ignore_State() { return m_mapParameters[87];}
};



//   OUR COMMAND CLASS 

class Orbiter_Plugin_Command : public Command_Impl
{
public:
	Orbiter_Plugin_Command(int DeviceID, string ServerAddress,bool bConnectEventHandler=true,bool bLocalMode=false,class Router *pRouter=NULL)
	: Command_Impl(DeviceID, ServerAddress, bLocalMode, pRouter)
	{
	}
	virtual bool GetConfig()
	{
		if( m_bLocalMode )
			return true;
		m_pData=NULL;
		m_pEvent = new Orbiter_Plugin_Event(m_dwPK_Device, m_sHostName);
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
					m_pEvent = new Orbiter_Plugin_Event(m_dwPK_Device, m_sHostName);
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
		
		if( m_pEvent->m_pClientSocket->m_eLastError!=cs_err_None || m_pEvent->m_pClientSocket->m_Socket==INVALID_SOCKET )
			return false;

		int Size; char *pConfig = m_pEvent->GetConfig(Size);
		if( !pConfig )
			return false;
		m_pData = new Orbiter_Plugin_Data();
		if( Size )
			m_pData->SerializeRead(Size,pConfig);
		delete[] pConfig;
		pConfig = m_pEvent->GetDeviceList(Size);
		m_pData->m_AllDevices.SerializeRead(Size,pConfig);
		delete[] pConfig;
		m_pData->m_pEvent_Impl = m_pEvent;
		m_pcRequestSocket = new Event_Impl(m_dwPK_Device, 12,m_sHostName);
		if( m_iInstanceID )
		{
			m_pEvent->m_pClientSocket->SendString("INSTANCE " + StringUtils::itos(m_iInstanceID));
			m_pcRequestSocket->m_pClientSocket->SendString("INSTANCE " + StringUtils::itos(m_iInstanceID));
		}
		return true;
	};
	Orbiter_Plugin_Command(Command_Impl *pPrimaryDeviceCommand, DeviceData_Impl *pData, Event_Impl *pEvent, Router *pRouter) : Command_Impl(pPrimaryDeviceCommand, pData, pEvent, pRouter) {};
	virtual ~Orbiter_Plugin_Command() {};
	Orbiter_Plugin_Event *GetEvents() { return (Orbiter_Plugin_Event *) m_pEvent; };
	Orbiter_Plugin_Data *GetData() { return (Orbiter_Plugin_Data *) m_pData; };
	const char *GetClassName() { return "Orbiter_Plugin_Command"; };
	virtual int PK_DeviceTemplate_get() { return 12; };
	static int PK_DeviceTemplate_get_static() { return 12; };
	virtual void ReceivedCommandForChild(DeviceData_Base *pDeviceData_Base,string &sCMD_Result,Message *pMessage) { };
	virtual void ReceivedUnknownCommand(string &sCMD_Result,Message *pMessage) { };
	Command_Impl *CreateCommand(int PK_DeviceTemplate, Command_Impl *pPrimaryDeviceCommand, DeviceData_Impl *pData, Event_Impl *pEvent);
	//Data accessors
	int DATA_Get_ThreshHold() { return GetData()->Get_ThreshHold(); }
	string DATA_Get_Ignore_State() { return GetData()->Get_Ignore_State(); }
	//Event accessors
	void EVENT_Follow_Me_Lighting(int iPK_Orbiter,int iPK_Room,int iPK_Users,int iPK_Room_Left) { GetEvents()->Follow_Me_Lighting(iPK_Orbiter,iPK_Room,iPK_Users,iPK_Room_Left); }
	void EVENT_Follow_Me_Climate(int iPK_Orbiter,int iPK_Room,int iPK_Users,int iPK_Room_Left) { GetEvents()->Follow_Me_Climate(iPK_Orbiter,iPK_Room,iPK_Users,iPK_Room_Left); }
	void EVENT_Follow_Me_Media(int iPK_Orbiter,int iPK_Users,int iPK_EntArea,int iPK_EntArea_Left) { GetEvents()->Follow_Me_Media(iPK_Orbiter,iPK_Users,iPK_EntArea,iPK_EntArea_Left); }
	void EVENT_Follow_Me_Telecom(int iPK_Orbiter,int iPK_Room,int iPK_Users,int iPK_Room_Left) { GetEvents()->Follow_Me_Telecom(iPK_Orbiter,iPK_Room,iPK_Users,iPK_Room_Left); }
	void EVENT_Follow_Me_Security(int iPK_Orbiter,int iPK_Room,int iPK_Users,int iPK_Room_Left) { GetEvents()->Follow_Me_Security(iPK_Orbiter,iPK_Room,iPK_Users,iPK_Room_Left); }
	//Commands - Override these to handle commands from the server
	virtual void CMD_Set_Current_User(int iPK_Users,string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Set_Entertainment_Area(string sPK_EntertainArea,string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Set_Current_Room(int iPK_Room,string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_New_Orbiter(string sType,int iPK_Users,int iPK_DeviceTemplate,string sMac_address,int iPK_Room,int iWidth,int iHeight,int iPK_Skin,int iPK_Language,int iPK_Size,int iUses_Wifi_connection,int *iPK_Device,string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Add_Unknown_Device(string sText,string sID,string sMac_address,string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Get_Floorplan_Layout(string *sValue_To_Assign,string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Get_Current_Floorplan(string sID,int iPK_FloorplanType,string *sValue_To_Assign,string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Orbiter_Registered(string sOnOff,int iPK_Users,string sPK_EntertainArea,int iPK_Room,char **pData,int *iData_Size,string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Set_FollowMe(int iPK_Device,string sText,int iPK_Users,string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Regen_Orbiter(int iPK_Device,string sForce,string sReset,string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Regen_Orbiter_Finished(int iPK_Device,string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Set_Room_For_Device(int iPK_Device,string sName,int iPK_Room,string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Set_Auto_Switch_to_Remote(int iPK_Device,bool bTrueFalse,string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Display_Message(string sText,string sType,string sName,string sTime,string sPK_Device_List,string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Display_Dialog_Box_On_Orbiter(string sText,string sOptions,string sPK_Device_List,string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Send_File_To_Phone(string sMac_address,string sCommand_Line,int iApp_Server_Device_ID,string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Get_Orbiter_Status(int iPK_Device,string *sValue_To_Assign,string *sText,int *iValue,string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Get_Orbiter_Options(string sText,string *sValue_To_Assign,string &sCMD_Result,class Message *pMessage) {};

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
				case 58:
					{
						string sCMD_Result="OK";
					int iPK_Users=atoi(pMessage->m_mapParameters[17].c_str());
						CMD_Set_Current_User(iPK_Users,sCMD_Result,pMessage);
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
								CMD_Set_Current_User(iPK_Users,sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 59:
					{
						string sCMD_Result="OK";
					string sPK_EntertainArea=pMessage->m_mapParameters[45];
						CMD_Set_Entertainment_Area(sPK_EntertainArea.c_str(),sCMD_Result,pMessage);
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
								CMD_Set_Entertainment_Area(sPK_EntertainArea.c_str(),sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 77:
					{
						string sCMD_Result="OK";
					int iPK_Room=atoi(pMessage->m_mapParameters[57].c_str());
						CMD_Set_Current_Room(iPK_Room,sCMD_Result,pMessage);
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
								CMD_Set_Current_Room(iPK_Room,sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 78:
					{
						string sCMD_Result="OK";
					string sType=pMessage->m_mapParameters[14];
					int iPK_Users=atoi(pMessage->m_mapParameters[17].c_str());
					int iPK_DeviceTemplate=atoi(pMessage->m_mapParameters[44].c_str());
					string sMac_address=pMessage->m_mapParameters[47];
					int iPK_Room=atoi(pMessage->m_mapParameters[57].c_str());
					int iWidth=atoi(pMessage->m_mapParameters[60].c_str());
					int iHeight=atoi(pMessage->m_mapParameters[61].c_str());
					int iPK_Skin=atoi(pMessage->m_mapParameters[141].c_str());
					int iPK_Language=atoi(pMessage->m_mapParameters[142].c_str());
					int iPK_Size=atoi(pMessage->m_mapParameters[143].c_str());
					int iUses_Wifi_connection=atoi(pMessage->m_mapParameters[147].c_str());
					int iPK_Device=atoi(pMessage->m_mapParameters[2].c_str());
						CMD_New_Orbiter(sType.c_str(),iPK_Users,iPK_DeviceTemplate,sMac_address.c_str(),iPK_Room,iWidth,iHeight,iPK_Skin,iPK_Language,iPK_Size,iUses_Wifi_connection,&iPK_Device,sCMD_Result,pMessage);
						if( pMessage->m_eExpectedResponse==ER_ReplyMessage && !pMessage->m_bRespondedToMessage )
						{
							pMessage->m_bRespondedToMessage=true;
							Message *pMessageOut=new Message(m_dwPK_Device,pMessage->m_dwPK_Device_From,PRIORITY_NORMAL,MESSAGETYPE_REPLY,0,0);
						pMessageOut->m_mapParameters[2]=StringUtils::itos(iPK_Device);
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
								CMD_New_Orbiter(sType.c_str(),iPK_Users,iPK_DeviceTemplate,sMac_address.c_str(),iPK_Room,iWidth,iHeight,iPK_Skin,iPK_Language,iPK_Size,iUses_Wifi_connection,&iPK_Device,sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 79:
					{
						string sCMD_Result="OK";
					string sText=pMessage->m_mapParameters[9];
					string sID=pMessage->m_mapParameters[10];
					string sMac_address=pMessage->m_mapParameters[47];
						CMD_Add_Unknown_Device(sText.c_str(),sID.c_str(),sMac_address.c_str(),sCMD_Result,pMessage);
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
								CMD_Add_Unknown_Device(sText.c_str(),sID.c_str(),sMac_address.c_str(),sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 183:
					{
						string sCMD_Result="OK";
					string sValue_To_Assign=pMessage->m_mapParameters[5];
						CMD_Get_Floorplan_Layout(&sValue_To_Assign,sCMD_Result,pMessage);
						if( pMessage->m_eExpectedResponse==ER_ReplyMessage && !pMessage->m_bRespondedToMessage )
						{
							pMessage->m_bRespondedToMessage=true;
							Message *pMessageOut=new Message(m_dwPK_Device,pMessage->m_dwPK_Device_From,PRIORITY_NORMAL,MESSAGETYPE_REPLY,0,0);
						pMessageOut->m_mapParameters[5]=sValue_To_Assign;
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
								CMD_Get_Floorplan_Layout(&sValue_To_Assign,sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 186:
					{
						string sCMD_Result="OK";
					string sID=pMessage->m_mapParameters[10];
					int iPK_FloorplanType=atoi(pMessage->m_mapParameters[46].c_str());
					string sValue_To_Assign=pMessage->m_mapParameters[5];
						CMD_Get_Current_Floorplan(sID.c_str(),iPK_FloorplanType,&sValue_To_Assign,sCMD_Result,pMessage);
						if( pMessage->m_eExpectedResponse==ER_ReplyMessage && !pMessage->m_bRespondedToMessage )
						{
							pMessage->m_bRespondedToMessage=true;
							Message *pMessageOut=new Message(m_dwPK_Device,pMessage->m_dwPK_Device_From,PRIORITY_NORMAL,MESSAGETYPE_REPLY,0,0);
						pMessageOut->m_mapParameters[5]=sValue_To_Assign;
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
								CMD_Get_Current_Floorplan(sID.c_str(),iPK_FloorplanType,&sValue_To_Assign,sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 255:
					{
						string sCMD_Result="OK";
					string sOnOff=pMessage->m_mapParameters[8];
					int iPK_Users=atoi(pMessage->m_mapParameters[17].c_str());
					string sPK_EntertainArea=pMessage->m_mapParameters[45];
					int iPK_Room=atoi(pMessage->m_mapParameters[57].c_str());
					char *pData=pMessage->m_mapData_Parameters[19];
					int iData_Size=pMessage->m_mapData_Lengths[19];
						CMD_Orbiter_Registered(sOnOff.c_str(),iPK_Users,sPK_EntertainArea.c_str(),iPK_Room,&pData,&iData_Size,sCMD_Result,pMessage);
						if( pMessage->m_eExpectedResponse==ER_ReplyMessage && !pMessage->m_bRespondedToMessage )
						{
							pMessage->m_bRespondedToMessage=true;
							Message *pMessageOut=new Message(m_dwPK_Device,pMessage->m_dwPK_Device_From,PRIORITY_NORMAL,MESSAGETYPE_REPLY,0,0);
						pMessageOut->m_mapData_Parameters[19]=pData; pMessageOut->m_mapData_Lengths[19]=iData_Size;
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
								CMD_Orbiter_Registered(sOnOff.c_str(),iPK_Users,sPK_EntertainArea.c_str(),iPK_Room,&pData,&iData_Size,sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 261:
					{
						string sCMD_Result="OK";
					int iPK_Device=atoi(pMessage->m_mapParameters[2].c_str());
					string sText=pMessage->m_mapParameters[9];
					int iPK_Users=atoi(pMessage->m_mapParameters[17].c_str());
						CMD_Set_FollowMe(iPK_Device,sText.c_str(),iPK_Users,sCMD_Result,pMessage);
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
								CMD_Set_FollowMe(iPK_Device,sText.c_str(),iPK_Users,sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 266:
					{
						string sCMD_Result="OK";
					int iPK_Device=atoi(pMessage->m_mapParameters[2].c_str());
					string sForce=pMessage->m_mapParameters[21];
					string sReset=pMessage->m_mapParameters[24];
						CMD_Regen_Orbiter(iPK_Device,sForce.c_str(),sReset.c_str(),sCMD_Result,pMessage);
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
								CMD_Regen_Orbiter(iPK_Device,sForce.c_str(),sReset.c_str(),sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 267:
					{
						string sCMD_Result="OK";
					int iPK_Device=atoi(pMessage->m_mapParameters[2].c_str());
						CMD_Regen_Orbiter_Finished(iPK_Device,sCMD_Result,pMessage);
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
								CMD_Regen_Orbiter_Finished(iPK_Device,sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 274:
					{
						string sCMD_Result="OK";
					int iPK_Device=atoi(pMessage->m_mapParameters[2].c_str());
					string sName=pMessage->m_mapParameters[50];
					int iPK_Room=atoi(pMessage->m_mapParameters[57].c_str());
						CMD_Set_Room_For_Device(iPK_Device,sName.c_str(),iPK_Room,sCMD_Result,pMessage);
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
								CMD_Set_Room_For_Device(iPK_Device,sName.c_str(),iPK_Room,sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 404:
					{
						string sCMD_Result="OK";
					int iPK_Device=atoi(pMessage->m_mapParameters[2].c_str());
					bool bTrueFalse=(pMessage->m_mapParameters[119]=="1" ? true : false);
						CMD_Set_Auto_Switch_to_Remote(iPK_Device,bTrueFalse,sCMD_Result,pMessage);
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
								CMD_Set_Auto_Switch_to_Remote(iPK_Device,bTrueFalse,sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 406:
					{
						string sCMD_Result="OK";
					string sText=pMessage->m_mapParameters[9];
					string sType=pMessage->m_mapParameters[14];
					string sName=pMessage->m_mapParameters[50];
					string sTime=pMessage->m_mapParameters[102];
					string sPK_Device_List=pMessage->m_mapParameters[103];
						CMD_Display_Message(sText.c_str(),sType.c_str(),sName.c_str(),sTime.c_str(),sPK_Device_List.c_str(),sCMD_Result,pMessage);
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
								CMD_Display_Message(sText.c_str(),sType.c_str(),sName.c_str(),sTime.c_str(),sPK_Device_List.c_str(),sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 686:
					{
						string sCMD_Result="OK";
					string sText=pMessage->m_mapParameters[9];
					string sOptions=pMessage->m_mapParameters[39];
					string sPK_Device_List=pMessage->m_mapParameters[103];
						CMD_Display_Dialog_Box_On_Orbiter(sText.c_str(),sOptions.c_str(),sPK_Device_List.c_str(),sCMD_Result,pMessage);
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
								CMD_Display_Dialog_Box_On_Orbiter(sText.c_str(),sOptions.c_str(),sPK_Device_List.c_str(),sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 693:
					{
						string sCMD_Result="OK";
					string sMac_address=pMessage->m_mapParameters[47];
					string sCommand_Line=pMessage->m_mapParameters[137];
					int iApp_Server_Device_ID=atoi(pMessage->m_mapParameters[140].c_str());
						CMD_Send_File_To_Phone(sMac_address.c_str(),sCommand_Line.c_str(),iApp_Server_Device_ID,sCMD_Result,pMessage);
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
								CMD_Send_File_To_Phone(sMac_address.c_str(),sCommand_Line.c_str(),iApp_Server_Device_ID,sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 694:
					{
						string sCMD_Result="OK";
					int iPK_Device=atoi(pMessage->m_mapParameters[2].c_str());
					string sValue_To_Assign=pMessage->m_mapParameters[5];
					string sText=pMessage->m_mapParameters[9];
					int iValue=atoi(pMessage->m_mapParameters[48].c_str());
						CMD_Get_Orbiter_Status(iPK_Device,&sValue_To_Assign,&sText,&iValue,sCMD_Result,pMessage);
						if( pMessage->m_eExpectedResponse==ER_ReplyMessage && !pMessage->m_bRespondedToMessage )
						{
							pMessage->m_bRespondedToMessage=true;
							Message *pMessageOut=new Message(m_dwPK_Device,pMessage->m_dwPK_Device_From,PRIORITY_NORMAL,MESSAGETYPE_REPLY,0,0);
						pMessageOut->m_mapParameters[5]=sValue_To_Assign;
						pMessageOut->m_mapParameters[9]=sText;
						pMessageOut->m_mapParameters[48]=StringUtils::itos(iValue);
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
								CMD_Get_Orbiter_Status(iPK_Device,&sValue_To_Assign,&sText,&iValue,sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 695:
					{
						string sCMD_Result="OK";
					string sText=pMessage->m_mapParameters[9];
					string sValue_To_Assign=pMessage->m_mapParameters[5];
						CMD_Get_Orbiter_Options(sText.c_str(),&sValue_To_Assign,sCMD_Result,pMessage);
						if( pMessage->m_eExpectedResponse==ER_ReplyMessage && !pMessage->m_bRespondedToMessage )
						{
							pMessage->m_bRespondedToMessage=true;
							Message *pMessageOut=new Message(m_dwPK_Device,pMessage->m_dwPK_Device_From,PRIORITY_NORMAL,MESSAGETYPE_REPLY,0,0);
						pMessageOut->m_mapParameters[5]=sValue_To_Assign;
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
								CMD_Get_Orbiter_Options(sText.c_str(),&sValue_To_Assign,sCMD_Result,pMessage);
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
				DeviceData_Base *pDeviceData_Base = m_pData->m_AllDevices.m_mapDeviceData_Base_Find(pMessage->m_dwPK_Device_To);
				string sCMD_Result="UNHANDLED";
				if( pDeviceData_Base && pDeviceData_Base->IsChildOf(m_pData) )
					ReceivedCommandForChild(pDeviceData_Base,sCMD_Result,pMessage);
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
