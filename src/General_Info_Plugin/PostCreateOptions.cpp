/*
 PostCreateOptions

 Copyright (C) 2004 Pluto, Inc., a Florida Corporation

 www.plutohome.com

 Phone: +1 (877) 758-8648

 This program is distributed according to the terms of the Pluto Public License, available at:
 http://plutohome.com/index.php?section=public_license

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the Pluto Public License for more details.

 */

#include "PlutoUtils/CommonIncludes.h"
#include "PlutoUtils/DatabaseUtils.h"
#include "PlutoUtils/ProcessUtils.h"
#include "PostCreateOptions.h"
#include "DeviceData_Base.h"
#include "Gen_Devices/AllScreens.h"
#include "DCERouter.h"
#include "Command_Impl.h"
#include "Orbiter_Plugin/Orbiter_Plugin.h"
#include "pluto_main/Database_pluto_main.h"
#include "pluto_main/Table_DeviceTemplate.h"
#include "pluto_main/Define_DeviceCategory.h"
#include "pluto_main/Define_DeviceData.h"
#include "pluto_main/Define_FloorplanObjectType.h"
#include "pluto_main/Table_Device_Device_Related.h"

extern class Command_Impl *g_pCommand_Impl;

PostCreateOptions::PostCreateOptions(Database_pluto_main *pDatabase_pluto_main,Router *pRouter)
{
	m_pDatabase_pluto_main=pDatabase_pluto_main;
	m_pRouter=pRouter;
	m_pOrbiter_Plugin=( Orbiter_Plugin * ) m_pRouter->FindPluginByTemplate(DEVICETEMPLATE_Orbiter_Plugin_CONST);
}

void PostCreateOptions::PostCreateDevice(Row_Device *pRow_Device, OH_Orbiter *pOH_Orbiter)
{
	DeviceCategory *pDeviceCategory = m_pRouter->m_mapDeviceCategory_Find(pRow_Device->FK_DeviceTemplate_getrow()->FK_DeviceCategory_get());
	if( !pDeviceCategory )
	{
		g_pPlutoLogger->Write(LV_CRITICAL,"PostCreateOptions::PostCreateDevice category for Device %d is invalid",pRow_Device->PK_Device_get());
		return;
	}

	if( pDeviceCategory->WithinCategory(DEVICECATEGORY_Storage_Devices_CONST) )
		PostCreateDevice_NetworkStorage(pRow_Device,pOH_Orbiter);
	else if( pDeviceCategory->WithinCategory(DEVICECATEGORY_FileMedia_Server_CONST) )
		PostCreateDevice_FileServer(pRow_Device,pOH_Orbiter);
	else if( pDeviceCategory->WithinCategory(DEVICECATEGORY_Surveillance_Cameras_CONST) )
		PostCreateDevice_Cameras(pRow_Device,pOH_Orbiter);
	else if( pDeviceCategory->WithinCategory(DEVICECATEGORY_Security_Device_CONST) )
		PostCreateSecurityDevice(pRow_Device,pOH_Orbiter);
	else if( pDeviceCategory->WithinCategory(DEVICECATEGORY_Media_Director_CONST) )
		PostCreateDevice_DisklessMD(pRow_Device,pOH_Orbiter);
	else if( pDeviceCategory->WithinCategory(DEVICECATEGORY_Capture_Cards_CONST) )
		PostCreateDevice_CaptureCard(pRow_Device,pOH_Orbiter);
}

void PostCreateOptions::PostCreateDevice_AlarmPanel(Row_Device *pRow_Device, OH_Orbiter *pOH_Orbiter)
{
}

void PostCreateOptions::PostCreateSecurityDevice(Row_Device *pRow_Device, OH_Orbiter *pOH_Orbiter)
{
	int PK_FloorplanObjectType=0;
	Row_Device_DeviceData *pRow_Device_DeviceData = m_pDatabase_pluto_main->Device_DeviceData_get()->GetRow(pRow_Device->PK_Device_get(),DEVICEDATA_PK_FloorplanObjectType_CONST);
	if( pRow_Device_DeviceData )
		PK_FloorplanObjectType = atoi(pRow_Device_DeviceData->IK_DeviceData_get().c_str());

	int iPK_DeviceTemplate=pRow_Device->FK_DeviceTemplate_get();
	string sDefaultSecuritySetting;
	if( PK_FloorplanObjectType==FLOORPLANOBJECTTYPE_SECURITY_DOOR_CONST || iPK_DeviceTemplate==DEVICETEMPLATE_Door_Sensor_CONST )
		sDefaultSecuritySetting = "1,0,N1,1,1,2,N1"; // Monitor mode, security alert when armed, announcement when entertaining
	else if( PK_FloorplanObjectType==FLOORPLANOBJECTTYPE_SECURITY_WINDOW_CONST || iPK_DeviceTemplate==DEVICETEMPLATE_Window_Sensor_CONST )
		sDefaultSecuritySetting = "0,0,N1,2,2,0,N1"; // Make an announcement when the user is at home, security breach when away
	else if( iPK_DeviceTemplate==DEVICETEMPLATE_Glass_Break_Sensor_CONST )
		sDefaultSecuritySetting = "0,2,N1,N1,N1,2,N1"; // Always announcement or trigger an alarm
	else if( PK_FloorplanObjectType==FLOORPLANOBJECTTYPE_SECURITY_MOTION_DETECTOR_CONST || iPK_DeviceTemplate==DEVICETEMPLATE_Motion_Detector_CONST )
		sDefaultSecuritySetting = "0,0,N1,0,0,0,N1"; // Only active when away
	else if( PK_FloorplanObjectType==FLOORPLANOBJECTTYPE_SECURITY_SMOKE_CONST || iPK_DeviceTemplate==DEVICETEMPLATE_Smoke_Detector_CONST )
		sDefaultSecuritySetting = "1,N1,N1,N1,N1,N1,N1"; // Always an alarm
	else if( PK_FloorplanObjectType==FLOORPLANOBJECTTYPE_SECURITY_AIRQUALITY_CONST || iPK_DeviceTemplate==DEVICETEMPLATE_Air_Quality_Sensor_CONST )
		sDefaultSecuritySetting = "1,N1,N1,N1,N1,N1,N1"; // Always an alarm

	DatabaseUtils::SetDeviceData(m_pDatabase_pluto_main,pRow_Device->PK_Device_get(),DEVICEDATA_Alert_CONST,sDefaultSecuritySetting);
}

