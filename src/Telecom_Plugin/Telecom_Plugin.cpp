/*
 Telecom_Plugin

 Copyright (C) 2004 Pluto, Inc., a Florida Corporation

 www.plutohome.com

 Phone: +1 (877) 758-8648

 This program is distributed according to the terms of the Pluto Public License, available at:
 http://plutohome.com/index.php?section=public_license

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the Pluto Public License for more details.

 */

//<-dceag-d-b->
#include "Telecom_Plugin.h"
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

#include "Gen_Devices/AllScreens.h"

#include "pluto_main/Database_pluto_main.h"
#include "pluto_telecom/Database_pluto_telecom.h"
#include "pluto_main/Table_Device.h"
#include "pluto_main/Table_Device_DeviceData.h"
#include "pluto_main/Table_CommandGroup.h"
#include "pluto_main/Define_Array.h"
#include "pluto_main/Define_Screen.h"
#include "pluto_main/Define_DeviceTemplate.h"
#include "pluto_main/Define_DataGrid.h"
#include "pluto_main/Define_DeviceData.h"
#include "pluto_main/Define_Command.h"
#include "pluto_main/Define_CommandParameter.h"
#include "pluto_main/Define_Event.h"
#include "pluto_main/Define_EventParameter.h"
#include "pluto_main/Define_MediaType.h"
#include "pluto_main/Table_Users.h"
#include "pluto_telecom/Table_Contact.h"
#include "pluto_telecom/Table_PhoneNumber.h"
#include "pluto_telecom/Table_PhoneType.h"
#include "callmanager.h"
#include "Orbiter_Plugin/Orbiter_Plugin.h"

#include "DCE/DCEConfig.h"
#include "PlutoUtils/MySQLHelper.h"

#include "SerializeClass/ShapesColors.h"
#define MAX_TELECOM_COLORS 5
int UniqueColors[MAX_TELECOM_COLORS];
#include <time.h>

#ifndef WIN32
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#endif

#define CONFERENCE_PREFIX "C000"
void * startDisplayThread(void * Arg);

//<-dceag-const-b->
// The primary constructor when the class is created as a stand-alone device
Telecom_Plugin::Telecom_Plugin(int DeviceID, string ServerAddress,bool bConnectEventHandler,bool bLocalMode,class Router *pRouter)
	: Telecom_Plugin_Command(DeviceID, ServerAddress,bConnectEventHandler,bLocalMode,pRouter)
//<-dceag-const-e->
{
	m_pDatabase_pluto_main = NULL;
	m_pDatabase_pluto_telecom = NULL;
	pthread_mutex_init(&mtx_err_messages,0);
	iCmdCounter = 0;
	next_conf_room = 1;
}

//<-dceag-getconfig-b->
bool Telecom_Plugin::GetConfig()
{
	if( !Telecom_Plugin_Command::GetConfig() )
		return false;
//<-dceag-getconfig-e->

	m_pDatabase_pluto_main = new Database_pluto_main();
	if(!m_pDatabase_pluto_main->Connect(m_pRouter->sDBHost_get(),m_pRouter->sDBUser_get(),m_pRouter->sDBPassword_get(),m_pRouter->sDBName_get(),m_pRouter->iDBPort_get()) )
	{
		g_pPlutoLogger->Write(LV_CRITICAL, "Cannot connect to database!");
		m_bQuit=true;
		return false;
	}

	m_pDatabase_pluto_telecom = new Database_pluto_telecom();
	if(!m_pDatabase_pluto_telecom->Connect(m_pRouter->sDBHost_get(),m_pRouter->sDBUser_get(),m_pRouter->sDBPassword_get(),"pluto_telecom",m_pRouter->iDBPort_get()) )
	{
		g_pPlutoLogger->Write(LV_CRITICAL, "Cannot connect to telecom database!");
		m_bQuit=true;
		return false;
	}
	vector<class Row_Device*> vectDevices;
	m_pDatabase_pluto_main->Device_get()->GetRows(DEVICE_FK_DEVICETEMPLATE_FIELD + string("=") + StringUtils::itos(DEVICETEMPLATE_Orbiter_Embedded_Phone_CONST), &vectDevices);
	for(size_t s=0;s<vectDevices.size();s++)
	{
		map_orbiter2embedphone[vectDevices[s]->FK_Device_ControlledVia_get()] = vectDevices[s]->PK_Device_get();
		map_embedphone2orbiter[vectDevices[s]->PK_Device_get()] = vectDevices[s]->FK_Device_ControlledVia_get();
	}
	vector<class Row_Device_DeviceData*> vectDeviceData;
	m_pDatabase_pluto_main->Device_DeviceData_get()->GetRows(DEVICE_DEVICEDATA_FK_DEVICEDATA_FIELD+string("=") + StringUtils::itos(DEVICEDATA_PhoneNumber_CONST), &vectDeviceData);
	for(size_t s=0;s<vectDeviceData.size();s++)
	{
		map_ext2device[atoi(vectDeviceData[s]->IK_DeviceData_get().c_str())] = vectDeviceData[s]->FK_Device_get();
		map_device2ext[vectDeviceData[s]->FK_Device_get()] = atoi(vectDeviceData[s]->IK_DeviceData_get().c_str());
	}
	UniqueColors[0] = PlutoColor(128,0,0).m_Value;
	UniqueColors[1] = PlutoColor(0,128,0).m_Value;
	UniqueColors[2] = PlutoColor(0,0,128).m_Value;
	UniqueColors[3] = PlutoColor(0,128,128).m_Value;
	UniqueColors[4] = PlutoColor(128,128,0).m_Value;
	return true;
}

//<-dceag-const2-b->!

//<-dceag-dest-b->
Telecom_Plugin::~Telecom_Plugin()
//<-dceag-dest-e->
{
	delete m_pDatabase_pluto_main;

}

//<-dceag-reg-b->
// This function will only be used if this device is loaded into the DCE Router's memory space as a plug-in.  Otherwise Connect() will be called from the main()
bool Telecom_Plugin::Register()
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
		new DataGridGeneratorCallBack(this,(DCEDataGridGeneratorFn)(&Telecom_Plugin::TelecomScenariosGrid))
		,DATAGRID_Telecom_Scenarios_CONST,PK_DeviceTemplate_get());

	m_pDatagrid_Plugin->RegisterDatagridGenerator(
		new DataGridGeneratorCallBack(this,(DCEDataGridGeneratorFn)(&Telecom_Plugin::PhoneBookAutoCompl))
		,DATAGRID_Phone_Book_Auto_Compl_CONST,PK_DeviceTemplate_get());

	m_pDatagrid_Plugin->RegisterDatagridGenerator(
		new DataGridGeneratorCallBack(this,(DCEDataGridGeneratorFn)(&Telecom_Plugin::PhoneBookListOfNos))
		,DATAGRID_Phone_Book_List_of_Nos_CONST,PK_DeviceTemplate_get());

	m_pDatagrid_Plugin->RegisterDatagridGenerator(
		new DataGridGeneratorCallBack(this,(DCEDataGridGeneratorFn)(&Telecom_Plugin::ActiveCallsGrid))
		,DATAGRID_Floorplan_Telecom_Streams_CONST,PK_DeviceTemplate_get());

	m_pDatagrid_Plugin->RegisterDatagridGenerator(
		new DataGridGeneratorCallBack(this,(DCEDataGridGeneratorFn)(&Telecom_Plugin::RecentCallsGrid)),
		DATAGRID_Recent_Calls_CONST,PK_DeviceTemplate_get());

	m_pDatagrid_Plugin->RegisterDatagridGenerator(
		new DataGridGeneratorCallBack(this,(DCEDataGridGeneratorFn)(&Telecom_Plugin::SpeedDialGrid)),
		DATAGRID_Speed_Dial_CONST,PK_DeviceTemplate_get());

	m_pDatagrid_Plugin->RegisterDatagridGenerator(
		new DataGridGeneratorCallBack(this,(DCEDataGridGeneratorFn)(&Telecom_Plugin::UserVoiceMailGrid)),
		DATAGRID_User_VoiceMail_CONST,PK_DeviceTemplate_get());

	RegisterMsgInterceptor( ( MessageInterceptorFn )( &Telecom_Plugin::CommandResult ), 0, 0, 0, 0, MESSAGETYPE_EVENT, EVENT_PBX_CommandResult_CONST );
	RegisterMsgInterceptor( ( MessageInterceptorFn )( &Telecom_Plugin::Ring ), 0, 0, 0, 0, MESSAGETYPE_EVENT, EVENT_PBX_Ring_CONST );
	RegisterMsgInterceptor( ( MessageInterceptorFn )( &Telecom_Plugin::IncomingCall ), 0, 0, 0, 0, MESSAGETYPE_EVENT, EVENT_Incoming_Call_CONST );
    RegisterMsgInterceptor( ( MessageInterceptorFn )(&Telecom_Plugin::OrbiterRegistered) ,0,0,0,0,MESSAGETYPE_COMMAND,COMMAND_Orbiter_Registered_CONST);
    RegisterMsgInterceptor( ( MessageInterceptorFn )(&Telecom_Plugin::Hangup) ,0,0,0,0,MESSAGETYPE_EVENT,EVENT_PBX_Hangup_CONST);
	RegisterMsgInterceptor( ( MessageInterceptorFn )(&Telecom_Plugin::VoIP_Problem) ,0,0,0,0,MESSAGETYPE_EVENT,EVENT_VoIP_Problem_Detected_CONST);
	RegisterMsgInterceptor( ( MessageInterceptorFn )(&Telecom_Plugin::VoiceMailChanged) ,0,0,0,0,MESSAGETYPE_EVENT,EVENT_Voice_Mail_Changed_CONST);
	
    if (pthread_create(&displayThread, NULL, startDisplayThread, (void *) this))
    {
        g_pPlutoLogger->Write(LV_CRITICAL, "Failed to create Display Thread");
        m_bQuit = 1;
        exit(1);
    }
    pthread_detach(displayThread);
	
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
void Telecom_Plugin::ReceivedCommandForChild(DeviceData_Impl *pDeviceData_Impl,string &sCMD_Result,Message *pMessage)
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
void Telecom_Plugin::ReceivedUnknownCommand(string &sCMD_Result,Message *pMessage)
//<-dceag-cmduk-e->
{
	sCMD_Result = "UNKNOWN DEVICE";
}

