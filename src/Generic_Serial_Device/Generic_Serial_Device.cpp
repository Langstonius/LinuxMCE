/*
	Copyright (C) 2004 Pluto, Inc., a Florida Corporation

	http://www.plutohome.com

	Phone: +1 (877) 758-8648

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License.
	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
	of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.
*/

//<-dceag-d-b->
#include "Generic_Serial_Device.h"
#include "DCE/Logger.h"
#include "PlutoUtils/FileUtils.h"
#include "PlutoUtils/StringUtils.h"
#include "PlutoUtils/Other.h"

#include <iostream>
using namespace std;
using namespace DCE;

#include "Gen_Devices/AllCommandsRequests.h"
//<-dceag-d-e->

#include "DCE/DCEConfig.h"
#include "RubyIOManager.h"

#define GSD_COMMAND_LINE	"Generic_Serial_Device"

//<-dceag-const-b->
// The primary constructor when the class is created as a stand-alone device
Generic_Serial_Device::Generic_Serial_Device(int DeviceID, string ServerAddress,bool bConnectEventHandler,bool bLocalMode,class Router *pRouter)
	: Generic_Serial_Device_Command(DeviceID, ServerAddress,bConnectEventHandler,bLocalMode,pRouter)
//<-dceag-const-e->
{
}

//<-dceag-getconfig-b->
bool Generic_Serial_Device::GetConfig()
{
	if( !Generic_Serial_Device_Command::GetConfig() )
		return false;
//<-dceag-getconfig-e->
	RubyIOManager* pmanager = RubyIOManager::getInstance();
	pmanager->setEventDispatcher(GetEvents());
	GSDMessageProcessor::setCommandImpl(this);
	Start();
	return true;
}

//<-dceag-const2-b->!

//<-dceag-dest-b->
Generic_Serial_Device::~Generic_Serial_Device()
//<-dceag-dest-e->
{

}

bool Generic_Serial_Device::Connect(int iPK_DeviceTemplate )
{
	if( !Generic_Serial_Device_Command::Connect(iPK_DeviceTemplate) )
		return false;
	return true;
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
void Generic_Serial_Device::ReceivedCommandForChild(DeviceData_Base *pDeviceData_Base,string &sCMD_Result,Message *pMessage)
//<-dceag-cmdch-e->
{
	sCMD_Result = "UNHANDLED CHILD";

	if(GSDMessageProcessor::ProcessMessage(pMessage)) {
	    g_pPlutoLogger->Write(LV_STATUS, "Message processed by Translator.");
		return;
	}

	DispatchMessage(pMessage);
}

/*
	When you received a valid command, but it wasn't for one of your children,
	then ReceivedUnknownCommand gets called.  If you handle the message, you 
	should change the sCMD_Result to OK
*/
//<-dceag-cmduk-b->
void Generic_Serial_Device::ReceivedUnknownCommand(string &sCMD_Result,Message *pMessage)
//<-dceag-cmduk-e->
{
	sCMD_Result = "UNKNOWN DEVICE";
    g_pPlutoLogger->Write(LV_STATUS, "Received UNKNOWN command.");
	
	if(GSDMessageProcessor::ProcessMessage(pMessage)) {
	    g_pPlutoLogger->Write(LV_STATUS, "Message processed by Translator.");
		return;
	}
	
	DispatchMessage(pMessage);
}

//<-dceag-sample-b->!
//<-dceag-reg-b->!
bool Generic_Serial_Device::Register(){return false;}
//<-dceag-createinst-b->!

/*

	COMMANDS TO IMPLEMENT

*/

// Intercept all commands
/*
bool Generic_Serial_Device::ReceivedMessage(class Message *pMessageOriginal)
{
	map<int,string>::iterator itCodeMap;
	int iHandled=0;

	for(int s=-1;s<(int) pMessageOriginal->m_vectExtraMessages.size(); ++s)
	{
		Message *pMessage = s>=0 ? pMessageOriginal->m_vectExtraMessages[s] : pMessageOriginal;
		if( pMessage->m_dwMessage_Type != MESSAGETYPE_COMMAND || (itCodeMap=m_CodeMap.find(pMessage->m_dwID))==m_CodeMap.end() )
			iHandled += Generic_Serial_Device_Command::ReceivedMessage(pMessage);  // If it's not a command or we don't have an entry for it, let the base class handle it
		else
		{
//			TransmitCommand((*itCodeMap).second,pMessage);
		}
	}
	return iHandled!=0;
}
*/
void Generic_Serial_Device::Transmit(const char *pData,int iSize)
{
}

void Generic_Serial_Device::DispatchMessage(Message* pmsg) {
	g_pPlutoLogger->Write(LV_STATUS, "Routing Message %d to %d...", pmsg->m_dwID, pmsg->m_dwPK_Device_To);

	RubyIOManager* pmanager = RubyIOManager::getInstance();
	DeviceData_Base *pDeviceData_Base = 
			m_pData->m_AllDevices.m_mapDeviceData_Base_Find(pmsg->m_dwPK_Device_To);
	pmanager->RouteMessage(pDeviceData_Base, pmsg);
}

void Generic_Serial_Device::RunThread() {
	RubyIOManager* pmanager = RubyIOManager::getInstance();
	ParseDeviceHierarchy(m_pData);
	
	pmanager->Run(false);
	Generic_Serial_Device_Command::RunThread();
	pmanager->Wait(true);
    g_pPlutoLogger->Write(LV_STATUS, "Generic Serial Device RunThread ended.");
}

void Generic_Serial_Device::ParseDeviceHierarchy(DeviceData_Impl *pdevdata) {
	g_pPlutoLogger->Write(LV_STATUS, "Device %d has commad line <%s>.", pdevdata->m_dwPK_Device, pdevdata->m_sCommandLine.c_str());
	RubyIOManager* pmanager = RubyIOManager::getInstance();
	if(pdevdata->m_sCommandLine == GSD_COMMAND_LINE || pdevdata->m_bImplementsDCE) {
		if(pmanager->addDevice(this, pdevdata) >= 0) {
			VectDeviceData_Impl& vDeviceData = pdevdata->m_vectDeviceData_Impl_Children;
			for(VectDeviceData_Impl::size_type i = 0; i < vDeviceData.size(); i++) {
				if(!pmanager->hasDevice(vDeviceData[i])) {
					ParseDeviceHierarchy(vDeviceData[i]);
				}
			}
		}
	} else {
	    g_pPlutoLogger->Write(LV_STATUS, "Device %d has no GSD specified as command line. No ruby code will be executed.",
						pdevdata->m_dwPK_Device);
	}
}
