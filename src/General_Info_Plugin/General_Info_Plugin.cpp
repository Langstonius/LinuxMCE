/*
 General_Info_Plugin
 
 Copyright (C) 2004 Pluto, Inc., a Florida Corporation
 
 www.plutohome.com		
 
 Phone: +1 (877) 758-8648
 
 This program is distributed according to the terms of the Pluto Public License, available at: 
 http://plutohome.com/index.php?section=public_license 
 
 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 or FITNESS FOR A PARTICULAR PURPOSE. See the Pluto Public License for more details.
 
 */

//<-dceag-d-b->
#include "General_Info_Plugin.h"
#include "DCE/Logger.h"
#include "PlutoUtils/FileUtils.h"
#include "PlutoUtils/StringUtils.h"
#include "PlutoUtils/Other.h"

#include <iostream>
using namespace std;
using namespace DCE;

#include "Gen_Devices/AllCommandsRequests.h"
//<-dceag-d-e->

#include "../PlutoUtils/md5.h"

#include "DCERouter.h"
#include "DCE/DeviceData_Router.h"
#include "pluto_main/Database_pluto_main.h"
#include "pluto_main/Table_UnknownDevices.h"
#include "pluto_main/Table_Device_QuickStart.h"
#include "pluto_main/Table_QuickStartTemplate.h"
#include "pluto_main/Table_Device_MRU.h"
#include "pluto_main/Define_DataGrid.h"
#include "pluto_main/Define_Command.h"
#include "pluto_main/Define_CommandParameter.h"
#include "pluto_main/Define_Text.h"
#include "DataGrid.h"
#include "Orbiter_Plugin/Orbiter_Plugin.h"
#include "Event_Plugin/Event_Plugin.h"

//<-dceag-const-b->
// The primary constructor when the class is created as a stand-alone device
General_Info_Plugin::General_Info_Plugin(int DeviceID, string ServerAddress,bool bConnectEventHandler,bool bLocalMode,class Router *pRouter)
	: General_Info_Plugin_Command(DeviceID, ServerAddress,bConnectEventHandler,bLocalMode,pRouter)
//<-dceag-const-e->
	, m_GipMutex("GeneralInfo")
{
    pthread_mutexattr_init( &m_MutexAttr );
    pthread_mutexattr_settype( &m_MutexAttr, PTHREAD_MUTEX_RECURSIVE_NP );
    m_GipMutex.Init( &m_MutexAttr );

	m_bRerunConfigWhenDone=false;
	m_pDatabase_pluto_main=NULL;
}

//<-dceag-getconfig-b->
bool General_Info_Plugin::GetConfig()
{
	if( !General_Info_Plugin_Command::GetConfig() )
		return false;
//<-dceag-getconfig-e->

	m_pDatabase_pluto_main = new Database_pluto_main();
	if(!m_pDatabase_pluto_main->Connect(m_pRouter->sDBHost_get(),m_pRouter->sDBUser_get(),m_pRouter->sDBPassword_get(),m_pRouter->sDBName_get(),m_pRouter->iDBPort_get()) )
	{
		g_pPlutoLogger->Write(LV_CRITICAL, "Cannot connect to database!");
		m_bQuit=true;
		return false;
	}
	return true;
}

//<-dceag-const2-b->!

//<-dceag-dest-b->
General_Info_Plugin::~General_Info_Plugin()
//<-dceag-dest-e->
{
	delete m_pDatabase_pluto_main;
	
}

//<-dceag-reg-b->
// This function will only be used if this device is loaded into the DCE Router's memory space as a plug-in.  Otherwise Connect() will be called from the main()
bool General_Info_Plugin::Register()
//<-dceag-reg-e->
{
    // Get the datagrid plugin
	m_pDatagrid_Plugin=( Datagrid_Plugin * ) m_pRouter->FindPluginByTemplate(DEVICETEMPLATE_Datagrid_Plugin_CONST);
	m_pOrbiter_Plugin=( Orbiter_Plugin * ) m_pRouter->FindPluginByTemplate(DEVICETEMPLATE_Orbiter_Plugin_CONST);
	m_pEvent_Plugin=( Event_Plugin * ) m_pRouter->FindPluginByTemplate(DEVICETEMPLATE_Event_Plugin_CONST);
	if( !m_pDatagrid_Plugin || !m_pOrbiter_Plugin || !m_pEvent_Plugin )
	{
		g_pPlutoLogger->Write(LV_CRITICAL,"Cannot find sister plugins");
		return false;
	}

	m_pDatagrid_Plugin->RegisterDatagridGenerator(
        new DataGridGeneratorCallBack( this, ( DCEDataGridGeneratorFn )( &General_Info_Plugin::PendingTasks ) )
        , DATAGRID_Pending_Tasks_CONST );

	m_pDatagrid_Plugin->RegisterDatagridGenerator(
        new DataGridGeneratorCallBack( this, ( DCEDataGridGeneratorFn )( &General_Info_Plugin::QuickStartApps ) )
        , DATAGRID_Quick_Start_Apps_CONST );

	m_pDatagrid_Plugin->RegisterDatagridGenerator(
        new DataGridGeneratorCallBack( this, ( DCEDataGridGeneratorFn )( &General_Info_Plugin::MRUDocuments ) )
        , DATAGRID_MRU_Documents_CONST );

	m_pDatagrid_Plugin->RegisterDatagridGenerator(
        new DataGridGeneratorCallBack( this, ( DCEDataGridGeneratorFn )( &General_Info_Plugin::Rooms ) )
        , DATAGRID_Rooms_CONST );

	m_pDatagrid_Plugin->RegisterDatagridGenerator(
		new DataGridGeneratorCallBack(this, (DCEDataGridGeneratorFn) (&General_Info_Plugin::BookmarkList)), 
		DATAGRID_Mozilla_Bookmarks_CONST);

	return Connect(PK_DeviceTemplate_get()); 
}