class DataGridTable *Telecom_Plugin::TelecomScenariosGrid(string GridID,string Parms,void *ExtraData,int *iPK_Variable,string *sValue_To_Assign,class Message *pMessage)
{
	DataGridTable *pDataGrid = new DataGridTable();
	DataGridCell *pCell;

	vector<Row_CommandGroup *> vectRowCommandGroup;
	m_pDatabase_pluto_main->CommandGroup_get()->GetRows( COMMANDGROUP_FK_ARRAY_FIELD + string("=") + StringUtils::itos(ARRAY_Communication_Scenarios_CONST) + " AND "
			+ COMMANDGROUP_FK_INSTALLATION_FIELD + "=" + StringUtils::itos(m_pRouter->iPK_Installation_get()),&vectRowCommandGroup );
	for(size_t s=0;s<vectRowCommandGroup.size();++s)
	{
		Row_CommandGroup *pRow_CommandGroup = vectRowCommandGroup[s];
		pCell = new DataGridCell(pRow_CommandGroup->Description_get(),StringUtils::itos(pRow_CommandGroup->PK_CommandGroup_get()));
		pDataGrid->SetData(0,(int) s,pCell);
	}

	return pDataGrid;
}

class DataGridTable *Telecom_Plugin::PhoneBookAutoCompl(string GridID,string Parms,void *ExtraData,int *iPK_Variable,string *sValue_To_Assign,class Message *pMessage)
{
	g_pPlutoLogger->Write(LV_STATUS, "Phone Book Autocomplete request received for GridID: %s with Params: %s.",
																					GridID.c_str(), Parms.c_str());

	DataGridTable *pDataGrid = new DataGridTable();
	DataGridCell *pCell;

	// get uid and filter text
	string::size_type pos=0;
	string sPK_Users = StringUtils::Tokenize(Parms,"|",pos);
	string sFind = StringUtils::Tokenize(Parms,"|",pos);
	if( sFind.size()==0 )
		return pDataGrid;

	string sWhere = "(Name like '" + StringUtils::SQLEscape(sFind) + "%' OR Company like '" + StringUtils::SQLEscape(sFind) + "%') "
		"AND (EK_Users IS NULL OR EK_Users=" + sPK_Users + ")";
	vector<Row_Contact *> vectRow_Contact;
	m_pDatabase_pluto_telecom->Contact_get()->GetRows(sWhere,&vectRow_Contact);
	size_t FirstBatch;  // Counter used to remember how many rows we added during the first sql query
	string sPK;  // PK's we got during the first scan so we can exclude them on the second
	for(FirstBatch=0;FirstBatch<vectRow_Contact.size();++FirstBatch)
	{
		Row_Contact *pRow_Contact = vectRow_Contact[FirstBatch];
		string sDescription;
		if( pRow_Contact->Name_get().size() )
		{
			sDescription += pRow_Contact->Name_get();
			if( pRow_Contact->Title_get().size() )
				sDescription += "," + pRow_Contact->Title_get();
			sDescription += "\n";
		}
		if( pRow_Contact->Company_get().size() )
			sDescription += pRow_Contact->Company_get() + "\n";

		if( sPK.size() )
			sPK += "," + StringUtils::itos(pRow_Contact->PK_Contact_get());
		else
			sPK = StringUtils::itos(pRow_Contact->PK_Contact_get());

		pCell = new DataGridCell(sDescription,StringUtils::itos(pRow_Contact->PK_Contact_get()) );
		pDataGrid->SetData(0, FirstBatch, pCell);
	}

	sWhere = (sPK.size() ? "PK_Contact NOT IN("+sPK+") AND " : "") +
		"(Name like '% " + StringUtils::SQLEscape(sFind) + "%' OR Company like '% " + StringUtils::SQLEscape(sFind) + "%') "
		"AND (EK_Users IS NULL OR EK_Users=" + sPK_Users + ")";
	vectRow_Contact.clear();
	m_pDatabase_pluto_telecom->Contact_get()->GetRows(sWhere,&vectRow_Contact);
	for(size_t s=0;s<vectRow_Contact.size();++s)
	{
		Row_Contact *pRow_Contact = vectRow_Contact[s];
		string sDescription;
		if( pRow_Contact->Name_get().size() )
		{
			sDescription += pRow_Contact->Name_get();
			if( pRow_Contact->Title_get().size() )
				sDescription += ", " + pRow_Contact->Title_get();
			sDescription += "\n";
		}
		if( pRow_Contact->Company_get().size() )
			sDescription += pRow_Contact->Company_get() + "\n";

		pCell = new DataGridCell(sDescription, StringUtils::itos(pRow_Contact->PK_Contact_get()));
		pDataGrid->SetData(0, FirstBatch + s, pCell);
	}

	return pDataGrid;
}

class DataGridTable *Telecom_Plugin::PhoneBookListOfNos(string GridID,string Parms,void *ExtraData,int *iPK_Variable,string *sValue_To_Assign,class Message *pMessage)
{
	g_pPlutoLogger->Write(LV_STATUS, "Phone Book List of NOS request received for GridID: %s with Params: %s.",
																					GridID.c_str(), Parms.c_str());

	DataGridTable *pDataGrid = new DataGridTable();
	DataGridCell *pCell;

	if( Parms.size()==0 )
		return pDataGrid;
	Row_Contact *pRow_Contact = m_pDatabase_pluto_telecom->Contact_get()->GetRow( atoi(Parms.c_str()) );
	if( !pRow_Contact )
		return pDataGrid; // Shouldn't happen

	vector<Row_PhoneNumber *> vectRow_PhoneNumber;
	pRow_Contact->PhoneNumber_FK_Contact_getrows(&vectRow_PhoneNumber);
	int Row=0;
	for(size_t s=0;s<vectRow_PhoneNumber.size();++s)
	{
		Row_PhoneNumber *pRow_PhoneNumber = vectRow_PhoneNumber[s];
		Row_PhoneType *pRow_PhoneType = pRow_PhoneNumber->FK_PhoneType_getrow();
		string sPhoneType,sDescription,sDial=GetDialNumber(pRow_PhoneNumber);

		if( pRow_PhoneType )
			sPhoneType = pRow_PhoneType->Description_get();

		if( pRow_PhoneNumber->CountryCode_get().size() )
			sDescription += "+" + pRow_PhoneNumber->CountryCode_get() + " ";
		if( pRow_PhoneNumber->AreaCode_get().size() )
			sDescription += "(" + pRow_PhoneNumber->AreaCode_get() + ") ";
		if( pRow_PhoneNumber->PhoneNumber_get().size() )
			sDescription += pRow_PhoneNumber->PhoneNumber_get();

		if( sDescription.size()==0 )
			sDescription = pRow_PhoneNumber->DialAs_get();

		if( pRow_PhoneNumber->Extension_get().size() )
			sDescription += "x." + pRow_PhoneNumber->Extension_get();

		if( sDescription.size()==0 )
			continue;

		pCell = new DataGridCell(pRow_PhoneType->Description_get(), sDial);
		pDataGrid->SetData(0,Row, pCell);

		pCell = new DataGridCell(sDescription, sDial);
		pCell->m_Colspan=2;
		pDataGrid->SetData(1,Row, pCell);

		Row++;
	}

	return pDataGrid;
}


bool
Telecom_Plugin::CommandResult( class Socket *pSocket, class Message *pMessage,
		                                class DeviceData_Base *pDeviceFrom, class DeviceData_Base *pDeviceTo ) {

	g_pPlutoLogger->Write(LV_STATUS, "Command Result received from PBX.");
	int iCommandID = atoi(pMessage->m_mapParameters[EVENTPARAMETER_CommandID_CONST].c_str());
	int iResult = atoi(pMessage->m_mapParameters[EVENTPARAMETER_Result_CONST].c_str());
	string sMessage = pMessage->m_mapParameters[EVENTPARAMETER_Message_CONST];

	ProcessResult(iCommandID, iResult, sMessage);
	return true;
}

