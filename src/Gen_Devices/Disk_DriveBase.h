#ifndef Disk_DriveBase_h
#define Disk_DriveBase_h
#include "DeviceData_Impl.h"
#include "Message.h"
#include "Command_Impl.h"
#include "Logger.h"


namespace DCE
{
//   OUR EVENT CLASS 

class Disk_Drive_Event : public Event_Impl
{
public:
	Disk_Drive_Event(int DeviceID, string ServerAddress, bool bConnectEventHandler=true) : Event_Impl(DeviceID,11, ServerAddress, bConnectEventHandler) {};
	Disk_Drive_Event(class ClientSocket *pOCClientSocket, int DeviceID) : Event_Impl(pOCClientSocket, DeviceID) {};
	//Events
	class Event_Impl *CreateEvent( unsigned long dwPK_DeviceTemplate, ClientSocket *pOCClientSocket, unsigned long dwDevice );
	virtual void Media_Inserted(int iFK_MediaType,string sMRL,string sID,string sName)
	{
		SendMessage(new Message(m_dwPK_Device, DEVICEID_EVENTMANAGER, PRIORITY_NORMAL, MESSAGETYPE_EVENT, 3,4,3,StringUtils::itos(iFK_MediaType).c_str(),4,sMRL.c_str(),7,sID.c_str(),35,sName.c_str()));
	}

	virtual void Ripping_Progress(string sText,int iResult,string sValue,string sName,int iEK_Disc)
	{
		SendMessage(new Message(m_dwPK_Device, DEVICEID_EVENTMANAGER, PRIORITY_NORMAL, MESSAGETYPE_EVENT, 35,5,13,sText.c_str(),20,StringUtils::itos(iResult).c_str(),30,sValue.c_str(),35,sName.c_str(),43,StringUtils::itos(iEK_Disc).c_str()));
	}

};


//   OUR DATA CLASS 

class Disk_Drive_Data : public DeviceData_Impl
{
public:
	virtual ~Disk_Drive_Data() {};
	class DeviceData_Impl *CreateData(DeviceData_Impl *Parent,char *pDataBlock,unsigned long AllocatedSize,char *CurrentPosition);
	virtual int GetPK_DeviceList() { return 11; } ;
	virtual const char *GetDeviceDescription() { return "Disk_Drive"; } ;
	string Get_Drive() { return m_mapParameters[6];}
	void Set_Drive(string Value) { SetParm(6,Value.c_str()); }
};



//   OUR COMMAND CLASS 

class Disk_Drive_Command : public Command_Impl
{
public:
	Disk_Drive_Command(int DeviceID, string ServerAddress,bool bConnectEventHandler=true,bool bLocalMode=false,class Router *pRouter=NULL)
	: Command_Impl(DeviceID, ServerAddress, bLocalMode, pRouter)
	{
	}
	virtual bool GetConfig()
	{
		
		m_pData=NULL;
		m_pEvent = new Disk_Drive_Event(m_dwPK_Device, m_sHostName, !m_bLocalMode);
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
					m_pEvent = new Disk_Drive_Event(m_dwPK_Device, m_sHostName, !m_bLocalMode);
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
			m_pData = new Disk_Drive_Data();
			return true;
		}
		if( m_pEvent->m_pClientSocket->m_eLastError!=cs_err_None || m_pEvent->m_pClientSocket->m_Socket==INVALID_SOCKET )
			return false;