/*
	When you receive commands that are destined to one of your children,
	then if that child implements DCE then there will already be a separate class
	created for the child that will get the message.  If the child does not, then you will 
	get all	commands for your children in ReceivedCommandForChild, where 
	pDeviceData_Base is the child device.  If you handle the message, you 
	should change the sCMD_Result to OK
*/
//<-dceag-cmdch-b->
void General_Info_Plugin::ReceivedCommandForChild(DeviceData_Base *pDeviceData_Base,string &sCMD_Result,Message *pMessage)
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
void General_Info_Plugin::ReceivedUnknownCommand(string &sCMD_Result,Message *pMessage)
//<-dceag-cmduk-e->
{
	sCMD_Result = "UNKNOWN DEVICE";
}

//<-dceag-sample-b->!

/*

	COMMANDS TO IMPLEMENT

*/

//<-dceag-c68-b->

	/** @brief COMMAND: #68 - Get Device Data */
	/** Return the device data for a device. */
		/** @param #2 PK_Device */
			/** The device which you want data from */
		/** @param #5 Value To Assign */
			/** The value. */
		/** @param #52 PK_DeviceData */
			/** What parameter to get. */
		/** @param #53 UseDefault */
			/** Report the default value, rather than requesting a live value. */

void General_Info_Plugin::CMD_Get_Device_Data(int iPK_Device,int iPK_DeviceData,bool bUseDefault,string *sValue_To_Assign,string &sCMD_Result,Message *pMessage)
//<-dceag-c68-e->
{
	if( bUseDefault )
	{
		DeviceData_Router *pDeviceData_Router = m_pRouter->m_mapDeviceData_Router_Find(iPK_Device);
		if( !pDeviceData_Router )
			*sValue_To_Assign="BAD DEVICE";
		else
		{
			if( pDeviceData_Router->m_mapParameters.find(iPK_DeviceData)==pDeviceData_Router->m_mapParameters.end() )
				*sValue_To_Assign="BAD PARAMETER";
			else
				*sValue_To_Assign=pDeviceData_Router->m_mapParameters[iPK_DeviceData];
		}
		return;
	}
	else
	{
		// TODO -- Send device request
	}
}

//<-dceag-c71-b->

	/** @brief COMMAND: #71 - Request File */
	/** Get the contents of a file from the server */
		/** @param #13 Filename */
			/** The file to get */
		/** @param #19 Data */
			/** The file's contents */

void General_Info_Plugin::CMD_Request_File(string sFilename,char **pData,int *iData_Size,string &sCMD_Result,Message *pMessage)
//<-dceag-c71-e->
{
	size_t Length;
	char *c = FileUtils::ReadFileIntoBuffer(sFilename, Length);
	if( c==NULL && m_pRouter )
		c = FileUtils::ReadFileIntoBuffer(m_pRouter->sBasePath_get() + sFilename, Length);

	if( c && Length )
		g_pPlutoLogger->Write(LV_FILEREQUEST, "sending file: %s size: %d", sFilename.c_str(),(int) Length);
	else
		g_pPlutoLogger->Write(LV_CRITICAL, "requested missing file: %s", sFilename.c_str());

	*iData_Size = (int) Length;
	*pData = c;
}

//<-dceag-c79-b->

	/** @brief COMMAND: #79 - Add Unknown Device */
	/** Adds an unknown device into the database.  These are devices that are not part of the Pluto system. */
		/** @param #9 Text */
			/** A description of the device */
		/** @param #10 ID */
			/** The IP Address */
		/** @param #47 Mac address */
			/** The MAC address of the device */

void General_Info_Plugin::CMD_Add_Unknown_Device(string sText,string sID,string sMac_address,string &sCMD_Result,Message *pMessage)
//<-dceag-c79-e->
{
	Row_UnknownDevices *pRow_UnknownDevices = m_pDatabase_pluto_main->UnknownDevices_get()->AddRow();
	pRow_UnknownDevices->MacAddress_set(sMac_address);
	m_pDatabase_pluto_main->UnknownDevices_get()->Commit();
g_pPlutoLogger->Write(LV_STATUS,"uknown device, setting: %d to mac: %s",pRow_UnknownDevices->PK_UnknownDevices_get(),pRow_UnknownDevices->MacAddress_get().c_str());
}
//<-dceag-c239-b->

	/** @brief COMMAND: #239 - Request File And Checksum */
	/** Get the contents of a file from the server and the checksum of the file */
		/** @param #13 Filename */
			/** The file to get */
		/** @param #19 Data */
			/** The file's contents */
		/** @param #91 Checksum */
			/** The checksum of the file */
		/** @param #92 Checksum Only */
			/** If this is true, this command will return only the checksum of the file, Data will be null. */

void General_Info_Plugin::CMD_Request_File_And_Checksum(string sFilename,char **pData,int *iData_Size,string *sChecksum,bool *bChecksum_Only,string &sCMD_Result,Message *pMessage)
//<-dceag-c239-e->
{
	cout << "Command #239 - Request File And Checksum" << endl;
	cout << "Parm #13 - Filename=" << sFilename << endl;
	cout << "Parm #19 - Data  (data value)" << endl;
	cout << "Parm #91 - Checksum=" << sChecksum << endl;
	cout << "Parm #92 - Checksum_Only=" << bChecksum_Only << endl; 

	g_pPlutoLogger->Write(LV_FILEREQUEST, "request missing  file: %s", sFilename.c_str());
	size_t Length = 0;
	char *c = FileUtils::ReadFileIntoBuffer(sFilename, Length);
	if( c==NULL && m_pRouter )
		c = FileUtils::ReadFileIntoBuffer(m_pRouter->sBasePath_get() + sFilename, Length);

	if(c==NULL) //file not found
	{
		g_pPlutoLogger->Write(LV_FILEREQUEST, "The requested file '%s' was not found", sFilename.c_str());
		return;
	}

	*iData_Size = (int) Length;
	*pData = c;
    *sChecksum = FileUtils::FileChecksum(*pData, *iData_Size);

	if(*bChecksum_Only)
	{
		*iData_Size = 0;

		if(NULL != *pData)
		{
			delete *pData;
			*pData = NULL;
		}
	}
}
//<-dceag-c246-b->

	/** @brief COMMAND: #246 - Set Device Data */
	/** Gets the device data for a device */
		/** @param #2 PK_Device */
			/** The device to set */
		/** @param #5 Value To Assign */
			/** The value to assign */
		/** @param #52 PK_DeviceData */
			/** The device data */

