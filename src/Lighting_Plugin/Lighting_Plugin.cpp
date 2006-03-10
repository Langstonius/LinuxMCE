/*
 Lighting_Plugin
 
 Copyright (C) 2004 Pluto, Inc., a Florida Corporation
 
 www.plutohome.com		
 
 Phone: +1 (877) 758-8648
 
 This program is distributed according to the terms of the Pluto Public License, available at: 
 http://plutohome.com/index.php?section=public_license 
 
 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 or FITNESS FOR A PARTICULAR PURPOSE. See the Pluto Public License for more details.
 
 */

//<-dceag-d-b->
#include "Lighting_Plugin.h"
#include "DCE/Logger.h"
#include "PlutoUtils/FileUtils.h"
#include "PlutoUtils/StringUtils.h"
#include "PlutoUtils/Other.h"

#include <iostream>
using namespace std;
using namespace DCE;

#include "Gen_Devices/AllCommandsRequests.h"
//<-dceag-d-e->

#include "DCERouter.h"
#include "Datagrid_Plugin/Datagrid_Plugin.h"
#include "DCE/DataGrid.h"
#include "Orbiter_Plugin/Orbiter_Plugin.h"
#include "pluto_main/Database_pluto_main.h"
#include "pluto_main/Table_CommandGroup.h"
#include "pluto_main/Table_CommandGroup_Room.h"
#include "pluto_main/Define_Array.h"
#include "pluto_main/Define_DeviceTemplate.h"
#include "pluto_main/Define_DataGrid.h"
#include "pluto_main/Define_DeviceCategory.h"
#include "pluto_main/Define_Command.h"
#include "pluto_main/Define_CommandParameter.h"
#include "pluto_main/Define_Event.h"
#include "pluto_main/Define_FloorplanObjectType.h"
#include "pluto_main/Define_FloorplanObjectType_Color.h"

//<-dceag-const-b->
// The primary constructor when the class is created as a stand-alone device
Lighting_Plugin::Lighting_Plugin(int DeviceID, string ServerAddress,bool bConnectEventHandler,bool bLocalMode,class Router *pRouter)
	: Lighting_Plugin_Command(DeviceID, ServerAddress,bConnectEventHandler,bLocalMode,pRouter)
//<-dceag-const-e->
{
	m_pDatabase_pluto_main = NULL;
}

//<-dceag-getconfig-b->
bool Lighting_Plugin::GetConfig()
{
	if( !Lighting_Plugin_Command::GetConfig() )
		return false;
//<-dceag-getconfig-e->

	m_pDatabase_pluto_main = new Database_pluto_main( );
	if( !m_pDatabase_pluto_main->Connect( m_pRouter->sDBHost_get( ), m_pRouter->sDBUser_get( ), m_pRouter->sDBPassword_get( ), m_pRouter->sDBName_get( ), m_pRouter->iDBPort_get( ) ) )
	{
		g_pPlutoLogger->Write( LV_CRITICAL, "Cannot connect to database!" );
		m_bQuit=true;
		return false;
	}

	// The first 2 command groups for each room are considered the 'on' and 'off'
	string sWhere = "JOIN CommandGroup ON FK_CommandGroup=PK_CommandGroup WHERE FK_Array=" + StringUtils::itos(ARRAY_Lighting_Scenarios_CONST) + " ORDER BY FK_Room,Sort";
	vector<Row_CommandGroup_Room *> vectRow_CommandGroup_Room;
	m_pDatabase_pluto_main->CommandGroup_Room_get()->GetRows(sWhere,&vectRow_CommandGroup_Room);

	int PK_CommandGroup_On,PK_CommandGroup_Off,PK_Room=0;
	for(size_t s=0;s<vectRow_CommandGroup_Room.size();++s)
	{
		Row_CommandGroup_Room *pRow_CommandGroup_Room = vectRow_CommandGroup_Room[s];
		if( !PK_Room || pRow_CommandGroup_Room->FK_Room_get()!=PK_Room )
		{
			// This is a new room
			PK_Room = pRow_CommandGroup_Room->FK_Room_get();
			PK_CommandGroup_On = pRow_CommandGroup_Room->FK_CommandGroup_get();
			PK_CommandGroup_Off = 0;
		}
		else if( !PK_CommandGroup_Off )
		{
			PK_CommandGroup_Off = pRow_CommandGroup_Room->FK_CommandGroup_get();
			m_mapRoom_CommandGroup[PK_Room] = longPair(PK_CommandGroup_On,PK_CommandGroup_Off);
		}
	}
	return true;
}