		int Size; char *pConfig = m_pEvent->GetConfig(Size);
		if( !pConfig )
			return false;
		m_pData = new Disk_Drive_Data();
		if( Size )
			m_pData->SerializeRead(Size,pConfig);
		delete[] pConfig;
		pConfig = m_pEvent->GetDeviceList(Size);
		m_pData->m_AllDevices.SerializeRead(Size,pConfig);
		delete[] pConfig;
		m_pData->m_pEvent_Impl = m_pEvent;
		m_pcRequestSocket = new Event_Impl(m_dwPK_Device, 11,m_sHostName);
		if( m_iInstanceID )
		{
			m_pEvent->m_pClientSocket->SendString("INSTANCE " + StringUtils::itos(m_iInstanceID));
			m_pcRequestSocket->m_pClientSocket->SendString("INSTANCE " + StringUtils::itos(m_iInstanceID));
		}
		return true;
	};
	Disk_Drive_Command(Command_Impl *pPrimaryDeviceCommand, DeviceData_Impl *pData, Event_Impl *pEvent, Router *pRouter) : Command_Impl(pPrimaryDeviceCommand, pData, pEvent, pRouter) {};
	virtual ~Disk_Drive_Command() {};
	Disk_Drive_Event *GetEvents() { return (Disk_Drive_Event *) m_pEvent; };
	Disk_Drive_Data *GetData() { return (Disk_Drive_Data *) m_pData; };
	const char *GetClassName() { return "Disk_Drive_Command"; };
	virtual int PK_DeviceTemplate_get() { return 11; };
	static int PK_DeviceTemplate_get_static() { return 11; };
	virtual void ReceivedCommandForChild(DeviceData_Impl *pDeviceData_Impl,string &sCMD_Result,Message *pMessage) { };
	virtual void ReceivedUnknownCommand(string &sCMD_Result,Message *pMessage) { };
	Command_Impl *CreateCommand(int PK_DeviceTemplate, Command_Impl *pPrimaryDeviceCommand, DeviceData_Impl *pData, Event_Impl *pEvent);
	//Data accessors
	string DATA_Get_Drive() { return GetData()->Get_Drive(); }
	void DATA_Set_Drive(string Value,bool bUpdateDatabase=false) { GetData()->Set_Drive(Value); if( bUpdateDatabase ) SetDeviceDataInDB(m_dwPK_Device,6,Value); }
	//Event accessors
	void EVENT_Media_Inserted(int iFK_MediaType,string sMRL,string sID,string sName) { GetEvents()->Media_Inserted(iFK_MediaType,sMRL.c_str(),sID.c_str(),sName.c_str()); }
	void EVENT_Ripping_Progress(string sText,int iResult,string sValue,string sName,int iEK_Disc) { GetEvents()->Ripping_Progress(sText.c_str(),iResult,sValue.c_str(),sName.c_str(),iEK_Disc); }
	//Commands - Override these to handle commands from the server
	virtual void CMD_Disk_Drive_Monitoring_ON(string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Disk_Drive_Monitoring_OFF(string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Reset_Disk_Drive(string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Eject_Disk(string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Start_Burn_Session(string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Start_Ripping_Session(string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Add_File_To_Burning_Session(string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Start_Burning(string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Abort_Burning(string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Mount_Disk_Image(string sFilename,string *sMediaURL,string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Abort_Ripping(string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Format_Drive(string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Close_Tray(string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Rip_Disk(int iPK_Users,string sFormat,string sName,string sTracks,int iEK_Disc,string &sCMD_Result,class Message *pMessage) {};

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
				case 45:
					{
						string sCMD_Result="OK";
						CMD_Disk_Drive_Monitoring_ON(sCMD_Result,pMessage);
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
								CMD_Disk_Drive_Monitoring_ON(sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 46:
					{
						string sCMD_Result="OK";
						CMD_Disk_Drive_Monitoring_OFF(sCMD_Result,pMessage);
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
								CMD_Disk_Drive_Monitoring_OFF(sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 47:
					{
						string sCMD_Result="OK";
						CMD_Reset_Disk_Drive(sCMD_Result,pMessage);
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
								CMD_Reset_Disk_Drive(sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 48:
					{
						string sCMD_Result="OK";
						CMD_Eject_Disk(sCMD_Result,pMessage);
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
								CMD_Eject_Disk(sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 49:
					{
						string sCMD_Result="OK";
						CMD_Start_Burn_Session(sCMD_Result,pMessage);
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
								CMD_Start_Burn_Session(sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 50:
					{
						string sCMD_Result="OK";
						CMD_Start_Ripping_Session(sCMD_Result,pMessage);
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
								CMD_Start_Ripping_Session(sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 51:
					{
						string sCMD_Result="OK";
						CMD_Add_File_To_Burning_Session(sCMD_Result,pMessage);
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
								CMD_Add_File_To_Burning_Session(sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 52:
					{
						string sCMD_Result="OK";
						CMD_Start_Burning(sCMD_Result,pMessage);
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
								CMD_Start_Burning(sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 53:
					{
						string sCMD_Result="OK";
						CMD_Abort_Burning(sCMD_Result,pMessage);
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
								CMD_Abort_Burning(sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 54:
					{
						string sCMD_Result="OK";
					string sFilename=pMessage->m_mapParameters[13];
					string sMediaURL=pMessage->m_mapParameters[59];
						CMD_Mount_Disk_Image(sFilename.c_str(),&sMediaURL,sCMD_Result,pMessage);
						if( pMessage->m_eExpectedResponse==ER_ReplyMessage && !pMessage->m_bRespondedToMessage )
						{
							pMessage->m_bRespondedToMessage=true;
							Message *pMessageOut=new Message(m_dwPK_Device,pMessage->m_dwPK_Device_From,PRIORITY_NORMAL,MESSAGETYPE_REPLY,0,0);
						pMessageOut->m_mapParameters[59]=sMediaURL;
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
								CMD_Mount_Disk_Image(sFilename.c_str(),&sMediaURL,sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 55:
					{
						string sCMD_Result="OK";
						CMD_Abort_Ripping(sCMD_Result,pMessage);
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
								CMD_Abort_Ripping(sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 56:
					{
						string sCMD_Result="OK";
						CMD_Format_Drive(sCMD_Result,pMessage);
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
								CMD_Format_Drive(sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 57:
					{
						string sCMD_Result="OK";
						CMD_Close_Tray(sCMD_Result,pMessage);
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
								CMD_Close_Tray(sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 337:
					{
						string sCMD_Result="OK";
					int iPK_Users=atoi(pMessage->m_mapParameters[17].c_str());
					string sFormat=pMessage->m_mapParameters[20];
					string sName=pMessage->m_mapParameters[50];
					string sTracks=pMessage->m_mapParameters[121];
					int iEK_Disc=atoi(pMessage->m_mapParameters[131].c_str());
						CMD_Rip_Disk(iPK_Users,sFormat.c_str(),sName.c_str(),sTracks.c_str(),iEK_Disc,sCMD_Result,pMessage);
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
								CMD_Rip_Disk(iPK_Users,sFormat.c_str(),sName.c_str(),sTracks.c_str(),iEK_Disc,sCMD_Result,pMessage);
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