void General_Info_Plugin::CMD_Set_Device_Data(int iPK_Device,string sValue_To_Assign,int iPK_DeviceData,string &sCMD_Result,Message *pMessage)
//<-dceag-c246-e->
{
}

//<-dceag-c247-b->

	/** @brief COMMAND: #247 - Get Device State */
	/** Gets a device's current state */

void General_Info_Plugin::CMD_Get_Device_State(string &sCMD_Result,Message *pMessage)
//<-dceag-c247-e->
{
}

//<-dceag-c248-b->

	/** @brief COMMAND: #248 - Get Device Status */
	/** Gets the status for a device */

void General_Info_Plugin::CMD_Get_Device_Status(string &sCMD_Result,Message *pMessage)
//<-dceag-c248-e->
{
}
//<-dceag-createinst-b->!
//<-dceag-c272-b->

	/** @brief COMMAND: #272 - Restart DCERouter */
	/** Causes DCERouter to exit and restart. */
		/** @param #21 Force */

void General_Info_Plugin::CMD_Restart_DCERouter(string sForce,string &sCMD_Result,Message *pMessage)
//<-dceag-c272-e->
{
// temp debugging since this wasn't going through
g_pPlutoLogger->Write(LV_STATUS, "Forwarding reload to router");
	Message *pMessageOut = new Message(pMessage->m_dwPK_Device_From,DEVICEID_DCEROUTER,PRIORITY_NORMAL,MESSAGETYPE_SYSCOMMAND,SYSCOMMAND_RELOAD,0);
	SendMessageToRouter(pMessageOut);
}
//<-dceag-c322-b->

	/** @brief COMMAND: #322 - Wake Device */
	/** Sends a Wake on LAN to the specified device. */
		/** @param #2 PK_Device */
			/** The device to wake up */

void General_Info_Plugin::CMD_Wake_Device(int iPK_Device,string &sCMD_Result,Message *pMessage)
//<-dceag-c322-e->
{
}

//<-dceag-c323-b->

	/** @brief COMMAND: #323 - Halt Device */
	/** Halts, or suspends, the given device. */
		/** @param #2 PK_Device */
			/** The device to halt */
		/** @param #21 Force */
			/** If Force is not specified this will do a suspend if the device supports suspend/resume, otherwise it will do a halt.  Force:  "H"=halt, "S"=suspend, "D"=Display off, "R"=reboot, "N"=net boot, "V"=hard drive boot */

void General_Info_Plugin::CMD_Halt_Device(int iPK_Device,string sForce,string &sCMD_Result,Message *pMessage)
//<-dceag-c323-e->
{
	DeviceData_Router *pDevice = m_pRouter->m_mapDeviceData_Router_Find(iPK_Device);
	if( !pDevice )
	{
		g_pPlutoLogger->Write(LV_CRITICAL,"Cannot halt unknown device %d",iPK_Device);
		return;
	}

	DeviceData_Router *pDevice_AppServer=NULL;
	if( pDevice->m_pDevice_Core )
		pDevice_AppServer = (DeviceData_Router *) ((DeviceData_Impl *) (pDevice->m_pDevice_Core))->FindSelfOrChildWithinCategory( DEVICECATEGORY_App_Server_CONST );

	if( !pDevice_AppServer && pDevice->m_pDevice_MD )
		pDevice_AppServer = (DeviceData_Router *) ((DeviceData_Impl *) (pDevice->m_pDevice_MD))->FindSelfOrChildWithinCategory( DEVICECATEGORY_App_Server_CONST );

	if( !pDevice_AppServer )
		pDevice_AppServer = (DeviceData_Router *) pDevice->FindSelfOrChildWithinCategory( DEVICECATEGORY_App_Server_CONST );

	if( !pDevice_AppServer )
	{
		g_pPlutoLogger->Write(LV_CRITICAL,"Cannot halt unknown device %d without an app server",iPK_Device);
		return;
	}

	if( sForce=="V" || sForce=="N" )
	{
		SetNetBoot(pDevice,sForce=="N");
		DCE::CMD_Halt_Device CMD_Halt_Device(m_dwPK_Device,pDevice_AppServer->m_dwPK_Device,pDevice->m_dwPK_Device,sForce.c_str());
		SendCommand(CMD_Halt_Device);
	}
	else
	{
		Message *pMessage_Out = new Message(pMessage);
		pMessage_Out->m_dwPK_Device_To = pDevice_AppServer->m_dwPK_Device;
		SendMessageToRouter(pMessage_Out);
	}
}