void
Telecom_Plugin::ProcessResult(int iCommandID, int iResult, std::string sMessage) {
	CallManager *pCallManager = CallManager::getInstance();
	CallData *pCallData	= pCallManager->findCallByPendingCmdID(iCommandID);
	if(!pCallData) {
		g_pPlutoLogger->Write(LV_CRITICAL, "Obsolete command reply received.");
		return;
	}

	DeviceData_Router* pDeviceData = find_Device(pCallData->getOwnerDevID());
	if(!pDeviceData) {
		g_pPlutoLogger->Write(LV_CRITICAL, "Device %d is no more Available. Destroying call.", pCallData->getOwnerDevID());
		pCallManager->removeCall(pCallData);
		return;
	}

	switch(pCallData->getState()) {
		case CallData::STATE_ORIGINATING:
			if(!iResult) {
				pCallData->setState(CallData::STATE_OPENED);
				pCallData->setID(sMessage);

				/*switch orbiter screen*/
				if(pDeviceData->m_dwPK_DeviceTemplate == DEVICETEMPLATE_Orbiter_CONST) {
					CMD_Goto_DesignObj cmd_CMD_Goto_DesignObj(m_dwPK_Device, pDeviceData->m_dwPK_Device,
										m_dwPK_Device, "1281", pCallData->getID(), "", false, false);
					SendCommand(cmd_CMD_Goto_DesignObj);
				}
			} else {
				/*switch orbiter screen*/
				pCallManager->removeCall(pCallData);

			}
			break;
		case CallData::STATE_TRANSFERING:
			if(!iResult) {
			} else {
			}
			pCallData->setState(CallData::STATE_OPENED);
			break;
	}
}

bool
Telecom_Plugin::Ring( class Socket *pSocket, class Message *pMessage,
					 			class DeviceData_Base *pDeviceFrom, class DeviceData_Base *pDeviceTo ) {
	g_pPlutoLogger->Write(LV_STATUS, "Ring event received from PBX.");

	string sPhoneExtension = pMessage->m_mapParameters[EVENTPARAMETER_PhoneExtension_CONST];
	string sPhoneCallID = pMessage->m_mapParameters[EVENTPARAMETER_PhoneCallID_CONST];
	string sPhoneCallerID = pMessage->m_mapParameters[EVENTPARAMETER_PhoneCallerID_CONST];

	ProcessRing(sPhoneExtension, sPhoneCallerID, sPhoneCallID);
	return true;
}

void
Telecom_Plugin::ProcessRing(std::string sPhoneExtension, std::string sPhoneCallerID, std::string sPhoneCallID)
{
	int phoneID=map_ext2device[atoi(sPhoneExtension.c_str())];
	if(phoneID>0)
	{
		CallData *pCallData = CallManager::getInstance()->findCallByOwnerDevID(phoneID);
		if(!pCallData) {
			/*create new call data*/
			pCallData = new CallData();
			pCallData->setOwnerDevID(phoneID);
			pCallData->setID(sPhoneCallID);
			pCallData->setCallerID(sPhoneCallerID);
			CallManager::getInstance()->addCall(pCallData);

			g_pPlutoLogger->Write(LV_STATUS, "Creating calldata for ringing device %d (ext %s, id %s)",phoneID,sPhoneExtension.c_str(),sPhoneCallID.c_str());
			pCallData->setState(CallData::STATE_NOTDEFINED);
		}
		else
		{
			g_pPlutoLogger->Write(LV_WARNING, "Will modify calldata for device %d in state %d with channelid %s",phoneID,pCallData->getState(),sPhoneCallID.c_str());
			pCallData->setID(sPhoneCallID);
			pCallData->setState(CallData::STATE_NOTDEFINED);
		}
	}
	int iext, pos, oldpos=0;				
	string onechan, sext;
	do
	{
		pos = sPhoneCallID.find(' ',oldpos);
		if(pos <0)
		{
			onechan = sPhoneCallID.substr(oldpos, sPhoneCallID.length());
		}
		else
		{
			onechan= sPhoneCallID.substr(oldpos, pos - oldpos);
		}
		if(!ParseChannel(onechan,&iext,&sext))
		{
			phoneID=map_ext2device[iext];
			if(phoneID>0)
			{
				CallData *pCallData = CallManager::getInstance()->findCallByOwnerDevID(phoneID);
				if(!pCallData) {
					/*create new call data*/
					pCallData = new CallData();
					pCallData->setOwnerDevID(phoneID);
					pCallData->setID(sPhoneCallID);
					pCallData->setCallerID(sPhoneCallerID);
					CallManager::getInstance()->addCall(pCallData);

					g_pPlutoLogger->Write(LV_STATUS, "Creating calldata for ringing device %d (ext %s, id %s)",phoneID,sext.c_str(),sPhoneCallID.c_str());
					pCallData->setState(CallData::STATE_NOTDEFINED);
				}
				else
				{
					g_pPlutoLogger->Write(LV_WARNING, "Will modify calldata for device %d in state %d with channelid %s",phoneID,pCallData->getState(),sPhoneCallID.c_str());
					pCallData->setID(sPhoneCallID);
					pCallData->setState(CallData::STATE_NOTDEFINED);
				}
			}
		}
		oldpos = pos+1;
	}
	while(pos>=0);
	
}


DeviceData_Router*
Telecom_Plugin::find_Device(int iPK_Device) {
    /*search device by id*/
	return m_pRouter->m_mapDeviceData_Router_Find(iPK_Device);
}

DeviceData_Router*
Telecom_Plugin::find_AsteriskDevice() {
	DeviceData_Router *pDeviceData = NULL;

	ListDeviceData_Router* pListDeviceData =
								m_pRouter->m_mapDeviceByTemplate_Find(DEVICETEMPLATE_Asterisk_CONST);
	if(pListDeviceData) {
		if(pListDeviceData->size() > 1) {
			string errStr = "More then 1 Asterisk Handlers found:";
			list<DeviceData_Router*>::iterator it = pListDeviceData->begin();
			while(it != pListDeviceData->end()) {
				errStr += ' ' + StringUtils::ltos((*it)->m_dwPK_Device);
				it++;
			}
			g_pPlutoLogger->Write(LV_CRITICAL, errStr.c_str());
			//return -1;
		}

		pDeviceData = pListDeviceData->front();
	} else {
		g_pPlutoLogger->Write(LV_STATUS, "Asterisk Handler not found...");
	}
	return pDeviceData;
}

int
Telecom_Plugin::generate_NewCommandID() {
	return ++iCmdCounter;
}


//<-dceag-sample-b->!

/*

	COMMANDS TO IMPLEMENT

*/


//<-dceag-c232-b->

	/** @brief COMMAND: #232 - PL_Originate */
	/** Originate a call */
		/** @param #2 PK_Device */
			/** Device (phone) from which to place the call */
		/** @param #83 PhoneExtension */
			/** Phone extension to dial */
		/** @param #84 PhoneCallerID */
			/** Caller ID */

void Telecom_Plugin::CMD_PL_Originate(int iPK_Device,string sPhoneExtension,string sPhoneCallerID,string &sCMD_Result,Message *pMessage)
//<-dceag-c232-e->
{
	g_pPlutoLogger->Write(LV_STATUS, "Originate command called with params: DeviceID=%d, PhoneExtension=%s!", iPK_Device, sPhoneExtension.c_str());

	if(sPhoneExtension.empty()) {
		g_pPlutoLogger->Write(LV_CRITICAL, "Error validating input parameters");
		return;
	}

    /*search device by id*/
    DeviceData_Router *pDeviceData = find_Device(iPK_Device);
 	if(!pDeviceData) {
		g_pPlutoLogger->Write(LV_CRITICAL, "No device found with id: %d", iPK_Device);
		return;
	}
	if(pDeviceData->m_dwPK_DeviceTemplate == DEVICETEMPLATE_Orbiter_CONST)
	{
    	pDeviceData = find_Device(map_orbiter2embedphone[iPK_Device]);
 		if(!pDeviceData) {
			g_pPlutoLogger->Write(LV_CRITICAL, "No device found with id: %d", iPK_Device);
			return;
		}
		iPK_Device = map_orbiter2embedphone[iPK_Device];
	}
	if(pDeviceData->m_dwPK_DeviceTemplate == DEVICETEMPLATE_Orbiter_Embedded_Phone_CONST)
	{
		g_pPlutoLogger->Write(LV_STATUS, "Will originate using CMD_Phone_Initiate device#%d dial %s",iPK_Device,sPhoneExtension.c_str());
		DCE::CMD_Phone_Initiate cmd(m_dwPK_Device,iPK_Device,sPhoneExtension);
		SendCommand(cmd);
		CallData *pCallData = CallManager::getInstance()->findCallByOwnerDevID(iPK_Device);
		if(!pCallData) {
			/*create new call data*/
			pCallData = new CallData();
			pCallData->setOwnerDevID(iPK_Device);
			CallManager::getInstance()->addCall(pCallData);
		}
		pCallData->setState(CallData::STATE_ORIGINATING);
		return;
	}
	if(sPhoneCallerID == "")
	{
		sPhoneCallerID="pluto";
	}
	/*find phonetype and phonenumber*/
	string sSrcPhoneType = pDeviceData->mapParameters_Find(DEVICEDATA_PhoneType_CONST);
	string sSrcPhoneNumber = pDeviceData->mapParameters_Find(DEVICEDATA_PhoneNumber_CONST);

	g_pPlutoLogger->Write(LV_STATUS, "Using source phone with parameters: PhoneType=%s, PhoneNumber=%s!",
													sSrcPhoneType.c_str(), sSrcPhoneNumber.c_str());

	/*find PBX*/
	DeviceData_Router* pPBXDevice = find_AsteriskDevice();
	if(pPBXDevice) {

		CallData *pCallData = CallManager::getInstance()->findCallByOwnerDevID(pMessage->m_dwPK_Device_From);
		if(!pCallData) {
			/*create new call data*/
			pCallData = new CallData();
			pCallData->setOwnerDevID(pMessage->m_dwPK_Device_From);
			CallManager::getInstance()->addCall(pCallData);
		}

		pCallData->setState(CallData::STATE_ORIGINATING);

		/*send originate command to PBX*/
		pCallData->setPendingCmdID(generate_NewCommandID());
		CMD_PBX_Originate cmd_PBX_Originate(m_dwPK_Device, pPBXDevice->m_dwPK_Device,
					                        sSrcPhoneNumber, sSrcPhoneType, sPhoneExtension, sPhoneCallerID, pCallData->getPendingCmdID());
	    SendCommand(cmd_PBX_Originate);
	}
}