//<-dceag-const2-b->!

//<-dceag-dest-b->
Lighting_Plugin::~Lighting_Plugin()
//<-dceag-dest-e->
{
	delete m_pDatabase_pluto_main;
	
}

//<-dceag-reg-b->
// This function will only be used if this device is loaded into the DCE Router's memory space as a plug-in.  Otherwise Connect() will be called from the main()
bool Lighting_Plugin::Register()
//<-dceag-reg-e->
{
	m_pDatagrid_Plugin=( Datagrid_Plugin * ) m_pRouter->FindPluginByTemplate(DEVICETEMPLATE_Datagrid_Plugin_CONST);
	m_pOrbiter_Plugin=( Orbiter_Plugin * ) m_pRouter->FindPluginByTemplate(DEVICETEMPLATE_Orbiter_Plugin_CONST);
	if( !m_pDatagrid_Plugin || !m_pOrbiter_Plugin )
	{
		g_pPlutoLogger->Write(LV_CRITICAL,"Cannot find sister plugins");
		return false;
	}

	m_pDatagrid_Plugin->RegisterDatagridGenerator( 
		new DataGridGeneratorCallBack( this, ( DCEDataGridGeneratorFn )( &Lighting_Plugin::LightingScenariosGrid ) )
		, DATAGRID_Lighting_Scenarios_CONST,PK_DeviceTemplate_get() );

    RegisterMsgInterceptor(( MessageInterceptorFn )( &Lighting_Plugin::LightingCommand ), 0, 0, 0, DEVICECATEGORY_Lighting_Device_CONST, MESSAGETYPE_COMMAND, 0 );
    RegisterMsgInterceptor(( MessageInterceptorFn )( &Lighting_Plugin::LightingFollowMe ), 0, 0, 0, 0, MESSAGETYPE_EVENT, EVENT_Follow_Me_Lighting_CONST );
    RegisterMsgInterceptor(( MessageInterceptorFn )( &Lighting_Plugin::DeviceOnOff ), 0, 0, 0, 0, MESSAGETYPE_EVENT, EVENT_Device_OnOff_CONST );

	m_pListDeviceData_Router_Lights = m_pRouter->m_mapDeviceByCategory_Find(DEVICECATEGORY_Lighting_Device_CONST);

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
void Lighting_Plugin::ReceivedCommandForChild(DeviceData_Impl *pDeviceData_Impl,string &sCMD_Result,Message *pMessage)
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
void Lighting_Plugin::ReceivedUnknownCommand(string &sCMD_Result,Message *pMessage)
//<-dceag-cmduk-e->
{
	sCMD_Result = "UNKNOWN DEVICE";
}

class DataGridTable *Lighting_Plugin::LightingScenariosGrid( string GridID, string Parms, void *ExtraData, int *iPK_Variable, string *sValue_To_Assign, class Message *pMessage )
{
	DataGridTable *pDataGrid = new DataGridTable( );
	DataGridCell *pCell;

	vector<Row_CommandGroup *> vectRowCommandGroup;
	m_pDatabase_pluto_main->CommandGroup_get( )->GetRows( COMMANDGROUP_FK_ARRAY_FIELD + string( "=" ) + StringUtils::itos( ARRAY_Lighting_Scenarios_CONST ) + " AND " 
			+ COMMANDGROUP_FK_INSTALLATION_FIELD + "=" + StringUtils::itos( m_pRouter->iPK_Installation_get( ) ), &vectRowCommandGroup );
	for( size_t s=0;s<vectRowCommandGroup.size( );++s )
	{
		Row_CommandGroup *pRow_CommandGroup = vectRowCommandGroup[s];
		pCell = new DataGridCell( pRow_CommandGroup->Description_get( ), StringUtils::itos( pRow_CommandGroup->PK_CommandGroup_get( ) ) );
		pDataGrid->SetData( 0, ( int ) s, pCell );
	}

	return pDataGrid;
}

bool Lighting_Plugin::DeviceOnOff( class Socket *pSocket, class Message *pMessage, class DeviceData_Base *pDeviceFrom, class DeviceData_Base *pDeviceTo )
{
    // This only runs as a plug-in so we can safely cast it
    class DeviceData_Router *pDevice_RouterFrom = (class DeviceData_Router *) pDeviceFrom;
    
	if( pMessage->m_dwID == EVENT_Device_OnOff_CONST )
	{
		string sLevel = pMessage->m_mapParameters[EVENTPARAMETER_OnOff_CONST];
		if(sLevel == "0")
		{
			pDevice_RouterFrom->m_sState_set("OFF/" + StringUtils::itos(GetLightingLevel(pDevice_RouterFrom,0)));
		}
		else if(sLevel == "1")
		{
			pDevice_RouterFrom->m_sState_set("ON/" + StringUtils::itos(GetLightingLevel(pDevice_RouterFrom,100)));
		}
		else
		{
			g_pPlutoLogger->Write(LV_WARNING, "Received OnOff event with wrong parameter value %s",
				sLevel.c_str());
		}
	}
	
	return true;
}

bool Lighting_Plugin::LightingFollowMe( class Socket *pSocket, class Message *pMessage, class DeviceData_Base *pDeviceFrom, class DeviceData_Base *pDeviceTo )
{
	return HandleFollowMe(pMessage);
}

bool Lighting_Plugin::LightingCommand( class Socket *pSocket, class Message *pMessage, class DeviceData_Base *pDeviceFrom, class DeviceData_Base *pDeviceTo )
{
	// This only runs as a plug-in so we can safely cast it
	class DeviceData_Router *pDevice_RouterTo = (class DeviceData_Router *) pDeviceTo;
	PreprocessLightingMessage(pDevice_RouterTo,pMessage);

	if( pMessage->m_sPK_Device_List_To.length() ) 
	{
		int PK_Device;  size_t pos=0;
		while( true )
		{
			PK_Device=atoi(StringUtils::Tokenize(pMessage->m_sPK_Device_List_To,",",pos).c_str());
			DeviceData_Router *pDeviceData_Router = m_pRouter->m_mapDeviceData_Router_Find(PK_Device);
			PreprocessLightingMessage(pDeviceData_Router,pMessage);
			if( pos>=pMessage->m_sPK_Device_List_To.length() || pos==string::npos )
				break;
		}
	}

	return true;
}

void Lighting_Plugin::GetFloorplanDeviceInfo(DeviceData_Router *pDeviceData_Router,EntertainArea *pEntertainArea,int iFloorplanObjectType,int &iPK_FloorplanObjectType_Color,int &Color,string &sDescription,string &OSD,int &PK_DesignObj_Toolbar)
{
	switch(iFloorplanObjectType)
	{
	case FLOORPLANOBJECTTYPE_LIGHT_CEILING_LIGHT_CONST:
	case FLOORPLANOBJECTTYPE_LIGHT_TABLE_LAMP_CONST:
	case FLOORPLANOBJECTTYPE_LIGHT_WALL_SCONCE_CONST:
	case FLOORPLANOBJECTTYPE_LIGHT_FLOOR_LAMP_CONST:
	case FLOORPLANOBJECTTYPE_LIGHT_CHANDALIER_CONST:
	case FLOORPLANOBJECTTYPE_LIGHT_PICTURE_LIGHT_CONST:
	case FLOORPLANOBJECTTYPE_LIGHT_ACCENT_LIGHT_CONST:
		if( (OSD=pDeviceData_Router->m_sState_get())=="OFF" )
			iPK_FloorplanObjectType_Color = FLOORPLANOBJECTTYPE_COLOR_LIGHT_CEILING_LIGHT_OFF_CONST;
		else
			iPK_FloorplanObjectType_Color = FLOORPLANOBJECTTYPE_COLOR_LIGHT_CEILING_LIGHT_ON_CONST;
		
		PK_DesignObj_Toolbar=DESIGNOBJ_grpLightControls_CONST;
	};
}

//<-dceag-sample-b->!

/**

	@brief COMMANDS TO IMPLEMENT

*/


void Lighting_Plugin::FollowMe_EnteredRoom(int iPK_Event, int iPK_Orbiter, int iPK_Users, int iPK_RoomOrEntArea, int iPK_RoomOrEntArea_Left)
{
g_pPlutoLogger->Write(LV_WARNING,"Lighting entered room, exec %d",m_mapRoom_CommandGroup[iPK_RoomOrEntArea].first);
	ExecCommandGroup(m_mapRoom_CommandGroup[iPK_RoomOrEntArea].first);
}

void Lighting_Plugin::FollowMe_LeftRoom(int iPK_Event, int iPK_Orbiter, int iPK_Users, int iPK_RoomOrEntArea, int iPK_RoomOrEntArea_Left)
{
g_pPlutoLogger->Write(LV_WARNING,"Lighting left room, exec %d",m_mapRoom_CommandGroup[iPK_RoomOrEntArea_Left].second);
	ExecCommandGroup(m_mapRoom_CommandGroup[iPK_RoomOrEntArea_Left].second);
}
//<-dceag-createinst-b->!
//<-dceag-c184-b->

	/** @brief COMMAND: #184 - Set Level */
	/** Went sent by an orbiter, the plugin will adjust all lights in the Orbiter's room */
		/** @param #76 Level */
			/** The level to set, as a value between 0 (off) and 100 (full).  It can be preceeded with a - or + indicating a relative value.  +20 means up 20%. */

void Lighting_Plugin::CMD_Set_Level(string sLevel,string &sCMD_Result,Message *pMessage)
//<-dceag-c184-e->
{
	OH_Orbiter *pOH_Orbiter = m_pOrbiter_Plugin->m_mapOH_Orbiter_Find(pMessage->m_dwPK_Device_From);
	if( !pOH_Orbiter || !m_pListDeviceData_Router_Lights )
	{
		g_pPlutoLogger->Write(LV_CRITICAL,"Device %d sent a set lighting_level but it's not an orbiter or we have no lights %p",
			pMessage->m_dwPK_Device_From,m_pListDeviceData_Router_Lights);
		return;
	}

	for(ListDeviceData_Router::iterator it=m_pListDeviceData_Router_Lights->begin();it!=m_pListDeviceData_Router_Lights->end();++it)
	{
		DeviceData_Router *pDeviceData_Router = *it;
		if( pDeviceData_Router->m_dwPK_Room==pOH_Orbiter->m_dwPK_Room )
		{
			DCE::CMD_Set_Level CMD_Set_Level(m_dwPK_Device,pDeviceData_Router->m_dwPK_Device,sLevel);
			SendCommand(CMD_Set_Level);
		}
	}
}

void Lighting_Plugin::PreprocessLightingMessage(DeviceData_Router *pDevice,Message *pMessage)
{
	if( !pDevice || !pMessage )
		return;

	if( pMessage->m_dwID==COMMAND_Set_Level_CONST )
	{
		string sLevel = pMessage->m_mapParameters[COMMANDPARAMETER_Level_CONST];
		if( sLevel.size()==0 )
			pMessage->m_dwID=COMMAND_Generic_Off_CONST;
		else
		{
			int iLevel = atoi(sLevel.c_str());
			if( sLevel[0]=='+' )
				iLevel = min(100, GetLightingLevel(pDevice,0)+iLevel);
			else if( sLevel[0]=='-' )
				iLevel = max(0, GetLightingLevel(pDevice,0)+iLevel);

			if( iLevel==0 )
				pMessage->m_dwID=COMMAND_Generic_Off_CONST;
			else
			{
				pDevice->m_sState_set("ON/" + StringUtils::itos(iLevel));
				pMessage->m_mapParameters[COMMANDPARAMETER_Level_CONST] = StringUtils::itos(iLevel);
			}
		}
	}

	if( pMessage->m_dwID==COMMAND_Generic_On_CONST )
		pDevice->m_sState_set("ON/" + StringUtils::itos(GetLightingLevel(pDevice,100)));
	else if( pMessage->m_dwID==COMMAND_Generic_Off_CONST )
		pDevice->m_sState_set("OFF/" + StringUtils::itos(GetLightingLevel(pDevice,0)));
}

int Lighting_Plugin::GetLightingLevel(DeviceData_Router *pDevice,int iLevel_Default)
{
	string sState = pDevice->m_sState_get();
	string::size_type pos = sState.find("/");
	if( pos<sState.size()-1 && pos!=string::npos )
		return atoi(sState.substr(pos+1).c_str());
	else
		return iLevel_Default;
}