void General_Info_Plugin::SetNetBoot(DeviceData_Router *pDevice,bool bNetBoot)
{
	if( pDevice->m_sMacAddress.length()<17 )
	{
		g_pPlutoLogger->Write(LV_CRITICAL,"Cannot set boot on device %d with no Mac",pDevice->m_dwPK_Device);
		return;
	}

	string sFile = "/tftpboot/pxelinux.cfg/01-" + StringUtils::Replace(pDevice->m_sMacAddress,":","-");

	g_pPlutoLogger->Write(LV_STATUS,"Setting net boot for file %s to %d",sFile.c_str(),(int) bNetBoot);

	if( bNetBoot )
		StringUtils::Replace(sFile,sFile,"LOCALBOOT 0 #ERNEL ","KERNEL ");
	else
		StringUtils::Replace(sFile,sFile,"KERNEL ","LOCALBOOT 0 #ERNEL ");
}

//<-dceag-c365-b->

	/** @brief COMMAND: #365 - Get Room Description */
	/** Given a room or device ID, returns the description.  If device ID, also returns the room ID. */
		/** @param #2 PK_Device */
			/** The ID of the device.  Specify this or the room ID. */
		/** @param #9 Text */
			/** The description of the room */
		/** @param #57 PK_Room */
			/** The ID of the room.  Specify this or the device ID. */

void General_Info_Plugin::CMD_Get_Room_Description(int iPK_Device,string *sText,int *iPK_Room,string &sCMD_Result,Message *pMessage)
//<-dceag-c365-e->
{
	if( !(*iPK_Room ) )
	{
		DeviceData_Router *pDevice = m_pRouter->m_mapDeviceData_Router_Find(iPK_Device);
		if( !pDevice )
		{
			*sText = "Bad Device/Room";
			g_pPlutoLogger->Write(LV_CRITICAL,"Bad Device/room");
			return;
		}
		iPK_Device = pDevice->m_dwPK_Device;
		*iPK_Room = pDevice->m_dwPK_Room;
	}

	Room *pRoom = m_pRouter->m_mapRoom_Find(*iPK_Room);
	if( !pRoom )
	{
		*sText = "Bad Room";
		g_pPlutoLogger->Write(LV_CRITICAL,"Bad room");
		return;
	}

	*sText = pRoom->m_sDescription;
}
//<-dceag-c371-b->

	/** @brief COMMAND: #371 - Is Daytime */
	/** Returns true or false to indicate if it is daytime (ie between sunrise and sunset) */
		/** @param #119 True/False */
			/** Returns true if it is daytime. */

void General_Info_Plugin::CMD_Is_Daytime(bool *bTrueFalse,string &sCMD_Result,Message *pMessage)
//<-dceag-c371-e->
{
	(*bTrueFalse) = m_pEvent_Plugin->IsDaytime();
}

class DataGridTable *General_Info_Plugin::PendingTasks( string GridID, string Parms, void *ExtraData, int *iPK_Variable, string *sValue_To_Assign, class Message *pMessage )
{
	vector<string> vectPendingTasks;

    for(map<int,class Command_Impl *>::const_iterator it=m_pRouter->m_mapPlugIn_get()->begin();it!=m_pRouter->m_mapPlugIn_get()->end();++it)
    {
		Command_Impl *pPlugIn = (*it).second;
		// We don't care about the return code, just what tasks are pending
		pPlugIn->PendingTasks(&vectPendingTasks);
	}

    DataGridTable *pDataGrid = new DataGridTable( );
    DataGridCell *pCell;

	int RowCount=0;
	for(size_t s=0;s<vectPendingTasks.size();++s)
	{
        pCell = new DataGridCell( vectPendingTasks[s] );
        pDataGrid->SetData( 0, RowCount++, pCell );
	}

	return pDataGrid;
}

class DataGridTable *General_Info_Plugin::QuickStartApps( string GridID, string Parms, void *ExtraData, int *iPK_Variable, string *sValue_To_Assign, class Message *pMessage )
{
	PLUTO_SAFETY_LOCK(gm,m_GipMutex);
    DataGridTable *pDataGrid = new DataGridTable( );
    DataGridCell *pCellIcon,*pCellText;
	OH_Orbiter *pOH_Orbiter = m_pOrbiter_Plugin->m_mapOH_Orbiter_Find(pMessage->m_dwPK_Device_From);

	int PK_Device_MD=atoi(Parms.c_str());
	DeviceData_Router *pDevice_MD = m_pRouter->m_mapDeviceData_Router_Find(PK_Device_MD);
	Row_Device *pRow_Device = m_pDatabase_pluto_main->Device_get()->GetRow( PK_Device_MD );

	if( !pRow_Device || !pDevice_MD )
	{
		g_pPlutoLogger->Write(LV_CRITICAL,"QuickStartApps - invalid MD %s",Parms.c_str());
		return pDataGrid;
	}

	vector<DeviceData_Router *> vectDevice_AppServer;
	pDevice_MD->FindChildrenWithinCategory(DEVICECATEGORY_App_Server_CONST,vectDevice_AppServer);
	if( vectDevice_AppServer.size()==0 )
	{
		m_pOrbiter_Plugin->DisplayMessageOnOrbiter(pMessage->m_dwPK_Device_From,"There are no App Servers on this media director");
		return pDataGrid;
	}
	DeviceData_Router *pDevice_AppServer = vectDevice_AppServer[0];

	vector<DeviceData_Router *> vectDevice_Orbiter_OSD;
	pDevice_MD->FindChildrenWithinCategory(DEVICECATEGORY_Standard_Orbiter_CONST,vectDevice_Orbiter_OSD);
	if( vectDevice_Orbiter_OSD.size()==0 )
	{
		m_pOrbiter_Plugin->DisplayMessageOnOrbiter(pMessage->m_dwPK_Device_From,"There are is no OSD on this media director");
		return pDataGrid;
	}
	DeviceData_Router *pDevice_Orbiter_OSD = vectDevice_Orbiter_OSD[0];