//<-dceag-c234-b->

	/** @brief COMMAND: #234 - PL_Transfer */
	/** Transfers a call to other phone */
		/** @param #2 PK_Device */
			/** Device ID to transfer call to */
		/** @param #17 PK_Users */
			/** User ID to transfer call to */
		/** @param #83 PhoneExtension */
			/** Local Extension to transfer call to */
		/** @param #196 IsConference */
			/** Transfer the call to a conference room? */

void Telecom_Plugin::CMD_PL_Transfer(int iPK_Device,int iPK_Users,string sPhoneExtension,bool bIsConference,string &sCMD_Result,Message *pMessage)
//<-dceag-c234-e->
{
	g_pPlutoLogger->Write(LV_STATUS, "Transfer command called with params: DeviceID=%d UserID=%d Extension=%s", iPK_Device,iPK_Users,sPhoneExtension.c_str());
	string sPhoneNumber;
	if(iPK_Device != 0)
	{
		/*search device by id*/
		DeviceData_Router *pDeviceData = find_Device(iPK_Device);
		if(!pDeviceData) {
			g_pPlutoLogger->Write(LV_CRITICAL, "No device found with id: %d", iPK_Device);
			return;
	    }
		if(pDeviceData->m_dwPK_DeviceTemplate == DEVICETEMPLATE_Orbiter_CONST)
		{
	    	pDeviceData = find_Device(map_orbiter2embedphone[iPK_Device]);
 			if(!pDeviceData) {
				g_pPlutoLogger->Write(LV_CRITICAL, "No device found with id: %d", iPK_Device);
				return;
			}
		}
		sPhoneNumber = pDeviceData->mapParameters_Find(DEVICEDATA_PhoneNumber_CONST);
	}
	if(iPK_Users != 0)
	{
		/*search user by id*/

		class Row_Users* rowuser;
		rowuser=m_pDatabase_pluto_main->Users_get()->GetRow(iPK_Users);
		if(rowuser)
		{
			sPhoneNumber =  StringUtils::itos(rowuser->Extension_get());
		}
		else
		{
			g_pPlutoLogger->Write(LV_CRITICAL, "No user found with id: %d", iPK_Users);
			return;
		}
	}
	if(sPhoneExtension != "")
	{
		sPhoneNumber=sPhoneExtension;
	}
	if(sPhoneNumber == "")
	{
		g_pPlutoLogger->Write(LV_CRITICAL, "Nowhere to transfer !!!");
		return;
	}

	CallData *pCallData = CallManager::getInstance()->findCallByOwnerDevID(pMessage->m_dwPK_Device_From);
	if(!pCallData) {
		g_pPlutoLogger->Write(LV_WARNING, "No calldata found for device %d",pMessage->m_dwPK_Device_From);

		pCallData = CallManager::getInstance()->findCallByOwnerDevID(map_orbiter2embedphone[pMessage->m_dwPK_Device_From]);
		if(!pCallData) {
			g_pPlutoLogger->Write(LV_WARNING, "No calldata found for device %d",map_orbiter2embedphone[pMessage->m_dwPK_Device_From]);
			return;
		}
	}
	/*find PBX*/
	DeviceData_Router* pPBXDevice = find_AsteriskDevice();
	if(pPBXDevice) {
		pCallData->setState(CallData::STATE_TRANSFERING);
		pCallData->setPendingCmdID(generate_NewCommandID());
		if(pCallData->getID().find(CONFERENCE_PREFIX)==0)
		{
			if(!bIsConference)
			{
				g_pPlutoLogger->Write(LV_WARNING, "Transfer command called, but call is already in the conference, will continue this way");
				bIsConference=true;				
			}
		}
		if(bIsConference)
		{
			if(pCallData->getID().find(CONFERENCE_PREFIX)!=0) // not in the conference yet?
			{
				string room="000"+StringUtils::itos(next_conf_room);
				next_conf_room++;

				/*send transfer command to PBX*/
				CMD_PBX_Transfer cmd_PBX_Transfer(m_dwPK_Device, pPBXDevice->m_dwPK_Device, room, pCallData->getPendingCmdID(), pCallData->getID(),bIsConference);
				SendCommand(cmd_PBX_Transfer);
				//allow things to settle a bit
				Sleep(1000);
				DCE::CMD_PL_External_Originate cmd_invite(pCallData->getOwnerDevID(),m_dwPK_Device,sPhoneNumber,"conference",room);
				SendCommand(cmd_invite);
			}
			else
			{
				string chan_list = pCallData->getID();
				int pos=chan_list.find(' ');
				string room=chan_list.substr(1,pos-1);
				//just send invite into proper room
				DCE::CMD_PL_External_Originate cmd_invite(pCallData->getOwnerDevID(),m_dwPK_Device,sPhoneNumber,"conference",room);
				SendCommand(cmd_invite);
			}
		}
		else
		{
			/*send transfer command to PBX*/
			CMD_PBX_Transfer cmd_PBX_Transfer(pCallData->getOwnerDevID(), pPBXDevice->m_dwPK_Device, sPhoneNumber, pCallData->getPendingCmdID(), pCallData->getID(),bIsConference);
			SendCommand(cmd_PBX_Transfer);
		}
	}
}

//<-dceag-c236-b->

	/** @brief COMMAND: #236 - PL_Hangup */
	/** Hangs up a call */

void Telecom_Plugin::CMD_PL_Hangup(string &sCMD_Result,Message *pMessage)
//<-dceag-c236-e->
{
	g_pPlutoLogger->Write(LV_STATUS, "Hangup command called.");

	CallData *pCallData = CallManager::getInstance()->findCallByOwnerDevID(pMessage->m_dwPK_Device_From);
	if(!pCallData) {
		g_pPlutoLogger->Write(LV_WARNING, "No calldata found for device %d",pMessage->m_dwPK_Device_From);

		pCallData = CallManager::getInstance()->findCallByOwnerDevID(map_orbiter2embedphone[pMessage->m_dwPK_Device_From]);
		if(!pCallData) {
			g_pPlutoLogger->Write(LV_WARNING, "No calldata found for device %d",map_orbiter2embedphone[pMessage->m_dwPK_Device_From]);
			return;
		}
	}
	g_pPlutoLogger->Write(LV_STATUS, "Will hangup on channelid %s", pCallData->getID().c_str());
	/*find PBX*/
	DeviceData_Router* pPBXDevice = find_AsteriskDevice();
	if(pPBXDevice) {
		/*send transfer command to PBX*/
		CMD_PBX_Hangup cmd_PBX_Hangup(m_dwPK_Device, pPBXDevice->m_dwPK_Device,
								0, pCallData->getID());
		SendCommand(cmd_PBX_Hangup);
		CallManager::getInstance()->removeCall(pCallData);
	}
}
//<-dceag-createinst-b->!

void Telecom_Plugin::GetFloorplanDeviceInfo(DeviceData_Router *pDeviceData_Router,EntertainArea *pEntertainArea,int iFloorplanObjectType,int &iPK_FloorplanObjectType_Color,int &Color,string &sDescription,string &OSD,int &PK_DesignObj_Toolbar)
{
	int devid=pDeviceData_Router->m_dwPK_Device;
	std::list<CallData*> *calls = CallManager::getInstance()->getCallList();
	std::list<CallData*>::iterator it = calls->begin();
	std::list<string> uniq_chan;
	int count = 0;
	while(it != calls->end())
	{
		string channels = (*it)->getID();
		int pos = 0, oldpos = 0;
		do
		{
			pos = channels.find(' ',oldpos);
			string chan;
			int ext;
			string sext;
			if(pos < 0)
			{
				chan = channels.substr(oldpos, channels.length());
			}
			else
			{
				if(pos==oldpos)
				{
					oldpos=pos+1;
					continue;
				}
				chan = channels.substr(oldpos, pos - oldpos);
			}
			if(ParseChannel(chan,&ext,&sext)==0)
			{
				if(map_ext2device[ext]==devid)
				{
					Color = UniqueColors[count%MAX_TELECOM_COLORS];
					return;
				}
			}
			oldpos=pos+1;
		}
		while(pos>=0);
		it++;
		if(find(uniq_chan.begin(), uniq_chan.end(), channels) == uniq_chan.end())
		{
			uniq_chan.push_back(channels);
			count++;
		}
	}
	Color = PlutoColor::Black().m_Value;
}

//<-dceag-c414-b->

	/** @brief COMMAND: #414 - PL External Originate */
	/** Originate an external call */
		/** @param #75 PhoneNumber */
			/** Phone to call */
		/** @param #81 CallerID */
			/** Caller ID */
		/** @param #83 PhoneExtension */
			/** Phone extension to dial */

