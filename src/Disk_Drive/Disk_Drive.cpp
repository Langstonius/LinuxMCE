/*
 Disk_Drive

 Copyright (C) 2004 Pluto, Inc., a Florida Corporation

 www.plutohome.com

 Phone: +1 (877) 758-8648

 This program is free software; you can redistribute it
 and/or modify it under the terms of the GNU General Public License.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty
 of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 See the GNU General Public License for more details.
*/

//<-dceag-d-b->
#include "Disk_Drive.h"
#include "DCE/Logger.h"
#include "PlutoUtils/FileUtils.h"
#include "PlutoUtils/StringUtils.h"
#include "PlutoUtils/Other.h"

#include <iostream>
using namespace std;
using namespace DCE;

#include "Gen_Devices/AllCommandsRequests.h"
//<-dceag-d-e->

#include "pluto_main/Define_DeviceTemplate.h"
#include "pluto_main/Define_Event.h"
#include "pluto_main/Define_EventParameter.h"
#include "pluto_media/Database_pluto_media.h"
#include "PlutoUtils/ProcessUtils.h"
#include "Media_Plugin/MediaAttributes_LowLevel.h"
#include "JobHandler/Job.h"
#include "DCE/DCEConfig.h"
DCEConfig g_DCEConfig;

#include "Disk_Drive_Functions/RipTask.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef WIN32
#include <stropts.h>
#include <sys/wait.h>
extern "C"
{
    #include <linux/cdrom.h>
}
#endif

using namespace nsJobHandler;

#define SERVER_PORT 5150

//<-dceag-const-b->! custom
Disk_Drive::Disk_Drive(int DeviceID, string ServerAddress,bool bConnectEventHandler,bool bLocalMode,class Router *pRouter)
    : Disk_Drive_Command(DeviceID, ServerAddress,bConnectEventHandler,bLocalMode,pRouter),
        m_monitorEnabled(true), m_serverPid(-1), m_serverPort(SERVER_PORT)
//<-dceag-const-e->
{
	m_pJobHandler = new JobHandler();
	m_bAskBeforeReload=true;
	m_bImplementsPendingTasks=true;
}

//<-dceag-getconfig-b->
bool Disk_Drive::GetConfig()
{
	if( !Disk_Drive_Command::GetConfig() )
		return false;
//<-dceag-getconfig-e->
	string sDrive;
	
	system("modprobe ide-generic");
	system("modprobe ide-cd");
	sDrive = DATA_Get_Drive();
	if (sDrive == "")
	{
		sDrive = "/dev/cdrom";
		if (!FileUtils::FileExists(sDrive))
			sDrive = "/dev/cdrom1";
	}
	
	m_pDatabase_pluto_media = new Database_pluto_media(LoggerWrapper::GetInstance());
	string sIP = g_DCEConfig.m_mapParameters_Find("MySqlHost");
	if( sIP.empty() )
		sIP = m_sIPAddress;

	if( !m_pDatabase_pluto_media->Connect(sIP,"root","","pluto_media") )
	{
		LoggerWrapper::GetInstance()->Write(LV_CRITICAL,"Disk_Drive::GetConfig cannot connect to database on %s", sIP.c_str());
		delete m_pDatabase_pluto_media;
		m_pDatabase_pluto_media=NULL;
	}
	
	m_pMediaAttributes_LowLevel = new MediaAttributes_LowLevel(m_pDatabase_pluto_media,g_DCEConfig.m_iPK_Installation);

	m_pDisk_Drive_Functions = new Disk_Drive_Functions(this, sDrive, m_pJobHandler,m_pDatabase_pluto_media,m_pMediaAttributes_LowLevel);

	m_pJobHandler->StartThread();

/*
	// Quick and dirty, get nbd-server working
	string sNbdServer;
	sNbdServer += "NBD_PORT[0]=" + StringUtils::itos(m_dwPK_Device+18000) + "\n";
	sNbdServer += "NBD_FILE[0]=" + sDrive + "\n";
	sNbdServer += "NBD_SERVER_OPTS[0]=-r\n";

	string sFileName = "/etc/nbd-server";
	size_t Size;
	char *pPtr = FileUtils::ReadFileIntoBuffer( sFileName, Size );
	if( !pPtr || sNbdServer!=pPtr )
	{
		bool bResult = FileUtils::WriteBufferIntoFile( sFileName, sNbdServer.c_str(), sNbdServer.size() );
		LoggerWrapper::GetInstance()->Write(LV_WARNING,"Wrote nbd-server file %d chaged from %s to %s",
			(int) bResult,(pPtr ? pPtr : "*NONE*\n"),sNbdServer.c_str());
	}

	delete pPtr;
*/

	return true;
}