	vector<Row_Device_QuickStart *> vectRow_Device_QuickStart;
	list<pair<string, string> > *p_Bookmarks=NULL;
	size_t s=0;
	list<pair<string, string> >::iterator it;
	if( ExtraData )
	{
		p_Bookmarks = (list<pair<string, string> > *) ExtraData;
		it=p_Bookmarks->begin();
		if( it == p_Bookmarks->end() )
			return pDataGrid;
	}
	else
		pRow_Device->Device_QuickStart_FK_Device_getrows(&vectRow_Device_QuickStart);

	LastApplication *pLastApplication = m_mapLastApplication_Find(pDevice_MD->m_dwPK_Device);
	if( pLastApplication && pLastApplication->m_sName.size() )
	{
		int PK_DesignObj_OSD=DESIGNOBJ_generic_app_full_screen_CONST;
		int PK_DesignObj_Remote=DESIGNOBJ_mnuGenericAppController_CONST;
		if( pLastApplication->m_iPK_QuickStartTemplate )
		{
			Row_QuickStartTemplate *pRow_QuickStartTemplate = m_pDatabase_pluto_main->QuickStartTemplate_get()->GetRow(pLastApplication->m_iPK_QuickStartTemplate);
			if( pRow_QuickStartTemplate )
			{
				PK_DesignObj_OSD=pRow_QuickStartTemplate->FK_DesignObj_OSD_get();
				PK_DesignObj_Remote=pRow_QuickStartTemplate->FK_DesignObj_get();;
			}
		}

		string sMessage;
		if( pDevice_Orbiter_OSD->m_dwPK_Device==pMessage->m_dwPK_Device_From )
			sMessage = StringUtils::itos(m_dwPK_Device) + " " + StringUtils::itos(pMessage->m_dwPK_Device_From) +
				" 1 " + StringUtils::itos(COMMAND_Goto_Screen_CONST) + " " + StringUtils::itos(COMMANDPARAMETER_PK_DesignObj_CONST) + " " + 
				StringUtils::itos(PK_DesignObj_OSD);
		else
			sMessage = StringUtils::itos(m_dwPK_Device) + " " + StringUtils::itos(pMessage->m_dwPK_Device_From) +
				" 1 " + StringUtils::itos(COMMAND_Goto_Screen_CONST) + " " + StringUtils::itos(COMMANDPARAMETER_PK_DesignObj_CONST) + " " + 
				StringUtils::itos(PK_DesignObj_Remote) + " & " +
				StringUtils::itos(m_dwPK_Device) + " " + StringUtils::itos(pDevice_Orbiter_OSD->m_dwPK_Device) +
				" 1 " + StringUtils::itos(COMMAND_Goto_Screen_CONST) + " " + StringUtils::itos(COMMANDPARAMETER_PK_DesignObj_CONST) + " " + 
				StringUtils::itos(PK_DesignObj_OSD);
				
		DCE::CMD_Show_Object CMD_Show_Object(m_dwPK_Device,pMessage->m_dwPK_Device_From,StringUtils::itos(DESIGNOBJ_butResumeControl_CONST),
			0,"","","1");
		DCE::CMD_Show_Object CMD_Show_Object2(m_dwPK_Device,pMessage->m_dwPK_Device_From,StringUtils::itos(DESIGNOBJ_objExitAppOnOSD_CONST),
			0,"","","1");
		DCE::CMD_Set_Text CMD_Set_Text(m_dwPK_Device,pMessage->m_dwPK_Device_From,"",pLastApplication->m_sName,TEXT_STATUS_CONST);
		DCE::CMD_Set_Variable CMD_Set_Variable(m_dwPK_Device,pMessage->m_dwPK_Device_From,VARIABLE_Misc_Data_1_CONST,sMessage);

		CMD_Show_Object.m_pMessage->m_vectExtraMessages.push_back(CMD_Show_Object2.m_pMessage);
		CMD_Show_Object.m_pMessage->m_vectExtraMessages.push_back(CMD_Set_Text.m_pMessage);
		CMD_Show_Object.m_pMessage->m_vectExtraMessages.push_back(CMD_Set_Variable.m_pMessage);
		SendCommand(CMD_Show_Object);
	}

	// This will be the template we use for string applications
	Row_QuickStartTemplate *pRow_QuickStartTemplate = p_Bookmarks ? m_pDatabase_pluto_main->QuickStartTemplate_get()->GetRow(1) : NULL;