void Telecom_Plugin::CMD_PL_External_Originate(string sPhoneNumber,string sCallerID,string sPhoneExtension,string &sCMD_Result,Message *pMessage)
//<-dceag-c414-e->
{
    g_pPlutoLogger->Write(LV_STATUS, "Originate external command called with params: PhoneNumber=%s, PhoneExtension=%s!", sPhoneNumber.c_str(), sPhoneExtension.c_str());

    if(sPhoneExtension.empty()) {
        g_pPlutoLogger->Write(LV_CRITICAL, "Error validating input parameters");
        return;
    }

    string sSrcPhoneType = "Local";

    g_pPlutoLogger->Write(LV_STATUS, "Using source phone with parameters: PhoneChannel=%s, PhoneNumber=%s!",
        sSrcPhoneType.c_str(), sPhoneNumber.c_str());

	sPhoneNumber+="@trusted";

    /*find PBX*/
    DeviceData_Router* pPBXDevice = find_AsteriskDevice();
    if(pPBXDevice) {

        CallData *pCallData = CallManager::getInstance()->findCallByOwnerDevID(pMessage->m_dwPK_Device_From);
        if(!pCallData) {
            /*create new call data*/
            pCallData = new CallData();
            pCallData->setOwnerDevID(pMessage->m_dwPK_Device_From);
            CallManager::getInstance()->addCall(pCallData);
        }

        pCallData->setState(CallData::STATE_ORIGINATING);

        /*send originate command to PBX*/
        pCallData->setPendingCmdID(generate_NewCommandID());
        CMD_PBX_Originate cmd_PBX_Originate(m_dwPK_Device, pPBXDevice->m_dwPK_Device,
            sPhoneNumber, sSrcPhoneType, sPhoneExtension, sCallerID, pCallData->getPendingCmdID());
        SendCommand(cmd_PBX_Originate);
    }
}

string Telecom_Plugin::GetDialNumber(Row_PhoneNumber *pRow_PhoneNumber)
{
	if( pRow_PhoneNumber->DialAs_get().size() )
		return pRow_PhoneNumber->DialAs_get();

	// TODO - make this better
	string sDial;
	if( pRow_PhoneNumber->CountryCode_get().size() )
		sDial += "00" + pRow_PhoneNumber->CountryCode_get();

	if( pRow_PhoneNumber->AreaCode_get().size() )
		sDial += pRow_PhoneNumber->AreaCode_get();

	sDial += pRow_PhoneNumber->PhoneNumber_get();

	return sDial;
}



bool
Telecom_Plugin::IncomingCall( class Socket *pSocket, class Message *pMessage,
					 			class DeviceData_Base *pDeviceFrom, class DeviceData_Base *pDeviceTo ) {
	SCREEN_DevIncomingCall SCREEN_DevIncomingCall_(m_dwPK_Device,pDeviceFrom->m_dwPK_Device_ControlledVia);
	SendCommand(SCREEN_DevIncomingCall_);
	return true;
}
//<-dceag-c28-b->

	/** @brief COMMAND: #28 - Simulate Keypress */
	/** Send a DTMF code */
		/** @param #26 PK_Button */
			/** What key to simulate being pressed.  If 2 numbers are specified, separated by a comma, the second will be used if the Shift key is specified. */
		/** @param #50 Name */
			/** The application to send the keypress to. If not specified, it goes to the DCE device. */

void Telecom_Plugin::CMD_Simulate_Keypress(string sPK_Button,string sName,string &sCMD_Result,Message *pMessage)
//<-dceag-c28-e->
{
	int phoneID=map_orbiter2embedphone[pMessage->m_dwPK_Device_From];
	if(phoneID>0)
	{
		DCE::CMD_Simulate_Keypress cmd(m_dwPK_Device,phoneID,sPK_Button,sName);
		SendCommand(cmd);
	}
	sCMD_Result="OK";
}
//<-dceag-c334-b->

	/** @brief COMMAND: #334 - Phone_Initiate */
	/** Initiates a call */
		/** @param #83 PhoneExtension */
			/** Extention to dial */

void Telecom_Plugin::CMD_Phone_Initiate(string sPhoneExtension,string &sCMD_Result,Message *pMessage)
//<-dceag-c334-e->
{
	int phoneID=map_orbiter2embedphone[pMessage->m_dwPK_Device_From];
	if(phoneID>0)
	{
		DCE::CMD_Phone_Initiate cmd(m_dwPK_Device,phoneID,sPhoneExtension);
		SendCommand(cmd);
		CallData *pCallData = CallManager::getInstance()->findCallByOwnerDevID(phoneID);
		if(!pCallData) {
			/*create new call data*/
			pCallData = new CallData();
			pCallData->setOwnerDevID(phoneID);
			CallManager::getInstance()->addCall(pCallData);
		}
		pCallData->setState(CallData::STATE_ORIGINATING);
	}

	sCMD_Result="OK";
}
//<-dceag-c335-b->

	/** @brief COMMAND: #335 - Phone_Answer */
	/** Answer a call */

void Telecom_Plugin::CMD_Phone_Answer(string &sCMD_Result,Message *pMessage)
//<-dceag-c335-e->
{
	int phoneID=map_orbiter2embedphone[pMessage->m_dwPK_Device_From];
	if(phoneID>0)
	{
		DCE::CMD_Phone_Answer cmd(m_dwPK_Device,phoneID);
		SendCommand(cmd);
		CallData *pCallData = CallManager::getInstance()->findCallByOwnerDevID(phoneID);
		if(!pCallData) {
			/*create new call data*/
			pCallData = new CallData();
			pCallData->setOwnerDevID(phoneID);
			CallManager::getInstance()->addCall(pCallData);
			g_pPlutoLogger->Write(LV_WARNING, "Answering a call for which have no data");
		}
		pCallData->setState(CallData::STATE_OPENED);
	}
	sCMD_Result="OK";
}
//<-dceag-c336-b->

	/** @brief COMMAND: #336 - Phone_Drop */
	/** Drop a call */

void Telecom_Plugin::CMD_Phone_Drop(string &sCMD_Result,Message *pMessage)
//<-dceag-c336-e->
{
	int phoneID=map_orbiter2embedphone[pMessage->m_dwPK_Device_From];
	if(phoneID>0)
	{
		DCE::CMD_Phone_Drop cmd(m_dwPK_Device,phoneID);
		SendCommand(cmd);
		CallData *pCallData = CallManager::getInstance()->findCallByOwnerDevID(phoneID);
		if(pCallData) {
			CallManager::getInstance()->removeCall(pCallData);
		}
		else
		{
			g_pPlutoLogger->Write(LV_CRITICAL, "Dropping a call for which have no data");
		}
	}
	sCMD_Result="OK";
}
//<-dceag-c744-b->

	/** @brief COMMAND: #744 - Set User Mode */
	/** Set a user to a given mode (sleeping, etc.) */
		/** @param #17 PK_Users */
			/** The user */
		/** @param #194 PK_UserMode */
			/** The user mode */

void Telecom_Plugin::CMD_Set_User_Mode(int iPK_Users,int iPK_UserMode,string &sCMD_Result,Message *pMessage)
//<-dceag-c744-e->
{
	if( !iPK_Users )
	{
#ifdef DEBUG
		g_pPlutoLogger->Write(LV_STATUS,"Telecom_Plugin::CMD_Set_User_Mode no user specified.  Doing current user on orbiter %d",pMessage->m_dwPK_Device_From);
#endif
		OH_Orbiter *pOH_Orbiter = m_pOrbiter_Plugin->m_mapOH_Orbiter_Find(pMessage->m_dwPK_Device_From);
		if( pOH_Orbiter && pOH_Orbiter->m_pOH_User && pOH_Orbiter->m_pOH_User->m_iPK_Users )
			iPK_Users = pOH_Orbiter->m_pOH_User->m_iPK_Users;
		else
			return;
	}
	class Row_Users* pRow_Users;
	pRow_Users=m_pDatabase_pluto_main->Users_get()->GetRow(iPK_Users);
	if( pRow_Users )
	{
		pRow_Users->Reload();
		pRow_Users->FK_UserMode_set(iPK_UserMode);
		m_pDatabase_pluto_main->Users_get()->Commit();
	}
	else
		g_pPlutoLogger->Write(LV_CRITICAL,"Trying to set user mode for invalid user %d",iPK_Users);

	for(map<int,OH_Orbiter *>::iterator it=m_pOrbiter_Plugin->m_mapOH_Orbiter.begin();it!=m_pOrbiter_Plugin->m_mapOH_Orbiter.end();++it)
	{
		OH_Orbiter *pOH_Orbiter = (*it).second;

		DCE::CMD_Set_Bound_Icon CMD_Set_Bound_Icon(m_dwPK_Device,pOH_Orbiter->m_pDeviceData_Router->m_dwPK_Device,
			StringUtils::itos(iPK_UserMode),"","user" + StringUtils::itos(iPK_Users));
		SendCommand(CMD_Set_Bound_Icon);
	}
}
//<-dceag-c751-b->

	/** @brief COMMAND: #751 - PL_Add_VOIP_Account */
	/** Add a VOIP account */
		/** @param #50 Name */
			/** Provider name */
		/** @param #75 PhoneNumber */
			/** Phone number */
		/** @param #99 Password */
			/** Password */
		/** @param #189 Users */
			/** User name */