void PostCreateOptions::PostCreateDevice_FileServer(Row_Device *pRow_Device, OH_Orbiter *pOH_Orbiter)
{
#ifdef DEBUG
	g_pPlutoLogger->Write(LV_STATUS,"PostCreateOptions::PostCreateDevice_NetworkStorage device  %d template %d",
		pRow_Device->PK_Device_get(),pRow_Device->FK_DeviceTemplate_get());
#endif
	string sName = DatabaseUtils::GetDeviceData(m_pDatabase_pluto_main,pRow_Device->PK_Device_get(),DEVICEDATA_Description_CONST);
	if( sName.empty()==false )
	{
		pRow_Device->Description_set(pRow_Device->Description_get() + ": " + sName);
		m_pDatabase_pluto_main->Device_get()->Commit();
	}
}

void PostCreateOptions::PostCreateDevice_NetworkStorage(Row_Device *pRow_Device, OH_Orbiter *pOH_Orbiter)
{
#ifdef DEBUG
	g_pPlutoLogger->Write(LV_STATUS,"PostCreateOptions::PostCreateDevice_NetworkStorage device  %d template %d",
		pRow_Device->PK_Device_get(),pRow_Device->FK_DeviceTemplate_get());
#endif

	Command_Impl *pCommand_Impl_GIP = m_pRouter->FindPluginByTemplate(DEVICETEMPLATE_General_Info_Plugin_CONST);
	if( pCommand_Impl_GIP ) // Should always be there
	{
		DCE::CMD_Check_Mounts CMD_Check_Mounts(g_pCommand_Impl->m_dwPK_Device,pCommand_Impl_GIP->m_dwPK_Device);
		g_pCommand_Impl->SendCommand(CMD_Check_Mounts);
	}

	string sName = DatabaseUtils::GetDeviceData(m_pDatabase_pluto_main,pRow_Device->PK_Device_get(),DEVICEDATA_Share_Name_CONST);
	if( sName.empty()==false )
	{
		pRow_Device->Description_set(pRow_Device->Description_get() + ": " + sName);
		m_pDatabase_pluto_main->Device_get()->Commit();
	}
}

void PostCreateOptions::PostCreateDevice_Cameras(Row_Device *pRow_Device, OH_Orbiter *pOH_Orbiter)
{
#ifdef DEBUG
	g_pPlutoLogger->Write(LV_STATUS,"PostCreateOptions::PostCreateDevice_Cameras device  %d template %d",
		pRow_Device->PK_Device_get(),pRow_Device->FK_DeviceTemplate_get());
#endif
}

void PostCreateOptions::PostCreateDevice_DisklessMD(Row_Device *pRow_Device, OH_Orbiter *pOH_Orbiter)
{
#ifdef DEBUG
	g_pPlutoLogger->Write(LV_STATUS,"PostCreateOptions::PostCreateDevice_DisklessMD device  %d template %d",
		pRow_Device->PK_Device_get(),pRow_Device->FK_DeviceTemplate_get());
#endif
	string sPK_Device = StringUtils::itos(pRow_Device->PK_Device_get());
	char * args[] = { "/usr/pluto/bin/New_PnP_MD.sh", (char *)(pRow_Device->IPaddress_get().c_str()), (char *)(pRow_Device->MACaddress_get().c_str()),
		(char *)(sPK_Device.c_str()), NULL };
	ProcessUtils::SpawnDaemon(args[0], args);
}

void PostCreateOptions::PostCreateDevice_CaptureCard(Row_Device *pRow_Device, OH_Orbiter *pOH_Orbiter)
{
	int PK_Device_TopMost = DatabaseUtils::GetTopMostDevice(m_pDatabase_pluto_main,pRow_Device->PK_Device_get());
	DeviceData_Router *pDevice_PC = m_pRouter->m_mapDeviceData_Router_Find(PK_Device_TopMost);
	DeviceData_Router *pDevice_AppServer = pDevice_PC ? (DeviceData_Router *) pDevice_PC->FindFirstRelatedDeviceOfCategory(DEVICECATEGORY_App_Server_CONST) : NULL;

	if( !pDevice_AppServer )
	{
		g_pPlutoLogger->Write(LV_CRITICAL,"PostCreateOptions::PostCreateDevice_CaptureCard - no app server for %d",pRow_Device->PK_Device_get());
		return;
	}
#ifdef DEBUG
	g_pPlutoLogger->Write(LV_STATUS,"PostCreateOptions::PostCreateDevice_CaptureCard device  %d template %d top %d %p",
		pRow_Device->PK_Device_get(),pRow_Device->FK_DeviceTemplate_get(),PK_Device_TopMost,pDevice_AppServer);
#endif

	DCE::CMD_Spawn_Application CMD_Spawn_Application(g_pCommand_Impl->m_dwPK_Device,pDevice_AppServer->m_dwPK_Device,"/usr/pluto/bin/CaptureCards_Setup.sh","captcard",
		"","","",false,false,false,true);
	g_pCommand_Impl->SendCommand(CMD_Spawn_Application);
}