	int iRow=0;
	while(true)
	{
		string sDescription,sBinary,sArguments;
		size_t iSize;
		char *pBuffer=NULL;
		int PK_DesignObj_OSD=DESIGNOBJ_generic_app_full_screen_CONST;
		int PK_DesignObj_Remote=DESIGNOBJ_mnuGenericAppController_CONST;

		if( p_Bookmarks )
		{
			sDescription=it->second;
			sBinary=pRow_QuickStartTemplate ? pRow_QuickStartTemplate->Binary_get() : "/usr/pluto/bin/Mozilla.sh";
			sArguments=StringUtils::itos(pOH_Orbiter && pOH_Orbiter->m_pOH_User ? pOH_Orbiter->m_pOH_User->m_iPK_Users : 0) + "\t" + it->first;
		}
		else
		{
            if(!vectRow_Device_QuickStart.size())
                return pDataGrid;

			Row_Device_QuickStart *pRow_Device_QuickStart = vectRow_Device_QuickStart[s];
			sDescription = pRow_Device_QuickStart->Description_get();
			sBinary = pRow_Device_QuickStart->Binary_get();
			sArguments = pRow_Device_QuickStart->Arguments_get();
			pRow_QuickStartTemplate=pRow_Device_QuickStart->FK_QuickStartTemplate_getrow();
            if( pRow_Device_QuickStart->EK_Picture_get() )
                pBuffer = FileUtils::ReadFileIntoBuffer("/home/mediapics/" + StringUtils::itos(pRow_Device_QuickStart->EK_Picture_get()) + "_tn.jpg",iSize);
		}

		if( pRow_QuickStartTemplate )
		{
			if( pRow_QuickStartTemplate->FK_DesignObj_get() )
				PK_DesignObj_Remote=pRow_QuickStartTemplate->FK_DesignObj_get();
			if( pRow_QuickStartTemplate->FK_DesignObj_OSD_get() )
				PK_DesignObj_OSD=pRow_QuickStartTemplate->FK_DesignObj_OSD_get();
			if( !pBuffer )
                pBuffer = FileUtils::ReadFileIntoBuffer("/usr/pluto/orbiter/quickstart/" + StringUtils::itos(pRow_QuickStartTemplate->PK_QuickStartTemplate_get()) + "_tn.jpg",iSize);
		}

		if( sBinary.size()==0 )
		{
			g_pPlutoLogger->Write(LV_WARNING,"QuickStart device with no binary to run");
			continue;
		}

		if( sDescription.size()==0 )
			sDescription = sBinary;

		pCellIcon = new DataGridCell( "", "" );
		pCellText = new DataGridCell( sDescription, "" );
		pCellText->m_Colspan=3;
		pDataGrid->SetData( 0, iRow, pCellIcon );
		pDataGrid->SetData( 1, iRow, pCellText );
        if( pBuffer )
        {
            pCellIcon->m_pGraphicData = pBuffer;
            pCellIcon->m_GraphicLength = iSize;
        }

		string sMessage = "0 " + StringUtils::itos(pDevice_Orbiter_OSD->m_dwPK_Device) + 
			" 1 4 16 " + StringUtils::itos(PK_DesignObj_OSD) + 
			" & 0 " + StringUtils::itos(m_dwPK_Device) + " 1 " + StringUtils::itos(COMMAND_Set_Active_Application_CONST) +
			" " + StringUtils::itos(COMMANDPARAMETER_Name_CONST) + " \"\" " + StringUtils::itos(COMMANDPARAMETER_PK_Device_CONST) + " " +
			StringUtils::itos(pDevice_MD->m_dwPK_Device);
		if( pDevice_Orbiter_OSD->m_dwPK_Device!=pMessage->m_dwPK_Device_From )
			sMessage += " & 0 " + StringUtils::itos(pMessage->m_dwPK_Device_From ) + 
				" 1 4 16 " + StringUtils::itos(PK_DesignObj_Remote);


		DCE::CMD_Spawn_Application CMD_Spawn_Application(m_dwPK_Device,pDevice_AppServer->m_dwPK_Device,
			sBinary,"generic_app",sArguments,
			sMessage,sMessage,true,false,true);
		pCellIcon->m_pMessage = CMD_Spawn_Application.m_pMessage;
		pCellText->m_pMessage = new Message(pCellIcon->m_pMessage);

		DCE::CMD_Set_Active_Application CMD_Set_Active_Application(pMessage->m_dwPK_Device_From,m_dwPK_Device,
			pDevice_MD->m_dwPK_Device,sDescription,pRow_QuickStartTemplate ? pRow_QuickStartTemplate->PK_QuickStartTemplate_get() : 0);
		pCellIcon->m_pMessage->m_vectExtraMessages.push_back(CMD_Set_Active_Application.m_pMessage);
		pCellText->m_pMessage->m_vectExtraMessages.push_back( new Message(CMD_Set_Active_Application.m_pMessage) );


		// If this is the same on screen orbiter on which the app will run, we will send the user 
		// to a screen that retains a small strip at the bottom to terminate the app and return to the orbiter.
		// Otherwise, we will send the OSD to the full screen app, and the orbiter will become a remote control
		if( pDevice_Orbiter_OSD->m_dwPK_Device==pMessage->m_dwPK_Device_From )
		{
			// This is the OSD orbiter
			DCE::CMD_Goto_Screen CMD_Goto_Screen(m_dwPK_Device,pDevice_Orbiter_OSD->m_dwPK_Device,0,
				StringUtils::itos(PK_DesignObj_OSD),"","",false,false);
			pCellIcon->m_pMessage->m_vectExtraMessages.push_back(CMD_Goto_Screen.m_pMessage);
			pCellText->m_pMessage->m_vectExtraMessages.push_back(new Message(CMD_Goto_Screen.m_pMessage));

			DCE::CMD_Set_Variable CMD_Set_Variable(m_dwPK_Device,pDevice_Orbiter_OSD->m_dwPK_Device,
				VARIABLE_Array_Desc_CONST,sDescription);
			pCellIcon->m_pMessage->m_vectExtraMessages.push_back(CMD_Set_Variable.m_pMessage);
			pCellText->m_pMessage->m_vectExtraMessages.push_back(new Message(CMD_Set_Variable.m_pMessage));
		}
		else
		{
			// Do this on the OSD orbiter
			DCE::CMD_Goto_Screen CMD_Goto_Screen(m_dwPK_Device,pDevice_Orbiter_OSD->m_dwPK_Device,0,
				StringUtils::itos(PK_DesignObj_OSD),"","",false,false);
			pCellIcon->m_pMessage->m_vectExtraMessages.push_back(CMD_Goto_Screen.m_pMessage);
			pCellText->m_pMessage->m_vectExtraMessages.push_back(new Message(CMD_Goto_Screen.m_pMessage));

			// Do this on the controlling orbiter
			DCE::CMD_Goto_Screen CMD_Goto_Screen2(m_dwPK_Device,pMessage->m_dwPK_Device_From,0,
				StringUtils::itos(PK_DesignObj_Remote),"","",false,false);
			pCellIcon->m_pMessage->m_vectExtraMessages.push_back(CMD_Goto_Screen2.m_pMessage);
			pCellText->m_pMessage->m_vectExtraMessages.push_back(new Message(CMD_Goto_Screen2.m_pMessage));
		}

		if( (p_Bookmarks && ++it == p_Bookmarks->end()) || 
			(!p_Bookmarks && ++s>=vectRow_Device_QuickStart.size()) )
				break;
		iRow++;
	}