void Telecom_Plugin::CMD_PL_Add_VOIP_Account(string sName,string sPhoneNumber,string sPassword,string sUsers,string &sCMD_Result,Message *pMessage)
//<-dceag-c751-e->
{
	string cmdline = "";

	if(sName == "broadvoice")
	{
		cmdline += "/usr/pluto/bin/create_amp_broadvoice.pl";
	}
	if(sName == "freeworlddialup")
	{
		cmdline += "/usr/pluto/bin/create_amp_fwd.pl";
	}
	if(sName == "inphonex")
	{
		cmdline += "/usr/pluto/bin/create_amp_inphonex.pl";
	}
	if(sName == "efon")
	{
		cmdline += "/usr/pluto/bin/create_amp_efon.pl";
	}
	if(sName == "teliax")
	{
		cmdline += "/usr/pluto/bin/create_amp_teliax.pl";
	}
	if(sName == "nufone")
	{
		cmdline += "/usr/pluto/bin/create_amp_nufone.pl";
	}

	cmdline+= string(" ")+sUsers+(" ")+sPassword+string(" ")+sPhoneNumber;
	g_pPlutoLogger->Write(LV_WARNING, "Will call %s",cmdline.c_str());
	system(cmdline.c_str());
}

bool Telecom_Plugin::OrbiterRegistered(class Socket *pSocket,class Message *pMessage,class DeviceData_Base *pDeviceFrom,class DeviceData_Base *pDeviceTo)
{
	bool bRegistered = pMessage->m_mapParameters[COMMANDPARAMETER_OnOff_CONST]=="1";
	g_pPlutoLogger->Write(LV_STATUS,"Telecom_Plugin::OrbiterRegistered orbiter %d registered %d",pDeviceFrom->m_dwPK_Device,(int) bRegistered);
	
	if( bRegistered )
	{
		vector<Row_Users *> vectRow_Users;
		m_pDatabase_pluto_main->Users_get()->GetRows(USERS_FK_INSTALLATION_MAIN_FIELD + string("=") + StringUtils::itos(m_pRouter->iPK_Installation_get())+string(" OR ")+string(USERS_FK_INSTALLATION_MAIN_FIELD)+string(" IS NULL"),&vectRow_Users);
		for(size_t s=0;s<vectRow_Users.size();++s)
		{
			Row_Users *pRow_Users = vectRow_Users[s];
			DCE::CMD_Set_Bound_Icon CMD_Set_Bound_Icon(m_dwPK_Device,pMessage->m_dwPK_Device_From,
				StringUtils::itos(pRow_Users->FK_UserMode_get()),"","user" + StringUtils::itos(pRow_Users->PK_Users_get()));
			SendCommand(CMD_Set_Bound_Icon);
			g_pPlutoLogger->Write(LV_STATUS,"Telecom_Plugin::OrbiterRegistered Set_Bound_Icon(usermode = %d, user = %d)",pRow_Users->FK_UserMode_get(),pRow_Users->PK_Users_get());
		}
	}
	return true;
}

class DataGridTable *Telecom_Plugin::ActiveCallsGrid(string GridID,string Parms,void *ExtraData,int *iPK_Variable,string *sValue_To_Assign,class Message *pMessage)
{
	g_pPlutoLogger->Write(LV_STATUS, "ActiveCalls request received for GridID: %s",GridID.c_str());
	DataGridTable *pDataGrid = new DataGridTable();
	DataGridCell *pCell;
	int Row = 0;
	std::list<CallData*> *calls = CallManager::getInstance()->getCallList();
	std::list<CallData*>::iterator it = calls->begin();
	std::list<std::string> text_list;
	while(it != calls->end())
	{
		string channels = (*it)->getID();
		list<int> ext_list;
		string ext_txt;
		int pos = 0, oldpos = 0, count = 0;
		g_pPlutoLogger->Write(LV_STATUS,"Channels : %s associated with %d",channels.c_str(),(*it)->getOwnerDevID());
		do
		{
			pos = channels.find(' ',oldpos);
			string chan;
			int ext;
			string sext;
			if(pos < 0)
			{
				chan = channels.substr(oldpos, channels.length());
			}
			else
			{
				if(pos==oldpos)
				{
					oldpos=pos+1;
					continue;
				}
				chan = channels.substr(oldpos, pos - oldpos);
			}
			if(ParseChannel(chan,&ext,&sext)==0)
			{
				if(find(ext_list.begin(), ext_list.end(), ext) == ext_list.end())
				{
					ext_list.push_back(ext);
					ext_txt+=sext+string(" ");
				}
			}
			else
			{
				if(chan.find(CONFERENCE_PREFIX)!=0)
				{
					g_pPlutoLogger->Write(LV_STATUS,"   chan [%s], callerid [%s]",chan.c_str(),(*it)->getCallerID().c_str());
					if((*it)->getCallerID().find_first_not_of(" \"<>")>=0)
					{
						g_pPlutoLogger->Write(LV_STATUS,"   chan add");
						ext_txt+=((*it)->getCallerID())+string(" ");
					}
					else
					{
						ext_txt+= "UNKNOWN ";
						g_pPlutoLogger->Write(LV_STATUS,"   chan skip");
					}
				}
				else
				{
					g_pPlutoLogger->Write(LV_STATUS,"   this is a conference");
				}
			}
			oldpos=pos+1;
			count++;
		}
		while(pos>=0);
		if(count>1)
		{
			if(find(text_list.begin(), text_list.end(), ext_txt) == text_list.end())
			{
				g_pPlutoLogger->Write(LV_STATUS,"WILL SHOW CALLDATA %s",ext_txt.c_str());
				pCell = new DataGridCell(ext_txt,channels);
		        pCell->m_AltColor = UniqueColors[Row%MAX_TELECOM_COLORS];
				pDataGrid->SetData(0,Row,pCell);
				text_list.push_back(ext_txt);
				Row++;
			}
			else
			{
				g_pPlutoLogger->Write(LV_STATUS,"ALREADY HAVE THIS ONE %s",ext_txt.c_str());
			}
		}
		else
		{
			g_pPlutoLogger->Write(LV_STATUS,"INSUFFICIENT CALLDATA %s",ext_txt.c_str());
		}
		it++;
	}
	return pDataGrid;
}
static char* sec2str(int sec)
{
	static char buff[32];
	int s=sec%60;
	int m=(sec/60)%60;
	int h=(sec/3600);
	if(h>0)
		sprintf(buff,"%d:%02d:%02d",h,m,s);
	else
		sprintf(buff,"%02d:%02d",m,s);
	return buff;
}

class DataGridTable *Telecom_Plugin::RecentCallsGrid(string GridID,string Parms,void *ExtraData,int *iPK_Variable,string *sValue_To_Assign,class Message *pMessage)
{
	g_pPlutoLogger->Write(LV_STATUS, "RecentCalls request received for GridID: %s",GridID.c_str());
	DataGridTable *pDataGrid = new DataGridTable();
	DataGridCell *pCell;
	int Row = 0;

	DCEConfig dceconf;
	MySqlHelper mySqlHelper(dceconf.m_sDBHost, dceconf.m_sDBUser, dceconf.m_sDBPassword, "asteriskcdrdb" ,dceconf.m_iDBPort);
	PlutoSqlResult result_set;
	MYSQL_ROW row=NULL;
	if( (result_set.r=mySqlHelper.mysql_query_result("SELECT src, dst, calldate, billsec, channel FROM cdr ORDER BY calldate DESC LIMIT 0,20")) == NULL )
	{
		g_pPlutoLogger->Write(LV_WARNING, "SQL FAILED");
		return pDataGrid;
	}
	while((row = mysql_fetch_row(result_set.r)))
	{
		int ext;
		string sext=row[0];
		string sext2=row[1];
		if((sext2.find_first_not_of("0123456789")==sext2.npos) && (sext2.find("000")!=0))
		{
			g_pPlutoLogger->Write(LV_STATUS,"   sext1='%s'",sext.c_str());
			if(sext == "")
			{
				ParseChannel(row[4],&ext,&sext);
				g_pPlutoLogger->Write(LV_STATUS,"   sext2='%s'",sext.c_str());
				if(sext == "")
				{
					sext="Unknown";
				}
			}
			if(sext != sext2)
			{
				string text = sext+string(" to ")+sext2+string(" at ")+string(row[2])+string("(")+string(sec2str(atoi(row[3])))+string(")");
				g_pPlutoLogger->Write(LV_STATUS,"WILL SHOW  %s",text.c_str());
				pCell = new DataGridCell(text,sext2);
				pCell->m_pMessage=NULL;
				pDataGrid->SetData(0,Row,pCell);
				Row++;
			}
		}
	}

	return pDataGrid;
}

class DataGridTable *Telecom_Plugin::SpeedDialGrid(string GridID,string Parms,void *ExtraData,int *iPK_Variable,string *sValue_To_Assign,class Message *pMessage)
{
	g_pPlutoLogger->Write(LV_STATUS, "SpeedDial request received for GridID: %s with Params: %s",GridID.c_str(),Parms.c_str());
	DataGridTable *pDataGrid = new DataGridTable();
	DataGridCell *pCell;
	int Row = 0;
	string::size_type pos=0;
	string something7 = StringUtils::Tokenize(Parms,",",pos);
	string room = StringUtils::Tokenize(Parms,",",pos);
	if( room.size()==0 )
	{
		g_pPlutoLogger->Write(LV_WARNING, "SpeedDial request has no room");
		return pDataGrid;
	}

