//<-dceag-d-b->
#include "Photo_Screen_Saver.h"
#include "DCE/Logger.h"
#include "PlutoUtils/FileUtils.h"
#include "PlutoUtils/StringUtils.h"
#include "PlutoUtils/Other.h"

#include <iostream>
using namespace std;
using namespace DCE;

#include "Gen_Devices/AllCommandsRequests.h"
//<-dceag-d-e->
#include "Gallery.h"
#include "../pluto_main/Define_DeviceData.h"

#ifndef WIN32
#include "../utilities/linux/window_manager/WMController/WMController.h"
#endif

//<-dceag-const-b->
// The primary constructor when the class is created as a stand-alone device
Photo_Screen_Saver::Photo_Screen_Saver(int DeviceID, string ServerAddress,bool bConnectEventHandler,bool bLocalMode,class Router *pRouter)
	: Photo_Screen_Saver_Command(DeviceID, ServerAddress,bConnectEventHandler,bLocalMode,pRouter)
//<-dceag-const-e->
	, ThreadID(0), m_sPSSWindow("Photo_Screen_Saver")
{
}

//<-dceag-const2-b->
// The constructor when the class is created as an embedded instance within another stand-alone device
Photo_Screen_Saver::Photo_Screen_Saver(Command_Impl *pPrimaryDeviceCommand, DeviceData_Impl *pData, Event_Impl *pEvent, Router *pRouter)
	: Photo_Screen_Saver_Command(pPrimaryDeviceCommand, pData, pEvent, pRouter)
//<-dceag-const2-e->
{
}

//<-dceag-dest-b->
Photo_Screen_Saver::~Photo_Screen_Saver()
//<-dceag-dest-e->
{
	Terminate();
}

//<-dceag-getconfig-b->
bool Photo_Screen_Saver::GetConfig()
{
	if( !Photo_Screen_Saver_Command::GetConfig() )
		return false;
//<-dceag-getconfig-e->
	
	m_bIsOn=false;
	CMD_Reload();

	return true;
}

//<-dceag-reg-b->
// This function will only be used if this device is loaded into the DCE Router's memory space as a plug-in.  Otherwise Connect() will be called from the main()
bool Photo_Screen_Saver::Register()
//<-dceag-reg-e->
{
	return Connect(PK_DeviceTemplate_get()); 
}

/*  Since several parents can share the same child class, and each has it's own implementation, the base class in Gen_Devices
	cannot include the actual implementation.  Instead there's an extern function declared, and the actual new exists here.  You 
	can safely remove this block (put a ! after the dceag-createinst-b block) if this device is not embedded within other devices. */
//<-dceag-createinst-b->
Photo_Screen_Saver_Command *Create_Photo_Screen_Saver(Command_Impl *pPrimaryDeviceCommand, DeviceData_Impl *pData, Event_Impl *pEvent, Router *pRouter)
{
	return new Photo_Screen_Saver(pPrimaryDeviceCommand, pData, pEvent, pRouter);
}
//<-dceag-createinst-e->

/*
	When you receive commands that are destined to one of your children,
	then if that child implements DCE then there will already be a separate class
	created for the child that will get the message.  If the child does not, then you will 
	get all	commands for your children in ReceivedCommandForChild, where 
	pDeviceData_Base is the child device.  If you handle the message, you 
	should change the sCMD_Result to OK
*/
//<-dceag-cmdch-b->
void Photo_Screen_Saver::ReceivedCommandForChild(DeviceData_Impl *pDeviceData_Impl,string &sCMD_Result,Message *pMessage)
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
void Photo_Screen_Saver::ReceivedUnknownCommand(string &sCMD_Result,Message *pMessage)
//<-dceag-cmduk-e->
{
	sCMD_Result = "UNKNOWN DEVICE";
}

//<-dceag-sample-b->!

/*

	COMMANDS TO IMPLEMENT

*/

class GallerySetup
{
	int Width_, Height_;
	string m_sImages;
	int ZoomTime, FaddingTime;
	bool m_bUseAnimation;

public:
	pthread_t* ThreadID;
	Photo_Screen_Saver *m_pPhoto_Screen_Saver;
	
	GallerySetup(string Width, string Height, bool bUseAnimation, int ZoomTime, int FaddingTime, string ImageList, bool bNPOTTextures,Photo_Screen_Saver *pPhoto_Screen_Saver);
	int GetWidth();
	int GetHeight();
	int GetZoomTime();
	int GetFaddingTime();
	string GetSearchImagesPath();
	bool GetUseAnimation();
};

GallerySetup::GallerySetup(string Width, string Height, bool bUseAnimation, int ZoomTime, int FaddingTime, string ImageList, bool bNPOTTextures,Photo_Screen_Saver *pPhoto_Screen_Saver)
{
	this->Width_ = atoi(Width.c_str());
	this->Height_ = atoi(Height.c_str());
	m_sImages = ImageList;
	this->ZoomTime = ZoomTime;
	this->FaddingTime = FaddingTime;
	m_bUseAnimation = bUseAnimation;

	TextureManager::Instance()->SetupNPOTSupport(bNPOTTextures);
	m_pPhoto_Screen_Saver=pPhoto_Screen_Saver;
}

int GallerySetup::GetWidth()
{
	return this->Width_;
}
int GallerySetup::GetHeight()
{
	return Height_;
}

string GallerySetup::GetSearchImagesPath()
{
	return m_sImages;
}

int GallerySetup::GetZoomTime()
{
	return ZoomTime;
}

int GallerySetup::GetFaddingTime()
{
	return FaddingTime;
}

