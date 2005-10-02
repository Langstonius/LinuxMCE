#ifndef Datagrid_PluginBase_h
#define Datagrid_PluginBase_h
#include "DeviceData_Impl.h"
#include "Message.h"
#include "Command_Impl.h"
#include "Logger.h"


namespace DCE
{
//   OUR EVENT CLASS 

class Datagrid_Plugin_Event : public Event_Impl
{
public:
	Datagrid_Plugin_Event(int DeviceID, string ServerAddress, bool bConnectEventHandler=true) : Event_Impl(DeviceID,9, ServerAddress, bConnectEventHandler) {};
	Datagrid_Plugin_Event(class ClientSocket *pOCClientSocket, int DeviceID) : Event_Impl(pOCClientSocket, DeviceID) {};
	//Events
	class Event_Impl *CreateEvent( unsigned long dwPK_DeviceTemplate, ClientSocket *pOCClientSocket, unsigned long dwDevice );
};


//   OUR DATA CLASS 

class Datagrid_Plugin_Data : public DeviceData_Impl
{
public:
	virtual ~Datagrid_Plugin_Data() {};
	class DeviceData_Impl *CreateData(DeviceData_Impl *Parent,char *pDataBlock,unsigned long AllocatedSize,char *CurrentPosition);
	virtual int GetPK_DeviceList() { return 9; } ;
	virtual const char *GetDeviceDescription() { return "Datagrid_Plugin"; } ;
};



//   OUR COMMAND CLASS 

class Datagrid_Plugin_Command : public Command_Impl
{
public:
	Datagrid_Plugin_Command(int DeviceID, string ServerAddress,bool bConnectEventHandler=true,bool bLocalMode=false,class Router *pRouter=NULL)
	: Command_Impl(DeviceID, ServerAddress, bLocalMode, pRouter)
	{
	}
	virtual bool GetConfig()
	{
		if( m_bLocalMode )
			return true;
		m_pData=NULL;
		m_pEvent = new Datagrid_Plugin_Event(m_dwPK_Device, m_sHostName);
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
					m_pEvent = new Datagrid_Plugin_Event(m_dwPK_Device, m_sHostName);
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
		m_pData = new Datagrid_Plugin_Data();
		if( Size )
			m_pData->SerializeRead(Size,pConfig);
		delete[] pConfig;
		pConfig = m_pEvent->GetDeviceList(Size);
		m_pData->m_AllDevices.SerializeRead(Size,pConfig);
		delete[] pConfig;
		m_pData->m_pEvent_Impl = m_pEvent;
		m_pcRequestSocket = new Event_Impl(m_dwPK_Device, 9,m_sHostName);
		if( m_iInstanceID )
		{
			m_pEvent->m_pClientSocket->SendString("INSTANCE " + StringUtils::itos(m_iInstanceID));
			m_pcRequestSocket->m_pClientSocket->SendString("INSTANCE " + StringUtils::itos(m_iInstanceID));
		}
		return true;
	};
	Datagrid_Plugin_Command(Command_Impl *pPrimaryDeviceCommand, DeviceData_Impl *pData, Event_Impl *pEvent, Router *pRouter) : Command_Impl(pPrimaryDeviceCommand, pData, pEvent, pRouter) {};
	virtual ~Datagrid_Plugin_Command() {};
	Datagrid_Plugin_Event *GetEvents() { return (Datagrid_Plugin_Event *) m_pEvent; };
	Datagrid_Plugin_Data *GetData() { return (Datagrid_Plugin_Data *) m_pData; };
	const char *GetClassName() { return "Datagrid_Plugin_Command"; };
	virtual int PK_DeviceTemplate_get() { return 9; };
	static int PK_DeviceTemplate_get_static() { return 9; };
	virtual void ReceivedCommandForChild(DeviceData_Base *pDeviceData_Base,string &sCMD_Result,Message *pMessage) { };
	virtual void ReceivedUnknownCommand(string &sCMD_Result,Message *pMessage) { };
	Command_Impl *CreateCommand(int PK_DeviceTemplate, Command_Impl *pPrimaryDeviceCommand, DeviceData_Impl *pData, Event_Impl *pEvent);
	//Data accessors
	//Event accessors
	//Commands - Override these to handle commands from the server
	virtual void CMD_Request_Datagrid_Contents(string sID,string sDataGrid_ID,int iRow_count,int iColumn_count,bool bKeep_Row_Header,bool bKeep_Column_Header,bool bAdd_UpDown_Arrows,string sSeek,int iOffset,char **pData,int *iData_Size,int *iRow,int *iColumn,string &sCMD_Result,class Message *pMessage) {};
	virtual void CMD_Populate_Datagrid(string sID,string sDataGrid_ID,int iPK_DataGrid,string sOptions,int *iPK_Variable,string *sValue_To_Assign,bool *bIsSuccessful,int *iWidth,int *iHeight,string &sCMD_Result,class Message *pMessage) {};

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
				case 34:
					{
						string sCMD_Result="OK";
					string sID=pMessage->m_mapParameters[10];
					string sDataGrid_ID=pMessage->m_mapParameters[15];
					int iRow_count=atoi(pMessage->m_mapParameters[34].c_str());
					int iColumn_count=atoi(pMessage->m_mapParameters[35].c_str());
					bool bKeep_Row_Header=(pMessage->m_mapParameters[36]=="1" ? true : false);
					bool bKeep_Column_Header=(pMessage->m_mapParameters[37]=="1" ? true : false);
					bool bAdd_UpDown_Arrows=(pMessage->m_mapParameters[49]=="1" ? true : false);
					string sSeek=pMessage->m_mapParameters[73];
					int iOffset=atoi(pMessage->m_mapParameters[74].c_str());
					char *pData=pMessage->m_mapData_Parameters[19];
					int iData_Size=pMessage->m_mapData_Lengths[19];
					int iRow=atoi(pMessage->m_mapParameters[32].c_str());
					int iColumn=atoi(pMessage->m_mapParameters[33].c_str());
						CMD_Request_Datagrid_Contents(sID.c_str(),sDataGrid_ID.c_str(),iRow_count,iColumn_count,bKeep_Row_Header,bKeep_Column_Header,bAdd_UpDown_Arrows,sSeek.c_str(),iOffset,&pData,&iData_Size,&iRow,&iColumn,sCMD_Result,pMessage);
						if( pMessage->m_eExpectedResponse==ER_ReplyMessage && !pMessage->m_bRespondedToMessage )
						{
							pMessage->m_bRespondedToMessage=true;
							Message *pMessageOut=new Message(m_dwPK_Device,pMessage->m_dwPK_Device_From,PRIORITY_NORMAL,MESSAGETYPE_REPLY,0,0);
						pMessageOut->m_mapData_Parameters[19]=pData; pMessageOut->m_mapData_Lengths[19]=iData_Size;
						pMessageOut->m_mapParameters[32]=StringUtils::itos(iRow);
						pMessageOut->m_mapParameters[33]=StringUtils::itos(iColumn);
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
								CMD_Request_Datagrid_Contents(sID.c_str(),sDataGrid_ID.c_str(),iRow_count,iColumn_count,bKeep_Row_Header,bKeep_Column_Header,bAdd_UpDown_Arrows,sSeek.c_str(),iOffset,&pData,&iData_Size,&iRow,&iColumn,sCMD_Result,pMessage);
						}
					};
					iHandled++;
					continue;
				case 35:
					{
						string sCMD_Result="OK";
					string sID=pMessage->m_mapParameters[10];
					string sDataGrid_ID=pMessage->m_mapParameters[15];
					int iPK_DataGrid=atoi(pMessage->m_mapParameters[38].c_str());
					string sOptions=pMessage->m_mapParameters[39];
					int iPK_Variable=atoi(pMessage->m_mapParameters[4].c_str());
					string sValue_To_Assign=pMessage->m_mapParameters[5];
					bool bIsSuccessful=(pMessage->m_mapParameters[40]=="1" ? true : false);
					int iWidth=atoi(pMessage->m_mapParameters[60].c_str());
					int iHeight=atoi(pMessage->m_mapParameters[61].c_str());
						CMD_Populate_Datagrid(sID.c_str(),sDataGrid_ID.c_str(),iPK_DataGrid,sOptions.c_str(),&iPK_Variable,&sValue_To_Assign,&bIsSuccessful,&iWidth,&iHeight,sCMD_Result,pMessage);
						if( pMessage->m_eExpectedResponse==ER_ReplyMessage && !pMessage->m_bRespondedToMessage )
						{
							pMessage->m_bRespondedToMessage=true;
							Message *pMessageOut=new Message(m_dwPK_Device,pMessage->m_dwPK_Device_From,PRIORITY_NORMAL,MESSAGETYPE_REPLY,0,0);
						pMessageOut->m_mapParameters[4]=StringUtils::itos(iPK_Variable);
						pMessageOut->m_mapParameters[5]=sValue_To_Assign;
						pMessageOut->m_mapParameters[40]=(bIsSuccessful ? "1" : "0");
						pMessageOut->m_mapParameters[60]=StringUtils::itos(iWidth);
						pMessageOut->m_mapParameters[61]=StringUtils::itos(iHeight);
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
								CMD_Populate_Datagrid(sID.c_str(),sDataGrid_ID.c_str(),iPK_DataGrid,sOptions.c_str(),&iPK_Variable,&sValue_To_Assign,&bIsSuccessful,&iWidth,&iHeight,sCMD_Result,pMessage);
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