    ///SELECT CommandGroup.Description, FK_CommandParameter, IK_CommandParameter FROM CommandGroup JOIN CommandGroup_Command ON CommandGroup_Command.FK_CommandGroup=PK_CommandGroup JOIN CommandGroup_Command_CommandParameter ON FK_CommandGroup_Command=PK_CommandGroup_Command JOIN CommandGroup_Room ON CommandGroup_Room.FK_CommandGroup=PK_CommandGroup WHERE FK_Array=4 AND AutoGenerated=0 AND FK_Room=XXX ORDER BY PK_CommandGroup,FK_CommandParameter;
	string sql_buff = "SELECT CommandGroup.Description,FK_CommandParameter,IK_CommandParameter";
	sql_buff += " FROM CommandGroup";
	sql_buff += " JOIN CommandGroup_Command ON CommandGroup_Command.FK_CommandGroup=PK_CommandGroup";
	sql_buff += " JOIN CommandGroup_Command_CommandParameter ON FK_CommandGroup_Command=PK_CommandGroup_Command";
	sql_buff += " JOIN CommandGroup_Room ON CommandGroup_Room.FK_CommandGroup=PK_CommandGroup";
	sql_buff += " WHERE AutoGenerated=0";
	sql_buff += " AND FK_Array="+StringUtils::itos(ARRAY_Communication_Scenarios_CONST);
	sql_buff += " AND FK_Room="+room;
	sql_buff += " ORDER BY PK_CommandGroup,FK_CommandParameter";

	DCEConfig dceconf;
	MySqlHelper mySqlHelper(dceconf.m_sDBHost, dceconf.m_sDBUser, dceconf.m_sDBPassword, dceconf.m_sDBName ,dceconf.m_iDBPort);
	PlutoSqlResult result_set;
	MYSQL_ROW row=NULL;
	if( (result_set.r=mySqlHelper.mysql_query_result(sql_buff)) == NULL )
	{
		g_pPlutoLogger->Write(LV_WARNING, "SQL FAILED");
		return pDataGrid;
	}
	char desc[256];
	while((row = mysql_fetch_row(result_set.r)))
	{
		int device=0;
		if(atoi(row[1])==COMMANDPARAMETER_PK_Device_CONST)
		{
			device=atoi(row[2]);
			strncpy(desc,row[0],sizeof(desc)-1);
		}
		else
		{
			g_pPlutoLogger->Write(LV_WARNING,"This should not happend (1st row not PK_Device)");
			break;
		}
		if((row = mysql_fetch_row(result_set.r))==NULL)
		{
			g_pPlutoLogger->Write(LV_WARNING,"This should not happend (no 2nd row for '%s')",desc);
			break;
		}
		if(atoi(row[1])!=COMMANDPARAMETER_PhoneExtension_CONST)
		{
			g_pPlutoLogger->Write(LV_WARNING,"This should not happend (2nd row not PhoneExtension)");
			break;
		}
		if(strcmp(row[0],desc))
		{
			g_pPlutoLogger->Write(LV_WARNING,"This should not happend ('%s' != '%s')",desc,row[0]);
			break;
		}

		string text = string(row[0])+string(" (")+string(row[2])+string(")");
		g_pPlutoLogger->Write(LV_STATUS,"WILL SHOW  %s / %d",text.c_str(),device);
		pCell = new DataGridCell(text,"");
		DCE::CMD_PL_Originate CMD_PL_Originate_(m_dwPK_Device,m_dwPK_Device,device,row[2],"");
		pCell->m_pMessage=CMD_PL_Originate_.m_pMessage;
		pDataGrid->SetData(0,Row,pCell);
		Row++;
	}
	return pDataGrid;
}

bool Telecom_Plugin::Hangup( class Socket *pSocket, class Message *pMessage,
					 			class DeviceData_Base *pDeviceFrom, class DeviceData_Base *pDeviceTo ) {
	int iPhoneExtension = atoi(pMessage->m_mapParameters[EVENTPARAMETER_PhoneExtension_CONST].c_str());
	g_pPlutoLogger->Write(LV_STATUS, "Hangup %d(#%d) event received from PBX.",iPhoneExtension,map_ext2device[iPhoneExtension]);
	CallData *pCallData = CallManager::getInstance()->findCallByOwnerDevID(map_ext2device[iPhoneExtension]);
	if(pCallData) {
		g_pPlutoLogger->Write(LV_STATUS, "Will hangup on device %d callid %s",map_ext2device[iPhoneExtension],pCallData->getID().c_str());
		CallManager::getInstance()->removeCall(pCallData);
	}
	return true;
}

int Telecom_Plugin::ParseChannel(const std::string channel, int* iextension, string *sextension)
{
	int pos, oldpos = 0;

	pos = channel.find('/');
	if(pos < 0) {
		return -1;
	}

	oldpos = pos + 1;
	pos = channel.find('@',oldpos);
	if(pos < 0) {
		pos = channel.find('-',oldpos);
	}
	if(pos < 0) {
		pos = channel.find('/',oldpos);
	}
	if(pos < 0) {
		pos = channel.find('&',oldpos);
	}
	if(pos < 0) {
		pos = channel.find(',',oldpos);
	}
	if(pos < 0) {
		pos = channel.find('|',oldpos);
	}
	if(pos < 0) {
		return -1;
	}

	if(iextension && sextension) {
		*sextension = channel.substr(oldpos, pos - oldpos);
		if(sscanf(sextension->c_str(),"%d",iextension)!=1)
		{
			return -1;
		}
	}
	return 0;
}

bool Telecom_Plugin::VoIP_Problem(class Socket *pSocket,class Message *pMessage,class DeviceData_Base *pDeviceFrom,class DeviceData_Base *pDeviceTo)
{
	int iLevel = atoi(pMessage->m_mapParameters[EVENTPARAMETER_Value_CONST].c_str());
	string sText = pMessage->m_mapParameters[EVENTPARAMETER_Text_CONST];
	g_pPlutoLogger->Write(LV_WARNING, "Received VoIP problem : level %d, description :\n%s",iLevel,sText.c_str());
	time_t now=time(NULL);
	pthread_mutex_lock(&mtx_err_messages);
	if(map_err_messages.find(sText) != map_err_messages.end())
	{
		g_pPlutoLogger->Write(LV_STATUS, "Already have this message");	
	}
	else
	{
		map_err_messages[sText]=now;
		g_pPlutoLogger->Write(LV_STATUS, "Will add to queue");	
	}
	pthread_mutex_unlock(&mtx_err_messages);
	if((iLevel == 7) && (sText.length()>0))
	{
		SCREEN_DialogGenericNoButtons_Cat SCREEN_DialogGenericNoButtons_(m_dwPK_Device, 5, false, BL_SameHouse,sText,"0","0","0");
		SendCommand(SCREEN_DialogGenericNoButtons_);
	}
	return true;
}

void * startDisplayThread(void * Arg)
{
    Telecom_Plugin *telecom_pugin = (Telecom_Plugin *) Arg;
    g_pPlutoLogger->Write(LV_STATUS, "Started Display Thread");
    telecom_pugin->doDisplayMessages();
    return NULL;
}
void Telecom_Plugin::doDisplayMessages()
{
	while(!m_bQuit)
    {
		Sleep(1000);
		if(1)//the orbiter is at main menu ?? how to find out??
		{
			pthread_mutex_lock(&mtx_err_messages);
			string message;
			map<string,long>::iterator it = map_err_messages.begin();
			while(it != map_err_messages.end())
			{
				message += (*it).first + string("\n");
				it++;
			}
			if(message.length()>0)
			{
				g_pPlutoLogger->Write(LV_STATUS, "Will write mesaage : %s",message.c_str());
				map_err_messages.clear();
				//Don't send it yet				
				//SCREEN_DialogGenericNoButtons_Cat SCREEN_DialogGenericNoButtons_(m_dwPK_Device, 5, false, BL_SameHouse,message,"0","0","0");
				//SendCommand(SCREEN_DialogGenericNoButtons_);
			}
			pthread_mutex_unlock(&mtx_err_messages);
		}
	}
}

//<-dceag-c797-b->

	/** @brief COMMAND: #797 - PL_Join_Call */
	/** Will join you to an existing call */
		/** @param #86 CallID */
			/** Call ID to join to */
		/** @param #103 List PK Device */
			/** Devices which will be added to the call */