	return pDataGrid;
}

class DataGridTable *General_Info_Plugin::MRUDocuments( string GridID, string Parms, void *ExtraData, int *iPK_Variable, string *sValue_To_Assign, class Message *pMessage )
{
    DataGridTable *pDataGrid = new DataGridTable( );
    DataGridCell *pCell;


	return pDataGrid;
}

class DataGridTable *General_Info_Plugin::Rooms( string GridID, string Parms, void *ExtraData, int *iPK_Variable, string *sValue_To_Assign, class Message *pMessage )
{
	int iWidth = atoi(pMessage->m_mapParameters[COMMANDPARAMETER_Width_CONST].c_str());
	if( !iWidth )
		iWidth = 4;

    DataGridTable *pDataGrid = new DataGridTable( );
    DataGridCell *pCell;

	int iRow=0,iCol=0;
	string sql = "SELECT PK_Room,Description FROM Room WHERE FK_Installation=" + StringUtils::itos(m_pRouter->iPK_Installation_get()) + " ORDER BY Description";
	PlutoSqlResult result;
    MYSQL_ROW row;
	if( mysql_query(m_pDatabase_pluto_main->m_pMySQL,sql.c_str())==0 && (result.r = mysql_store_result(m_pDatabase_pluto_main->m_pMySQL)) )
    {
        while( ( row=mysql_fetch_row( result.r ) ) )
		{
			pCell = new DataGridCell( row[1], row[0] );
			pDataGrid->SetData( iCol++, iRow, pCell );
			if( iCol>=iWidth )
			{
				iCol=0;
				iRow++;
			}
		}
	}

	return pDataGrid;
}

//<-dceag-c395-b->

	/** @brief COMMAND: #395 - Check for updates */
	/** Check all PC's in the system to see if there are available updates on any of them by having all AppServer's run /usr/pluto/bin/Config_Device_Changes.sh */

void General_Info_Plugin::CMD_Check_for_updates(string &sCMD_Result,Message *pMessage)
//<-dceag-c395-e->
{
	PLUTO_SAFETY_LOCK(gm,m_GipMutex);

	if( PendingConfigs() )
	{
		g_pPlutoLogger->Write(LV_STATUS,"Schedule a m_bRerunConfigWhenDone");
		m_bRerunConfigWhenDone=true;
		return;
	}

	ListDeviceData_Router *pListDeviceData_Router = 
		m_pRouter->m_mapDeviceByTemplate_Find(DEVICETEMPLATE_App_Server_CONST);

	g_pPlutoLogger->Write(LV_WARNING,"General plugin checking for updates %p",pListDeviceData_Router);

	if( !pListDeviceData_Router )
		return;

	DeviceData_Router *pDevice_AppServerOnCore=NULL; // We will use this to be sure we don't run 2 app server's
	bool bAlreadyRanOnCore=false;
	for(ListDeviceData_Router::iterator it=pListDeviceData_Router->begin();it!=pListDeviceData_Router->end();++it)
	{
		DeviceData_Router *pDevice = *it;
		if( pDevice->m_pDevice_ControlledVia && pDevice->m_pDevice_ControlledVia->WithinCategory(DEVICECATEGORY_Media_Director_CONST) )
		{
			if( pDevice->m_pDevice_Core )
				bAlreadyRanOnCore=true;

			if( !m_mapMediaDirectors_PendingConfig[pDevice->m_pDevice_ControlledVia->m_dwPK_Device] )
			{
				DCE::CMD_Spawn_Application CMD_Spawn_Application(m_dwPK_Device,pDevice->m_dwPK_Device,"/usr/pluto/bin/Config_Device_Changes.sh","cdc",
					"F","",StringUtils::itos(pDevice->m_dwPK_Device) + " " + StringUtils::itos(m_dwPK_Device) + " " +
					StringUtils::itos(MESSAGETYPE_COMMAND) + " " + StringUtils::itos(COMMAND_Check_for_updates_done_CONST),false,false,false);
				string sResponse;
				if( !SendCommand(CMD_Spawn_Application,&sResponse) || sResponse!="OK" )
					g_pPlutoLogger->Write(LV_CRITICAL,"Failed to send spawn application to %d",pDevice->m_dwPK_Device);
				else
					m_mapMediaDirectors_PendingConfig[pDevice->m_pDevice_ControlledVia->m_dwPK_Device]=true;
			}
		}
		else if( pDevice->m_pDevice_ControlledVia && pDevice->m_pDevice_ControlledVia->WithinCategory(DEVICECATEGORY_Core_CONST) )
			pDevice_AppServerOnCore = pDevice;
	}

	if( pDevice_AppServerOnCore && !bAlreadyRanOnCore && !m_mapMediaDirectors_PendingConfig[pDevice_AppServerOnCore->m_pDevice_ControlledVia->m_dwPK_Device] )
	{
		DCE::CMD_Spawn_Application CMD_Spawn_Application(m_dwPK_Device,pDevice_AppServerOnCore->m_dwPK_Device,"/usr/pluto/bin/Config_Device_Changes.sh","cdc",
			"F","",StringUtils::itos(pDevice_AppServerOnCore->m_dwPK_Device) + " " + StringUtils::itos(m_dwPK_Device) + " " +
			StringUtils::itos(MESSAGETYPE_COMMAND) + " " + StringUtils::itos(COMMAND_Check_for_updates_done_CONST),false,false,false);
		string sResponse;
		if( !SendCommand(CMD_Spawn_Application,&sResponse) || sResponse!="OK" )
			g_pPlutoLogger->Write(LV_CRITICAL,"Failed to send spawn application to %d",pDevice_AppServerOnCore->m_dwPK_Device);
		else
			m_mapMediaDirectors_PendingConfig[pDevice_AppServerOnCore->m_pDevice_ControlledVia->m_dwPK_Device]=true;
	}
}