bool GallerySetup::GetUseAnimation()
{
	return m_bUseAnimation;
}

//<-dceag-c192-b->

	/** @brief COMMAND: #192 - On */
	/** Turn on the device */
		/** @param #97 PK_Pipe */
			/** Normally when a device is turned on all the inputs and outputs are selected automatically.  If this parameter is specified, only the settings along this pipe will be set. */
		/** @param #98 PK_Device_Pipes */
			/** Normally when a device is turned on the corresponding "pipes" are enabled by default. if this parameter is blank.  If this parameter is 0, no pipes will be enabled.  This can also be a comma seperated list of devices, meaning only the pipes to those devic */

void Photo_Screen_Saver::CMD_On(int iPK_Pipe,string sPK_Device_Pipes,string &sCMD_Result,Message *pMessage)
//<-dceag-c192-e->
{
	string w = m_pEvent->GetDeviceDataFromDatabase(m_pData->m_dwPK_Device_ControlledVia, DEVICEDATA_ScreenWidth_CONST);
	string h = m_pEvent->GetDeviceDataFromDatabase(m_pData->m_dwPK_Device_ControlledVia, DEVICEDATA_ScreenHeight_CONST);
	bool bNPOTTextures = DATA_Get_Supports_NPOT_Textures();
	string sUseAnimation = m_pEvent->GetDeviceDataFromDatabase(m_pData->m_dwPK_Device_ControlledVia, DEVICEDATA_Use_OpenGL_effects_CONST);
	bool bUseAnimation = sUseAnimation == "1";
	
	GallerySetup* SetupInfo = new GallerySetup(w, h, bUseAnimation, DATA_Get_ZoomTime(), DATA_Get_FadeTime(), m_sFileList, bNPOTTextures, this);
	if(0 == ThreadID)
	{
		SetupInfo->ThreadID = &ThreadID;
		Gallery::Instance()->m_bQuit_set(false);
		pthread_create(&ThreadID, NULL, ThreadAnimation, SetupInfo);
	}

	Gallery::Instance()->Resume();
	m_bIsOn=true;

#ifndef WIN32
	WMControllerImpl wmctrl;
	wmctrl.SetVisible(m_sPSSWindow, true);
#endif
}

//<-dceag-c193-b->

	/** @brief COMMAND: #193 - Off */
	/** Turn off the device */
		/** @param #97 PK_Pipe */
			/** Normally when a device is turned on all the inputs and outputs are selected automatically.  If this parameter is specified, only the settings along this pipe will be set. */

void Photo_Screen_Saver::CMD_Off(int iPK_Pipe,string &sCMD_Result,Message *pMessage)
//<-dceag-c193-e->
{
	Gallery::Instance()->Pause();
	m_bIsOn=false;

#ifndef WIN32
	WMControllerImpl wmctrl;
	wmctrl.SetVisible(m_sPSSWindow, false);
#endif
}

void Photo_Screen_Saver::Terminate()
{
	g_pPlutoLogger->Write(LV_WARNING, "Received CMD_Off. Terminating...");
	WM_Event Event;
	Event.Quit();
	Gallery::Instance()->m_bQuit_set(true);
	if( ThreadID )
		pthread_join(ThreadID, NULL);
	ThreadID = 0;
}

/*virtual*/ void Photo_Screen_Saver::OnReload()
{
	Terminate();
	Command_Impl::OnReload();
}

void* ThreadAnimation(void* ThreadInfo)
{
	g_pPlutoLogger->Write(LV_WARNING, "Start Gallery thread");
	GallerySetup* Info = (GallerySetup*) ThreadInfo;
	Gallery::Instance()->Setup(Info->GetWidth(), Info->GetHeight(), 
		Info->GetFaddingTime(),
		Info->GetZoomTime(),
		Info->GetSearchImagesPath(),
		Info->GetUseAnimation());
	Gallery::Instance()->MainLoop(Info->m_pPhoto_Screen_Saver); 
	Gallery::Instance()->CleanUp();

	*(Info->ThreadID) = 0;
	delete Info;
	g_pPlutoLogger->Write(LV_WARNING, "Finish Gallery thread");
	return NULL;
}

//<-dceag-c63-b->

	/** @brief COMMAND: #63 - Skip Fwd - Channel/Track Greater */
	/** Go forward in the list */

void Photo_Screen_Saver::CMD_Skip_Fwd_ChannelTrack_Greater(string &sCMD_Result,Message *pMessage)
//<-dceag-c63-e->
{
	// TODO -- make it go through the list
}

//<-dceag-c64-b->

	/** @brief COMMAND: #64 - Skip Back - Channel/Track Lower */
	/** Go back in the list */

void Photo_Screen_Saver::CMD_Skip_Back_ChannelTrack_Lower(string &sCMD_Result,Message *pMessage)
//<-dceag-c64-e->
{
	// TODO -- make it go through the list
}

//<-dceag-c606-b->

	/** @brief COMMAND: #606 - Reload */
	/** Reloads the list of screen saver files */

void Photo_Screen_Saver::CMD_Reload(string &sCMD_Result,Message *pMessage)
//<-dceag-c606-e->
{
	DCE::CMD_Get_Screen_Saver_Files_DT CMD_Get_Screen_Saver_Files_DT(m_dwPK_Device,DEVICETEMPLATE_Orbiter_Plugin_CONST,BL_SameHouse,m_pData->m_dwPK_Device_ControlledVia,&m_sFileList);
	SendCommand(CMD_Get_Screen_Saver_Files_DT);
	if( m_bIsOn )
	{
		// Restart it so it gets the new list
		CMD_Off(0);
		CMD_On(0,"");
	}
}