void Telecom_Plugin::CMD_PL_Join_Call(string sCallID,string sList_PK_Device,string &sCMD_Result,Message *pMessage)
//<-dceag-c797-e->
{
	g_pPlutoLogger->Write(LV_STATUS, "Command PL_Join_Call (call=%s, devices=%s)",sCallID.c_str(),sList_PK_Device.c_str());
	list <int> dev_join_list;
	list <int> dev_gts_list;
	int pos = 0, oldpos = 0;
	do
	{
		pos = sList_PK_Device.find(',',oldpos);
		string device;
		if(pos < 0)
		{
			device = sList_PK_Device.substr(oldpos, sList_PK_Device.length());
		}
		else
		{
			device = sList_PK_Device.substr(oldpos, pos - oldpos);
		}
		int idev=atoi(device.c_str());
		if(map_device2ext[idev] == 0)
		{
			idev=map_orbiter2embedphone[idev];
		}
		if(map_device2ext[idev] == 0)
		{
			g_pPlutoLogger->Write(LV_STATUS, "Ignoring device %s(%d) as has no extension",device.c_str(),idev);
			oldpos=pos+1;
			continue;
		}
		CallData *pCallData = CallManager::getInstance()->findCallByOwnerDevID(idev);
		if(pCallData)
		{
			if(pCallData->getID() == sCallID)
			{
				g_pPlutoLogger->Write(LV_STATUS, "Device #%d is already connected to this call",idev);
				if(map_embedphone2orbiter[idev] != 0)
				{
					g_pPlutoLogger->Write(LV_STATUS, "Add #%d in GTS list",idev);
					dev_gts_list.push_back(idev);
				}
			}
			else
			{
				g_pPlutoLogger->Write(LV_STATUS, "Device #%d has some other call '%s'",idev,pCallData->getID().c_str());
			}
		}
		else
		{
			g_pPlutoLogger->Write(LV_STATUS, "Add #%d in JOIN list",idev);
			dev_join_list.push_back(idev);
		}
		oldpos=pos+1;
	}
	while(pos>=0);
	
	pos = oldpos = 0;
	do
	{
		pos = sCallID.find(' ',oldpos);
		string chan;		
		if(pos < 0)
		{
			chan = sCallID.substr(oldpos, sCallID.length());
		}
		else
		{
			if(pos==oldpos)
			{
				oldpos=pos+1;
				continue;
			}
			chan = sCallID.substr(oldpos, pos - oldpos);
		}
		int ext;
		string sext;
		if(ParseChannel(chan,&ext,&sext)==0)
		{
			if(find(dev_gts_list.begin(), dev_gts_list.end(), ext) == dev_gts_list.end())
			{
				int idev = map_ext2device[ext];
				if(map_embedphone2orbiter[idev] != 0)
				{
					g_pPlutoLogger->Write(LV_STATUS, "Add #%d in GTS list",idev);
					dev_gts_list.push_back(idev);
				}
				else
				{
					g_pPlutoLogger->Write(LV_STATUS, "Don't add #%d in GTS list",idev);
				}
			}
		}
		oldpos=pos+1;
	}
	while(pos>=0);
	
	list<int>::iterator it;
	
	for(it=dev_gts_list.begin();it!=dev_gts_list.end();it++)
	{
		SCREEN_DevCallInProgress SCREEN_DevCallInProgress_(m_dwPK_Device,map_embedphone2orbiter[(*it)]);
	    SendCommand(SCREEN_DevCallInProgress_);
	}
	
	if(!dev_join_list.empty())
	{
		string room;	
		if(sCallID.find(CONFERENCE_PREFIX) != 0) //not a conference
		{
			room="000"+StringUtils::itos(next_conf_room);
			next_conf_room++;
			g_pPlutoLogger->Write(LV_STATUS, "Will create conference room %d",room.c_str());
	    	DeviceData_Router* pPBXDevice = find_AsteriskDevice();
	    	if(pPBXDevice) {
				CMD_PBX_Transfer cmd_PBX_Transfer(m_dwPK_Device, pPBXDevice->m_dwPK_Device, room, generate_NewCommandID(), sCallID, true);
				SendCommand(cmd_PBX_Transfer);
			}
			else
			{
				g_pPlutoLogger->Write(LV_CRITICAL,"Could not found Asterisk device");
				return;
			}
		}
		else
		{
			pos=sCallID.find(' ');
			room=sCallID.substr(1,pos-1);
			g_pPlutoLogger->Write(LV_STATUS, "Will join into conference room %d",room.c_str());		
		}
		Sleep(2000);
		for(it=dev_join_list.begin();it!=dev_join_list.end();it++)
		{
			DCE::CMD_PL_External_Originate cmd_invite((*it),m_dwPK_Device,StringUtils::itos(map_device2ext[(*it)]),"conference",room);
			SendCommand(cmd_invite);
			Sleep(200);		
		}
	}
}

bool Telecom_Plugin::VoiceMailChanged(class Socket *pSocket,class Message *pMessage,class DeviceData_Base *pDeviceFrom,class DeviceData_Base *pDeviceTo)
{
	int userid = atoi(pMessage->m_mapParameters[EVENTPARAMETER_PK_Users_CONST].c_str());
	string vmcount = pMessage->m_mapParameters[EVENTPARAMETER_Value_CONST];
	string::size_type pos=0;
	int vm_new = atoi(StringUtils::Tokenize(vmcount," ",pos).c_str());
	int vm_old = atoi(StringUtils::Tokenize(vmcount," ",pos).c_str());
	string value_param = "0";
	string text_param = "0";
	if(vm_old != 0)
	{
		value_param = "1";
		text_param = StringUtils::itos(vm_old);
	}
	if(vm_new != 0)
	{	
		value_param = "2";
		text_param = StringUtils::itos(vm_new);		
	}
	
	for(map<int,OH_Orbiter *>::iterator it=m_pOrbiter_Plugin->m_mapOH_Orbiter.begin();it!=m_pOrbiter_Plugin->m_mapOH_Orbiter.end();++it)
	{
		OH_Orbiter *pOH_Orbiter = (*it).second;
		DCE::CMD_Set_Bound_Icon CMD_Set_Bound_Icon(m_dwPK_Device,pOH_Orbiter->m_pDeviceData_Router->m_dwPK_Device,value_param,text_param,"vm"+StringUtils::itos(userid));
		SendCommand(CMD_Set_Bound_Icon);
	}

	return true;	
}

/* this makes the plugin asterisk dependend */
#define VOICEMAIL_LOCATION "/var/lib/asterisk/sounds/voicemail/default/"
class DataGridTable *Telecom_Plugin::UserVoiceMailGrid(string GridID,string Parms,void *ExtraData,int *iPK_Variable,string *sValue_To_Assign,class Message *pMessage)
{
	g_pPlutoLogger->Write(LV_STATUS, "UserVoiceMailGrid request received for GridID: %s with Params: %s",GridID.c_str(),Parms.c_str());
	DataGridTable *pDataGrid = new DataGridTable();
#ifndef WIN32
	DataGridCell *pCell;
	int Row = 0;
	string userid = Parms;

	if( userid.size()==0 )
	{
		g_pPlutoLogger->Write(LV_WARNING, "UserVoiceMailGrid request has no user");
		return pDataGrid;
	}
	
	string user_path=string(VOICEMAIL_LOCATION)+userid+string("/INBOX/");
	DIR * dir=opendir(user_path.c_str());
	struct dirent *dir_ent = NULL;
	while((dir_ent = readdir(dir)))
	{
	    struct stat statbuf;
		string buffer = user_path+dir_ent->d_name;
        stat(buffer.c_str(),&statbuf);
        if((S_ISREG(statbuf.st_mode)) && (dir_ent->d_name[0] != '.') && (strstr(dir_ent->d_name,".txt") != NULL))
        {
			string text = "New message " + StringUtils::itos(Row+1);
			string file_path=user_path+(dir_ent->d_name);
			file_path.replace(file_path.length()-4,4,".wav");
			g_pPlutoLogger->Write(LV_STATUS,"WILL SHOW %s / %s",text.c_str(),file_path.c_str());

			string url="http://dcerouter/pluto-admin/amp/recordings/misc/audio.php?recording="+file_path;
			DCE::CMD_MH_Play_Media CMD_MH_Play_Media_(m_dwPK_Device, DEVICETEMPLATE_VirtDev_Media_Plugin_CONST, pMessage->m_dwPK_Device_From, url, MEDIATYPE_pluto_StoredAudio_CONST,0,"",1,0);
			
			pCell = new DataGridCell(text,"");			
			pCell->m_pMessage=CMD_MH_Play_Media_.m_pMessage;
			pDataGrid->SetData(0,Row,pCell);
			Row++;
		}
	}
	user_path=string(VOICEMAIL_LOCATION)+userid+string("/INBOX/Old");
	dir=opendir(user_path.c_str());
	dir_ent = NULL;
	while((dir_ent = readdir(dir)))
	{
	    struct stat statbuf;
		string buffer = user_path+dir_ent->d_name;
        stat(buffer.c_str(),&statbuf);
        if((S_ISREG(statbuf.st_mode)) && (dir_ent->d_name[0] != '.') && (strstr(dir_ent->d_name,".txt") != NULL))
        {
			string text = "Old message " + StringUtils::itos(Row+1);
			string file_path=user_path+(dir_ent->d_name);
			file_path.replace(file_path.length()-4,4,".wav");
			g_pPlutoLogger->Write(LV_STATUS,"WILL SHOW %s / %s",text.c_str(),file_path.c_str());
			string url="http://dcerouter/pluto-admin/amp/recordings/misc/audio.php?recording="+file_path;
			DCE::CMD_MH_Play_Media CMD_MH_Play_Media_(m_dwPK_Device, DEVICETEMPLATE_VirtDev_Media_Plugin_CONST, pMessage->m_dwPK_Device_From, url, MEDIATYPE_pluto_StoredAudio_CONST,0,"",1,0);
			
			pCell = new DataGridCell(text,"");			
			pCell->m_pMessage=CMD_MH_Play_Media_.m_pMessage;
			pDataGrid->SetData(0,Row,pCell);
			Row++;
		}
	}	
#endif
	return pDataGrid;
}