//<-dceag-c396-b->

	/** @brief COMMAND: #396 - Check for updates done */
	/** An App Server finished running /usr/pluto/bin/Config_Device_Changes.sh and notifies the g.i. plugin. */

void General_Info_Plugin::CMD_Check_for_updates_done(string &sCMD_Result,Message *pMessage)
//<-dceag-c396-e->
{
	PLUTO_SAFETY_LOCK(gm,m_GipMutex);

	DeviceData_Router *pDevice_AppServer = m_pRouter->m_mapDeviceData_Router_Find(pMessage->m_dwPK_Device_From);
	if( !pDevice_AppServer || !pDevice_AppServer->m_pDevice_ControlledVia )
		return; // Shouldn't happen

	m_mapMediaDirectors_PendingConfig[pDevice_AppServer->m_pDevice_ControlledVia->m_dwPK_Device]=false;
	g_pPlutoLogger->Write(LV_STATUS,"device %d done config update",pMessage->m_dwPK_Device_From);

	if( PendingConfigs() )
		return; // Others are still running

	if( m_bRerunConfigWhenDone )
	{
		m_bRerunConfigWhenDone=false;
		g_pPlutoLogger->Write(LV_STATUS,"New requests came in the meantime.  Rerun again");
		CMD_Check_for_updates();  // Do it again
	}
	else
	{
		g_pPlutoLogger->Write(LV_STATUS,"Done updating config");
		m_pOrbiter_Plugin->DoneCheckingForUpdates();
	}
}

class DataGridTable * General_Info_Plugin::BookmarkList(string GridID, string Parms, void * ExtraData,int *iPK_Variable,string *sValue_To_Assign, class Message *pMessage)
{
	string::size_type pos=0;
	StringUtils::Tokenize(Parms,",",pos);  // Skip the MD
	list<pair<string, string> > Bookmarks = GetUserBookmarks(StringUtils::Tokenize(Parms,",",pos));
	return QuickStartApps( GridID, Parms, (void *) &Bookmarks, iPK_Variable, sValue_To_Assign, pMessage );
}

pair<string, string> strpair(string x, string y)
{
	return pair<string, string>(x, y);
}

list<pair<string, string> > General_Info_Plugin::GetUserBookmarks(string sPK_User)
{
    list<pair<string, string> > Bookmarks;
    // the following code reads the Mozilla bookmarks

    g_pPlutoLogger->Write(LV_CRITICAL,"Reading bookmarks from %s", 
        ("/home/user_" + sPK_User + "/bookmarks.html").c_str());

    size_t Size;
    char *Buffer = FileUtils::ReadFileIntoBuffer("/home/user_" + sPK_User + "/bookmarks.html", Size);
    if( Buffer )
    {
        char * BufferTop = Buffer;
        char *BraceA;
        char *PosInBuffer=Buffer;
        while( (BraceA=strstr(Buffer,"<A")) )
        {
            char *HRef = strstr(BraceA,"HREF");
            if( !HRef )
            {
                Buffer++;  // Skip past this and continue
                continue;
            }

            char *FirstQuote = strchr(HRef,'"');
            if( !FirstQuote )
            {
                Buffer++;  // Skip past this and continue
                continue;
            }

            char *SecondQuote = strchr(FirstQuote+1,'"');
            if( !SecondQuote )
            {
                Buffer++;  // Skip past this and continue
                continue;
            }
            *SecondQuote=0;

            string Link(FirstQuote+1);
            char *LastBrace = strchr(SecondQuote+1,'>');
            if( !LastBrace )
            {
                Buffer++;  // Skip past this and continue
                continue;
            }

            char * EndBraceA = strstr(LastBrace+1, "</A>");
            *EndBraceA = 0;
            string LinkText(LastBrace+1);

            Buffer = EndBraceA+1;
            g_pPlutoLogger->Write(LV_CRITICAL,"add bookmarks %s / %s",Link.c_str(), LinkText.c_str());
            Bookmarks.push_back(pair<string, string>(Link, LinkText));
        }

        delete[] BufferTop;
    }

    if( Bookmarks.size()==0 )
    {
        Bookmarks.push_back(make_pair<string,string> ("http://dcerouter/pluto-admin","Pluto Admin"));
        Bookmarks.push_back(make_pair<string,string> ("http://dcerouter/support","Pluto Support"));
    }

    return Bookmarks;
}
//<-dceag-c697-b->

	/** @brief COMMAND: #697 - Set Active Application */
	/** Tell General Info Plugin what computing application is running on an MD */
		/** @param #2 PK_Device */
			/** The media director */
		/** @param #50 Name */
			/** The name of the application */
		/** @param #146 PK_QuickStartTemplate */
			/** The quick start template */

void General_Info_Plugin::CMD_Set_Active_Application(int iPK_Device,string sName,int iPK_QuickStartTemplate,string &sCMD_Result,Message *pMessage)
//<-dceag-c697-e->
{
	PLUTO_SAFETY_LOCK(gm,m_GipMutex);
	LastApplication *pLastApplication = m_mapLastApplication_Find(iPK_Device);
	if( !pLastApplication )
	{
		pLastApplication = new LastApplication();
		m_mapLastApplication[iPK_Device]=pLastApplication;
	}
	pLastApplication->m_sName=sName;
	pLastApplication->m_iPK_QuickStartTemplate=iPK_QuickStartTemplate;
}