//<-dceag-const2-b->!

//<-dceag-dest-b->
Disk_Drive::~Disk_Drive()
//<-dceag-dest-e->
{
	delete m_pJobHandler;  // Delete this first since it can reference the others
	delete m_pDatabase_pluto_media;
	delete m_pMediaAttributes_LowLevel;
	delete m_pDisk_Drive_Functions;
}

//<-dceag-reg-b->
// This function will only be used if this device is loaded into the DCE Router's memory space as a plug-in.  Otherwise Connect() will be called from the main()
bool Disk_Drive::Register()
//<-dceag-reg-e->
{
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
void Disk_Drive::ReceivedCommandForChild(DeviceData_Impl *pDeviceData_Impl,string &sCMD_Result,Message *pMessage)
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
void Disk_Drive::ReceivedUnknownCommand(string &sCMD_Result,Message *pMessage)
//<-dceag-cmduk-e->
{
	sCMD_Result = "UNKNOWN DEVICE";
}

void Disk_Drive::PostConnect()
{
	DCE::CMD_Refresh_List_of_Online_Devices_Cat CMD_Refresh_List_of_Online_Devices_Cat(m_dwPK_Device,DEVICECATEGORY_Media_Plugins_CONST,
		true,BL_SameHouse);
	SendCommand(CMD_Refresh_List_of_Online_Devices_Cat);
}

/*

    COMMANDS TO IMPLEMENT

*/

//<-dceag-c45-b->

	/** @brief COMMAND: #45 - Disk Drive Monitoring ON */
	/** Turn ON the disk Monitoring. */

void Disk_Drive::CMD_Disk_Drive_Monitoring_ON(string &sCMD_Result,Message *pMessage)
//<-dceag-c45-e->
{
    LoggerWrapper::GetInstance()->Write(LV_ACTION,"Turning ON the disk drive monitoring.");
    m_monitorEnabled = true;
    m_pDisk_Drive_Functions->cdrom_lock(0);
}

//<-dceag-c46-b->

	/** @brief COMMAND: #46 - Disk Drive Monitoring OFF */
	/** Turn OFF the disk Monitoring. */

void Disk_Drive::CMD_Disk_Drive_Monitoring_OFF(string &sCMD_Result,Message *pMessage)
//<-dceag-c46-e->
{
    LoggerWrapper::GetInstance()->Write(LV_STATUS,"Turning OFF the Disk Drive Monitoring.");
    m_monitorEnabled = false;
    m_pDisk_Drive_Functions->cdrom_lock(1);
}

//<-dceag-c47-b->

	/** @brief COMMAND: #47 - Reset Disk Drive */
	/** Reset the disk drive. */
		/** @param #152 Drive Number */
			/** Disc unit index number
Disk_Drive: 0
Powerfile: 0, 1, ... */

void Disk_Drive::CMD_Reset_Disk_Drive(int iDrive_Number,string &sCMD_Result,Message *pMessage)
//<-dceag-c47-e->
{
	PLUTO_SAFETY_LOCK(dm,m_pDisk_Drive_Functions->m_DiskMutex);
	m_pDisk_Drive_Functions->m_mediaInserted = false;
	m_pDisk_Drive_Functions->m_mediaDiskStatus = DISCTYPE_NONE;
	m_pDisk_Drive_Functions->DisplayMessageOnOrbVFD("Checking disc...");
	
	m_pDisk_Drive_Functions->internal_reset_drive(true);
}

//<-dceag-c48-b->

	/** @brief COMMAND: #48 - Eject Disk */
	/** Eject the disk from the drive. */
		/** @param #152 Drive Number */
			/** Disc unit index number
Disk_Drive: 0
Powerfile: 0, 1, ... */

void Disk_Drive::CMD_Eject_Disk(int iDrive_Number,string &sCMD_Result,Message *pMessage)
//<-dceag-c48-e->
{
	static time_t tLastEject=0;

	LoggerWrapper::GetInstance()->Write(LV_STATUS,"Disk_Drive::CMD_Eject_Disk  tLastEject %d (%d) tray open: %d",
		(int) tLastEject, (int) time(NULL), (int) m_pDisk_Drive_Functions->m_bTrayOpen);

	if( time(NULL)-tLastEject<=2 )  // It can take the drive a while to spin down and the user hits eject multiple times
	{
		LoggerWrapper::GetInstance()->Write(LV_STATUS,"Disk_Drive::CMD_Eject_Disk skipping eject within last 2 seconds");
		return;
	}
	if( m_pDisk_Drive_Functions->m_bTrayOpen )
	{
		m_pDisk_Drive_Functions->DisplayMessageOnOrbVFD("Closing tray...");
	    system("eject -t");
	}
	else
	{
		m_pDisk_Drive_Functions->DisplayMessageOnOrbVFD("Opening tray...");
	    system("eject");
	}

	m_pDisk_Drive_Functions->m_mediaInserted = false;  // Be sure we re-identify any media in there
	m_pDisk_Drive_Functions->m_mediaDiskStatus = DISCTYPE_NONE;
	tLastEject = time(NULL); // Put this after the system call so we know when it's been less than 2 seconds since a successful one
}

//<-dceag-c49-b->

	/** @brief COMMAND: #49 - Start Burn Session */
	/** Initiates a new burning session. */

void Disk_Drive::CMD_Start_Burn_Session(string &sCMD_Result,Message *pMessage)
//<-dceag-c49-e->
{
    cout << "Need to implement command #49 - Start Burn Session" << endl;
}

//<-dceag-c50-b->

	/** @brief COMMAND: #50 - Start Ripping Session */
	/** Initiates a new ripping session. */

void Disk_Drive::CMD_Start_Ripping_Session(string &sCMD_Result,Message *pMessage)
//<-dceag-c50-e->
{
    cout << "Need to implement command #50 - Start Ripping Session" << endl;
}

//<-dceag-c51-b->

	/** @brief COMMAND: #51 - Add File To Burning Session */
	/** Add a new file to the initiated burning session. */

void Disk_Drive::CMD_Add_File_To_Burning_Session(string &sCMD_Result,Message *pMessage)
//<-dceag-c51-e->
{
    cout << "Need to implement command #51 - Add File To Burning Session" << endl;
}

//<-dceag-c52-b->

	/** @brief COMMAND: #52 - Start Burning */
	/** Starts burning. */

void Disk_Drive::CMD_Start_Burning(string &sCMD_Result,Message *pMessage)
//<-dceag-c52-e->
{
    cout << "Need to implement command #52 - Start Burning" << endl;
}

//<-dceag-c53-b->

	/** @brief COMMAND: #53 - Abort Burning */
	/** Aborts the burning session. */

void Disk_Drive::CMD_Abort_Burning(string &sCMD_Result,Message *pMessage)
//<-dceag-c53-e->
{
}

//<-dceag-c54-b->

	/** @brief COMMAND: #54 - Mount Disk Image */
	/** Will mount a disk image as a disk. */
		/** @param #13 Filename */
			/** What to mount. If it get's the Device name it will mount the actual disk in the drive. */
		/** @param #59 MediaURL */
			/** The URL which can be used to play the mounted media. */

void Disk_Drive::CMD_Mount_Disk_Image(string sFilename,string *sMediaURL,string &sCMD_Result,Message *pMessage)
//<-dceag-c54-e->
{
    LoggerWrapper::GetInstance()->Write(LV_STATUS, "Got a mount media request %s", sFilename.c_str());
    string stringMRL;

	if (! m_pDisk_Drive_Functions->mountDVD(sFilename, stringMRL))
	{
		LoggerWrapper::GetInstance()->Write(LV_WARNING, "Error executing the dvd mounting script (message: %s). Returning error.", stringMRL.c_str());
		sCMD_Result = "NOT_OK";
		*sMediaURL = stringMRL;
		return;
	}

	*sMediaURL += stringMRL;
	sCMD_Result = "OK";
	LoggerWrapper::GetInstance()->Write(LV_STATUS, "Returning new media URL: %s", sMediaURL->c_str());
}

//<-dceag-c55-b->

	/** @brief COMMAND: #55 - Abort Ripping */
	/** Aborts ripping a DVD. */

void Disk_Drive::CMD_Abort_Ripping(string &sCMD_Result,Message *pMessage)
//<-dceag-c55-e->
{
	if (! m_pDisk_Drive_Functions->m_pDevice_AppServer)
	{
		LoggerWrapper::GetInstance()->Write(LV_CRITICAL, "Cannot Abort rip -- no app server");
		sCMD_Result="NO App_Server";
		return;
	}

	LoggerWrapper::GetInstance()->Write(LV_STATUS,"Aborting Ripping");
	DCE::CMD_Kill_Application
		CMD_Kill_Application(m_dwPK_Device,
						m_pDisk_Drive_Functions->m_pDevice_AppServer->m_dwPK_Device,
						"rip_" + StringUtils::itos(m_dwPK_Device) + "_0",true);

    SendCommand(CMD_Kill_Application);
}

//<-dceag-c56-b->

	/** @brief COMMAND: #56 - Format Drive */
	/** Formats a disk. */

void Disk_Drive::CMD_Format_Drive(string &sCMD_Result,Message *pMessage)
//<-dceag-c56-e->
{
    cout << "Need to implement command #56 - Format Drive" << endl;
}

//<-dceag-c57-b->

	/** @brief COMMAND: #57 - Close Tray */
	/** Closes the tray. */

void Disk_Drive::CMD_Close_Tray(string &sCMD_Result,Message *pMessage)
//<-dceag-c57-e->
{
    cout << "Need to implement command #57 - Close Tray" << endl;
}


void Disk_Drive::RunMonitorLoop()
{
    m_pDisk_Drive_Functions->internal_monitor_step(false); // ignore any drive that is in the drive at the start.

    bool done = false;
    while ( ! done  && !m_bQuit_get())
    {
        done = ! m_pDisk_Drive_Functions->internal_monitor_step(true);
        Sleep(3000); // Sleep 3 seconds
    }
}

//<-dceag-sample-b->! no sample
//<-dceag-createinst-b->!
//<-dceag-c337-b->

	/** @brief COMMAND: #337 - Rip Disk */
	/** This will try to RIP a DVD to the HDD. */
		/** @param #13 Filename */
			/** The target disk name, or for cd's, a comma-delimited list of names for each track. */
		/** @param #17 PK_Users */
			/** The user who needs this rip in his private area. */
		/** @param #20 Format */
			/** wav, flac, ogg, etc. */
		/** @param #121 Tracks */
			/** For CD's, this must be a comma-delimted list of tracks (1 based) to rip. */
		/** @param #131 EK_Disc */
			/** The ID of the disc to rip.  If not specified this will be whatever disc is currently playing the entertainment area. */
		/** @param #151 Slot Number */
			/** The slot if this is a jukebox */
		/** @param #233 DriveID */
			/** The ID of the storage drive. Can be the ID of the core. */
		/** @param #234 Directory */
			/** The relative directory for the file to rip */

void Disk_Drive::CMD_Rip_Disk(string sFilename,int iPK_Users,string sFormat,string sTracks,int iEK_Disc,int iSlot_Number,int iDriveID,string sDirectory,string &sCMD_Result,Message *pMessage)
//<-dceag-c337-e->
{
	m_pDisk_Drive_Functions->CMD_Rip_Disk( sFilename,iPK_Users,sFormat,sTracks,iEK_Disc,iSlot_Number,iDriveID,sDirectory, sCMD_Result, pMessage);
Sleep(1000);
int k=2;
}
//<-dceag-c817-b->

	/** @brief COMMAND: #817 - Get Default Ripping Info */
	/** Get default ripping info: default filename, id and name of the storage device with most free space. */
		/** @param #13 Filename */
			/** Default ripping name. */
		/** @param #131 EK_Disc */
			/** The disc to rip.  If not specified, it will be whatever is playing in the entertainment area that sent this */
		/** @param #219 Path */
			/** Base path for ripping. */
		/** @param #233 DriveID */
			/** The id of the storage device with most free space. */
		/** @param #235 Storage Device Name */
			/** The name of the storage device with most free space. */

void Disk_Drive::CMD_Get_Default_Ripping_Info(int iEK_Disc,string *sFilename,string *sPath,int *iDriveID,string *sStorage_Device_Name,string &sCMD_Result,Message *pMessage)
//<-dceag-c817-e->
{
}
//<-dceag-c742-b->

	/** @brief COMMAND: #742 - Media Identified */
	/** Media has been identified */
		/** @param #2 PK_Device */
			/** The disk drive */
		/** @param #5 Value To Assign */
			/** The identified data */
		/** @param #10 ID */
			/** The ID of the disc */
		/** @param #19 Data */
			/** The picture/cover art */
		/** @param #20 Format */
			/** The format of the data */
		/** @param #29 PK_MediaType */
			/** The type of media */
		/** @param #59 MediaURL */
			/** The URL for the disc drive */
		/** @param #131 EK_Disc */
			/** If a disc was added accordingly, this reports the disc id */
		/** @param #193 URL */
			/** The URL for the picture */

void Disk_Drive::CMD_Media_Identified(int iPK_Device,string sValue_To_Assign,string sID,char *pData,int iData_Size,string sFormat,int iPK_MediaType,string sMediaURL,string sURL,int *iEK_Disc,string &sCMD_Result,Message *pMessage)
//<-dceag-c742-e->
{
	DCE::CMD_Media_Identified_DT CMD_Media_Identified_DT(m_dwPK_Device,DEVICETEMPLATE_Media_Plugin_CONST,
		BL_SameHouse,m_dwPK_Device,sValue_To_Assign,sID,pData,iData_Size,sFormat,iPK_MediaType,sMediaURL,sURL,iEK_Disc);
	SendCommand(CMD_Media_Identified_DT);

	LoggerWrapper::GetInstance()->Write(LV_STATUS,"Disk_Drive::CMD_Media_Identified disc is %d",*iEK_Disc);
	if( *iEK_Disc )
	{
		char cMediaType='M'; // The default
		if( iPK_MediaType==MEDIATYPE_pluto_CD_CONST )
			cMediaType='c';
		else if( iPK_MediaType==MEDIATYPE_pluto_DVD_CONST )
			cMediaType='d';
		m_pDisk_Drive_Functions->UpdateDiscLocation(cMediaType,*iEK_Disc,0);
	}
}
//<-dceag-c871-b->

	/** @brief COMMAND: #871 - Update Ripping Status */
	/** Update the status of a ripping job */
		/** @param #9 Text */
			/** A text message */
		/** @param #13 Filename */
			/** The filename being ripped */
		/** @param #102 Time */
			/** How much longer in seconds it will take */
		/** @param #199 Status */
			/** The status: [p] in progress, [e]rror, [s]uccess */
		/** @param #256 Percent */
			/** The percentage of completion */
		/** @param #257 Task */
			/** The task id */
		/** @param #258 Job */
			/** The job id */

void Disk_Drive::CMD_Update_Ripping_Status(string sText,string sFilename,string sTime,string sStatus,int iPercent,string sTask,string sJob,string &sCMD_Result,Message *pMessage)
//<-dceag-c871-e->
{
	PLUTO_SAFETY_LOCK(jm,*m_pJobHandler->m_ThreadMutex_get());
	Task *pTask = m_pJobHandler->FindTask(atoi(sJob.c_str()),atoi(sTask.c_str()));
	if( !pTask )
	{
		LoggerWrapper::GetInstance()->Write(LV_CRITICAL,"Disk_Drive::CMD_Update_Ripping_Status invalid job %s task %s",sJob.c_str(),sTask.c_str());
		return;
	}

	RipTask *pRipTask = (RipTask *) pTask;
	pRipTask->UpdateProgress(sStatus,iPercent,atoi(sTime.c_str()),sText,sFilename);
}
//<-dceag-c872-b->

	/** @brief COMMAND: #872 - Lock */
	/** Lock the drive for use by something else, normally the player */
		/** @param #2 PK_Device */
			/** The device requesting the lock */
		/** @param #9 Text */
			/** A description of the lock */
		/** @param #10 ID */
			/** The ID of what needs to be locked.  For a jukebox, this would be the slot. */
		/** @param #40 IsSuccessful */
			/** returns true if the lock was succesfull.  If not, it puts the current lock in Text */
		/** @param #252 Turn On */
			/** True to set the lock, false to release it */

void Disk_Drive::CMD_Lock(int iPK_Device,string sID,bool bTurn_On,string *sText,bool *bIsSuccessful,string &sCMD_Result,Message *pMessage)
//<-dceag-c872-e->
{
}

bool Disk_Drive::ReportPendingTasks(PendingTaskList *pPendingTaskList)
{
	return m_pJobHandler->ReportPendingTasks(pPendingTaskList);
}
//<-dceag-c882-b->

	/** @brief COMMAND: #882 - Abort Task */
	/** Abort a pending task */
		/** @param #248 Parameter ID */
			/** The ID of the task to abort */

void Disk_Drive::CMD_Abort_Task(int iParameter_ID,string &sCMD_Result,Message *pMessage)
//<-dceag-c882-e->
{
	PLUTO_SAFETY_LOCK(jm,*m_pJobHandler->m_ThreadMutex_get());
	Job *pJob = m_pJobHandler->FindJob(iParameter_ID);
	if( !pJob )
	{
		LoggerWrapper::GetInstance()->Write(LV_CRITICAL,"Disk_Drive::CMD_Abort_Task invalid job %d",iParameter_ID);
		return;
	}

	pJob->Abort();
}
