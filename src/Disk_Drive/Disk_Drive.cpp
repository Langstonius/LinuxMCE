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
#include "VFD_LCD/VFD_LCD_Base.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stropts.h>

#include <sstream>

#include <sys/wait.h>

extern "C"
{
    #include <linux/cdrom.h>
}
#include "pluto_main/Define_MediaType.h"

enum DiscTypes {
    DISCTYPE_NONE       = -1,
    DISCTYPE_CD_AUDIO   =  0,
    DISCTYPE_CD         =  1,
    DISCTYPE_DVD_VIDEO  =  2,
    DISCTYPE_DVD        =  3,
    DISCTYPE_BLANK      =  4,
    DISCTYPE_DATA       =  5,
    DISCTYPE_CD_MIXED   =  6,
    DISCTYPE_CD_VCD     =  7,
    DISCTYPE_CD_SVCD    =  8,
    DISCTYPE_DVD_AUDIO  =  9
};

#define SERVER_PORT 5150

//<-dceag-const-b->! custom
Disk_Drive::Disk_Drive(int DeviceID, string ServerAddress,bool bConnectEventHandler,bool bLocalMode,class Router *pRouter)
    : Disk_Drive_Command(DeviceID, ServerAddress,bConnectEventHandler,bLocalMode,pRouter),
        m_monitorEnabled(true), m_mediaInserted(false), m_mediaDiskStatus(DISCTYPE_NONE), m_serverPid(-1), m_serverPort(SERVER_PORT),
		m_DiskMutex("disk drive")
//<-dceag-const-e->
{
	m_DiskMutex.Init(NULL);
	m_pDevice_AppServer=NULL;
}

//<-dceag-getconfig-b->
bool Disk_Drive::GetConfig()
{
	if( !Disk_Drive_Command::GetConfig() )
		return false;
//<-dceag-getconfig-e->

	system("modprobe ide-generic");
	system("modprobe ide-cd");
	m_discid=0;  // No disc inserted
	m_sDrive=DATA_Get_Drive();
	if( m_sDrive=="" )
		m_sDrive="/dev/cdrom";
	m_pDevice_AppServer = m_pData->FindFirstRelatedDeviceOfTemplate(DEVICETEMPLATE_App_Server_CONST);

	return true;
}

//<-dceag-const2-b->!

//<-dceag-dest-b->
Disk_Drive::~Disk_Drive()
//<-dceag-dest-e->
{

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
void Disk_Drive::ReceivedCommandForChild(DeviceData_Base *pDeviceData_Base,string &sCMD_Result,Message *pMessage)
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


/*

    COMMANDS TO IMPLEMENT

*/

//<-dceag-c45-b->

	/** @brief COMMAND: #45 - Disk Drive Monitoring ON */
	/** Turn ON the disk Monitoring. */

void Disk_Drive::CMD_Disk_Drive_Monitoring_ON(string &sCMD_Result,Message *pMessage)
//<-dceag-c45-e->
{
    g_pPlutoLogger->Write(LV_ACTION,"Turning ON the disk drive monitoring.");
    m_monitorEnabled = true;
    cdrom_lock(0);
}

//<-dceag-c46-b->

	/** @brief COMMAND: #46 - Disk Drive Monitoring OFF */
	/** Turn OFF the disk Monitoring. */

void Disk_Drive::CMD_Disk_Drive_Monitoring_OFF(string &sCMD_Result,Message *pMessage)
//<-dceag-c46-e->
{
    g_pPlutoLogger->Write(LV_STATUS,"Turning OFF the Disk Drive Monitoring.");
    m_monitorEnabled = false;
    cdrom_lock(1);
}

//<-dceag-c47-b->

	/** @brief COMMAND: #47 - Reset Disk Drive */
	/** Reset the disk drive. */

void Disk_Drive::CMD_Reset_Disk_Drive(string &sCMD_Result,Message *pMessage)
//<-dceag-c47-e->
{
    m_mediaInserted = false;
    m_mediaDiskStatus = DISCTYPE_NONE;
	DisplayMessageOnOrbVFD("Checking disc...");

    internal_reset_drive(true);
}

//<-dceag-c48-b->

	/** @brief COMMAND: #48 - Eject Disk */
	/** Eject the disk from the drive. */

void Disk_Drive::CMD_Eject_Disk(string &sCMD_Result,Message *pMessage)
//<-dceag-c48-e->
{
    cout << "Need to implement command #48 - Eject Disk" << endl;
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
    g_pPlutoLogger->Write(LV_STATUS, "Got a mount media request %s", sFilename.c_str());
    string stringMRL;

	if ( ! mountDVD(sFilename, stringMRL) )
	{
		g_pPlutoLogger->Write(LV_WARNING, "Error executing the dvd mounting script (message: %s). Returning error.", stringMRL.c_str());
		sCMD_Result = "NOT_OK";
		*sMediaURL = stringMRL;
		return;
	}

	*sMediaURL += stringMRL;
	sCMD_Result = "OK";
	g_pPlutoLogger->Write(LV_STATUS, "Returning new media URL: %s", sMediaURL->c_str());
}

//<-dceag-c55-b->

	/** @brief COMMAND: #55 - Abort Ripping */
	/** Starts ripping a DVD. */

void Disk_Drive::CMD_Abort_Ripping(string &sCMD_Result,Message *pMessage)
//<-dceag-c55-e->
{
	if( !m_pDevice_AppServer )
	{
		g_pPlutoLogger->Write(LV_CRITICAL, "Cannot Abort rip -- no app server");
		sCMD_Result="NO App_Server";
		return;
	}

	g_pPlutoLogger->Write(LV_STATUS,"Aborting Ripping");
	DCE::CMD_Kill_Application
		CMD_Kill_Application(m_dwPK_Device,
						m_pDevice_AppServer->m_dwPK_Device,
						"rip_" + StringUtils::itos(m_dwPK_Device),true);

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
    internal_monitor_step(false); // ignore any drive that is in the drive at the start.

    bool done = false;
    while ( ! done  && !m_bQuit )
    {
        done = ! internal_monitor_step(true);
        sleep(3); // Sleep 3 seconds
    }
}

bool Disk_Drive::internal_monitor_step(bool bFireEvent)
{
//  const int max_bytes = 1024;
//  static char buf[2][max_bytes];
//  static int offset[2]={0,0};
//  int i;

//  struct timeval tv;
//  fd_set rfds;

//  tv.tv_sec = 0;
//  tv.tv_usec = 200000;
    if ( ! internal_reset_drive(bFireEvent) )
    {
        g_pPlutoLogger->Write(LV_STATUS, "Monitor drive returned false.");
        return false;
    }
/*
    if ( m_childOutFd == -1 || m_childErrFd == -1 )
    {
        struct timespec ts;
        ts.tv_sec = tv.tv_sec;
        ts.tv_nsec = tv.tv_usec * 1000;
        nanosleep( &ts, NULL );
        return true;
    }

    FD_ZERO( &rfds );
    FD_SET( m_childOutFd, &rfds );
    FD_SET( m_childErrFd, &rfds );

    time_t timeout=time(NULL) + 3;
    while ( select( m_childErrFd+1, &rfds, NULL, NULL, &tv ) > 0 && time(NULL)<timeout )
    {
        for ( i = 0; i < m_childErrFd+1; ++i )
        {
            if ( FD_ISSET( i, &rfds ) )
            {
                int buf_idx = (i == m_childOutFd ? 0 : 1);
                char *buffer = buf[buf_idx];
                int o = offset[buf_idx];
                char *c;

                if ( read( i, buffer + o, 1 ) > 0 )
                {
                    buffer[++o] = '\0';
                    //printf("got from child: %s\n",buffer);
                    if ( ( c = strchr( buffer, '\r' ) ) != NULL )
                    {
                        *c = '\0';
                        parseChildStatus( buffer );
                        o = 0;
                    }
                    else if ( ( c = strchr( buffer, '\n' ) ) != NULL )
                    {
                        *c = '\0';
                        parseChildStatus( buffer );
                        o = 0;
                    }
                    offset[buf_idx] = o;
                }
            }
        }
    }*/
    return true;
}

/**
 *  Internal helper functions.
 */
int Disk_Drive::cdrom_lock( int lock )
{
  FILE *fp;

  if( (fp = fopen("/proc/sys/dev/cdrom/lock","w")) != NULL ) {
    fprintf(fp,"%d",lock);
    fclose(fp);
    return 1;
  } else {
    return 0;
  }
}

int Disk_Drive::cdrom_checkdrive (const char *filename, int *flag, bool bFireEvent)
{
    int fd;
    int status;
    struct cdrom_tochdr th;

    // open the device
    fd = open (filename, O_RDONLY | O_NONBLOCK);
    if (fd < 0)
    {
        g_pPlutoLogger->Write(LV_WARNING, "Error: couldn't open %s.", filename);
        return -1;
    }

//     g_pPlutoLogger->Write(LV_STATUS, "Disk Drive %s file handle was opened!", DATA_Get_Drive().c_str());
    // read the drive status info
    status = ioctl (fd, CDROM_DRIVE_STATUS, CDSL_CURRENT);
//     g_pPlutoLogger->Write(LV_STATUS, "Current disk status %d", status);

    switch (status)
    {
        // if there's a ok disc in there
    case CDS_DISC_OK:
//         g_pPlutoLogger->Write(LV_WARNING, "Media inserted value here: %d", m_mediaInserted);
//         g_pPlutoLogger->Write(LV_WARNING, "Disk type value here: %d", *flag);

        if (*flag != DISCTYPE_NONE || m_mediaInserted )
            break;

		if( bFireEvent )
			DisplayMessageOnOrbVFD("Disc detected in drive.");
        g_pPlutoLogger->Write(LV_STATUS, "Got a disc. Sleep a sec, then reopen. One hack to allow the disk to spin I think.");
        close(fd);
        sleep(1);
        fd = open (filename, O_RDONLY | O_NONBLOCK);
        if (fd < 0)
        {
            g_pPlutoLogger->Write(LV_WARNING, "Error: couldn't open %s.", filename);
            return -1;
        }

        // read the drive status info
        status = ioctl (fd, CDROM_DRIVE_STATUS, CDSL_CURRENT);
        if( status!=CDS_DISC_OK )
        {
            g_pPlutoLogger->Write(LV_WARNING, "Disc was detected, but the status is no longer disc ok.");
			close(fd);
            return 0;  // Don't change anything, we'll try again later
        }

        // see if we can read the disc's table of contents (TOC).
        status = ioctl (fd, CDROMREADTOCHDR, &th);
        if (status != 0)
        {
            g_pPlutoLogger->Write(LV_STATUS, "Can't read disc TOC. The disc is either a blank, or has a broken TOC.");
            *flag = DISCTYPE_BLANK;
            break;
        }

        // read disc status info
        status = ioctl (fd, CDROM_DISC_STATUS, CDSL_CURRENT);

        switch (status)
        {
        case CDS_AUDIO:
            // found a audio cd
            g_pPlutoLogger->Write(LV_STATUS, "Detected audio cd!");
            *flag = DISCTYPE_CD_AUDIO;
            break;

        case CDS_DATA_1:
        case CDS_DATA_2:
            {
                // Try up to 10 seconds to see what the directories are
                int result=-1;
                time_t timeout=time(NULL) + 10;
                while( result==-1 && timeout>time(NULL) )
                {
                    result = cdrom_has_dir (fd, "video_ts");
                    if( result==-1 )
                    {
                        g_pPlutoLogger->Write(LV_STATUS, "Not ready to read the directories yet.");
                        sleep(1);
                    }
                    g_pPlutoLogger->Write(LV_STATUS, "Result of video_ts search: %d.", result);
                }

                if ( result > 0 )
                {
                    g_pPlutoLogger->Write(LV_STATUS, "I think it's a DVD...");
                    *flag =DISCTYPE_DVD_VIDEO;
                }
                else if ( cdrom_has_dir (fd, "vcd") > 0 )
                {
                    g_pPlutoLogger->Write(LV_STATUS, "I think it's a VCD...");
                    *flag = DISCTYPE_CD_VCD;
                }
                else if ( cdrom_has_dir (fd, "svcd") > 0 )
                {
                    g_pPlutoLogger->Write(LV_STATUS, "I think it's a SVCD...");
                    *flag = DISCTYPE_CD_SVCD;
                }
                else
                {
                    g_pPlutoLogger->Write(LV_STATUS, "Doesn't have any directories -- must be data.");
                    *flag = DISCTYPE_DATA;
                }
            }
            break;

        case CDS_MIXED:
            // found a mixed cd
            g_pPlutoLogger->Write(LV_STATUS, "Detected mixed audio/data cd!");
            *flag = DISCTYPE_CD_MIXED;
            break;

        default:
            g_pPlutoLogger->Write(LV_WARNING, "Could not determine disc type: Doing nothing!");
            break;
        }
        break;

    case CDS_NO_INFO:
        // drive doesnt support querying, so this program will never work on that drive.
        g_pPlutoLogger->Write(LV_WARNING, "%s does not support status queries.", filename);

    default:
        // g_pPlutoLogger->Write(LV_STATUS, "Nothing interesting hapened");
        // release the device
        *flag = DISCTYPE_NONE;
    }
    close (fd);
    return 0;
}




bool Disk_Drive::mountDVD(string fileName, string &strMediaUrl)
{
	// True if we're playing a physical dis, false if it's a file
	bool bDriveMount = StringUtils::StartsWith(fileName,"/dev/",true);

	string sDrive = bDriveMount ? fileName : m_sDrive;

	string cmd = "ln -sf " + sDrive + " /dev/dvd";
	g_pPlutoLogger->Write(LV_STATUS,"cmd drivemount: %d - %s",(int) bDriveMount,cmd.c_str());
	system(cmd.c_str());  // Don't care about the return.  Just making sure it's not loop 5 so we can delete it
	if( bDriveMount )
	{
g_pPlutoLogger->Write(LV_ACTION,"returning mounted drive");
		strMediaUrl = "dvd:/";
Sleep(500); // TODO: HACK  -- sometimes xine can't play dvd's.  Throw a small delay in to see if it has an effect
		return true;
	}

	string cmdUnmount = "losetup -d /dev/loop5";
	g_pPlutoLogger->Write(LV_STATUS,"cmd: %s",cmdUnmount.c_str());
	system(cmdUnmount.c_str());  // Don't care about the return. 

    cmd = "losetup /dev/loop5 \"" + fileName + "\"";
	g_pPlutoLogger->Write(LV_STATUS,"cmd: %s",cmd.c_str());
    int iResult2=0,iResult = system(cmd.c_str());
	
	if( iResult!=0 )
	{
		g_pPlutoLogger->Write(LV_WARNING,"first attempt to mount failed %s",cmd.c_str());
Sleep(1000);
		g_pPlutoLogger->Write(LV_STATUS,"cmd: %s",cmdUnmount.c_str());
		system(cmdUnmount.c_str());  // Don't care about the return. 
Sleep(500);
		iResult = system(cmd.c_str());
	}

	if( iResult==0 )
	{

Sleep(500); // TODO: HACK  -- sometimes xine can't play dvd's.  Throw a small delay in to see if it has an effect
		cmd = "ln -sf /dev/loop5 /dev/dvd";
		g_pPlutoLogger->Write(LV_STATUS,"cmd: %s",cmd.c_str());
		iResult2 = system(cmd.c_str());
	}

	if( iResult==0 && iResult2==0 )
	{
g_pPlutoLogger->Write(LV_ACTION,"returning mounted dvd");
		strMediaUrl = "dvd:/";
Sleep(500); // TODO: HACK  -- sometimes xine can't play dvd's.  Throw a small delay in to see if it has an effect
		return true;
	}

g_pPlutoLogger->Write(LV_CRITICAL,"Failed to mount %d %d",iResult,iResult2);
	cmd = "ln -sf " + sDrive + " /dev/dvd";
	g_pPlutoLogger->Write(LV_STATUS,"cmd: %s",cmd.c_str());
	system(cmd.c_str());  // Can't do anything if it fails

	return false;
}

int Disk_Drive::cdrom_has_dir (int fd, const char *directory)
{

    int ret = 0;        // return value
    unsigned short bs;  // the discs block size
    unsigned short ts;  // the path table size
    unsigned int tl;    // the path table location (in blocks)
    unsigned int i;
    unsigned int len_di = 0;    // length of the directory name in current path table entry
    unsigned int parent = 0;    // the number of the parent directory's path table entry
    char *dirname = NULL;   // filename for the current path table entry
    int pos = 0;        // our position into the path table
    int curr_record = 1;    // the path table record we're on

    // read the block size
    lseek (fd, 0x8080, SEEK_SET);
    int iFirstRead = read (fd, &bs, 2);

    g_pPlutoLogger->Write(LV_STATUS, "Looking for folder: %s.", directory);

    g_pPlutoLogger->Write(LV_STATUS, "FirstRead: %d fd: %d  directory: %s", iFirstRead, fd, directory);
    if( iFirstRead < 0 )
    {
        g_pPlutoLogger->Write(LV_STATUS, "Try again - Cannot read from drive.");
        return -1;
    }

    // read in size of path table
    lseek (fd, 2, SEEK_CUR);
    read (fd, &ts, 2);

    // read in which block path table is in
    lseek (fd, 6, SEEK_CUR);
    read (fd, &tl, 4);

    // seek to the path table
    lseek (fd, ((int) (bs) * tl), SEEK_SET);

    g_pPlutoLogger->Write(LV_STATUS, "Ready to loop through ts: %d.",ts);

    // loop through the path table entries
    while (pos < ts)
    {
        // get the length of the filename of the current entry
        int iRead = read (fd, &len_di, 1);

        // get the record number of this entry's parent
        // i'm pretty sure that the 1st entry is always the top directory
        int iSeek = lseek (fd, 5, SEEK_CUR);
        int iRead2 = read (fd, &parent, 2);

        // allocate and zero a string for the filename
        dirname = (char *) malloc (len_di + 1);
        for (i = 0; i < len_di + 1; i++)
            dirname[i] = '\0';

        // read the name
        int iRead3 = read (fd, dirname, len_di);

        g_pPlutoLogger->Write(LV_STATUS, "Directory: %s parent: %d (pos: %d  ts: %d read: %d read2: %d  read3: %d  seek: %d)",
                    dirname, parent,
                    pos, ts, iRead, iRead2, iRead3, iSeek);

        // if we found a folder that has the root as a parent, and the directory name matches
        // then return success
        if ((parent == 1) && (strcasecmp (dirname, directory) == 0))
        {
            g_pPlutoLogger->Write(LV_STATUS, "It's a match.");
            ret = 1;
            free (dirname);
            break;
        }

        free (dirname);

        // all path table entries are padded to be even, so if this is an odd-length table, seek a byte to fix it
        if (len_di % 2 == 1)
        {
            lseek (fd, 1, SEEK_CUR);
            pos++;
        }

        // update our position
        pos += 8 + len_di;
        curr_record++;
    }

    return ret;
}

bool Disk_Drive::internal_reset_drive(bool bFireEvent)
{
	PLUTO_SAFETY_LOCK(dm,m_DiskMutex);
    int status;
    string mrl = ""; //, serverMRL, title;

    int result = cdrom_checkdrive( m_sDrive.c_str(), &m_mediaDiskStatus, bFireEvent);

    //     g_pPlutoLogger->Write(LV_STATUS, "Disc Reset: checkdrive status: %d  result: %d", m_mediaDiskStatus, result);

    // we only care if a new CD was inserted in the meantime.
    if (result >= 0 && m_mediaDiskStatus != DISCTYPE_NONE && m_mediaInserted == false)
    {
        int fd = open( m_sDrive.c_str(), O_RDONLY | O_NONBLOCK );
        if (fd < 0)
        {
            g_pPlutoLogger->Write(LV_WARNING, "Failed to open device: %s", m_sDrive.c_str());
            // throw ("failed to open cdrom device" );
            return false;
        }

        mrl = m_sDrive;
        switch (m_mediaDiskStatus)
        {
            case DISCTYPE_CD_MIXED: // treat a mixed CD as an audio CD for now.
            case DISCTYPE_CD_AUDIO:
                mrl = getTracks("cdda:/").c_str();
                status = MEDIATYPE_pluto_CD_CONST;
                break;

            case DISCTYPE_DVD_VIDEO:
                mrl = m_sDrive;
                status = MEDIATYPE_pluto_DVD_CONST;
                break;

            case DISCTYPE_BLANK:
                status = MEDIATYPE_misc_BlankMedia_CONST;
                break;

            case DISCTYPE_CD_VCD:
                status = MEDIATYPE_pluto_StoredVideo_CONST;
                break;

            default:
                status = MEDIATYPE_pluto_CD_CONST;
                break;
        }
        close (fd);

        g_pPlutoLogger->Write(LV_WARNING, "Disc of type %d was detected", status, mrl.c_str());

//         if ( ! serverStarted() && (status == MEDIATYPE_pluto_DVD_CONST || status == MEDIATYPE_pluto_StoredAudio_CONST))
//         {
//             startServer(result);
//
//         }

        if ( bFireEvent )
        {
			m_discid=time(NULL);
			g_pPlutoLogger->Write(LV_WARNING, "One Media Inserted event fired (%s) m_discid: %d", mrl.c_str(),m_discid);
            EVENT_Media_Inserted(status, mrl,StringUtils::itos(m_discid),m_sDrive);
        }
        else
        {
            g_pPlutoLogger->Write(LV_WARNING, "Not firing the event");
        }

        m_mediaInserted = true;
//      GetEvents()->Media_Inserted(
//      mrl.c_str(), str_type, title.c_str(),genre.c_str());
/**
        Set_MRL_LOCAL (mrl.c_str());
        Set_MRL_EXTERNAL (serverMRL.c_str());
        Set_MEDIA_TYPE (str_type);
        Set_TITLE (title);
*/
    }

    // we mark media as not inserted on error or when the code tell us that no CD is in the unit.
    if (result < 0 || m_mediaDiskStatus == DISCTYPE_NONE )
    {
        if ( m_mediaInserted == true )
        {
			m_discid=0;
            m_mediaInserted = false;
            g_pPlutoLogger->Write(LV_STATUS, "Disk is not in the drive at the moment");
        }
    }

    return true;
//  cout << "Need to implement command #47 - Reset Disk Drive" << endl;
}

//<-dceag-sample-b->! no sample
//<-dceag-createinst-b->!
//<-dceag-c337-b->

	/** @brief COMMAND: #337 - Rip Disk */
	/** This will try to RIP a DVD to the HDD. */
		/** @param #17 PK_Users */
			/** The user who needs this rip in his private area. */
		/** @param #20 Format */
			/** wav, flac, ogg, etc. */
		/** @param #50 Name */
			/** The target disk name, or for cd's, a comma-delimited list of names for each track. */
		/** @param #121 Tracks */
			/** For CD's, this must be a comma-delimted list of tracks (1 based) to rip. */
		/** @param #131 EK_Disc */
			/** The ID of the disc to rip */

void Disk_Drive::CMD_Rip_Disk(int iPK_Users,string sFormat,string sName,string sTracks,int iEK_Disc,string &sCMD_Result,Message *pMessage)
//<-dceag-c337-e->
{
	if( !m_pDevice_AppServer )
	{
		g_pPlutoLogger->Write(LV_CRITICAL, "Cannot rip -- no appserver");
		sCMD_Result="NO App_Server";
		return;
	}
	g_pPlutoLogger->Write(LV_STATUS, "Going to rip %s", sName.c_str() );

	if ( m_isRipping )
	{
		EVENT_Ripping_Progress("",RIP_RESULT_ALREADY_RIPPING, "", sName, iEK_Disc);
		return;
	}

	if ( ! m_mediaInserted )
	{
		EVENT_Ripping_Progress("",RIP_RESULT_NO_DISC, "", sName, iEK_Disc);
		return;
	}

	if ( m_mediaDiskStatus != DISCTYPE_CD_AUDIO && m_mediaDiskStatus != DISCTYPE_DVD_VIDEO && m_mediaDiskStatus != DISCTYPE_CD_MIXED && m_mediaDiskStatus != DISCTYPE_CD_VCD )
	{
		EVENT_Ripping_Progress("", RIP_RESULT_INVALID_DISC_TYPE, "", sName, iEK_Disc);
		return;
	}

	string strParameters, strCommOnFailure, strCommOnSuccess;

	strParameters = StringUtils::Format("%d\t%d\t%s\t%s\t%d\t%d\t%s\t%s",
			m_dwPK_Device,
			pMessage->m_dwPK_Device_From,
			sName.c_str(),
			m_sDrive.c_str(),
			m_mediaDiskStatus, iPK_Users,
			sFormat.c_str(),
			sTracks.c_str());

	g_pPlutoLogger->Write(LV_STATUS, "Launching ripping job2 with name \"%s\" for disk with type \"%d\" parms %s", sName.c_str(), m_mediaDiskStatus, strParameters.c_str() );

	string sResultMessage =
		StringUtils::itos(m_dwPK_Device) + " -1001 " + StringUtils::itos(MESSAGETYPE_EVENT) + 
			" " + StringUtils::itos(EVENT_Ripping_Progress_CONST) + " " + StringUtils::itos(EVENTPARAMETER_EK_Disc_CONST) + " " + StringUtils::itos(iEK_Disc) +
			" " + StringUtils::itos(EVENTPARAMETER_Name_CONST) + " \"" +
			sName + "\" " + StringUtils::itos(EVENTPARAMETER_Result_CONST) + " ";

	DCE::CMD_Spawn_Application
		spawnApplication(m_dwPK_Device,
						m_pDevice_AppServer->m_dwPK_Device,
						"/usr/pluto/bin/ripDiskWrapper.sh", "rip_" + StringUtils::itos(m_dwPK_Device), strParameters,
						sResultMessage + StringUtils::itos(RIP_RESULT_FAILURE),
						sResultMessage + StringUtils::itos(RIP_RESULT_SUCCESS),
						false,false,false);

	string sResponse;
    if( !SendCommand(spawnApplication,&sResponse) || sResponse!="OK" )
	{
		g_pPlutoLogger->Write(LV_CRITICAL, "Trying to rip - App server returned %s",sResponse.c_str());
		sCMD_Result = "App_Server: " + sResponse;
		return;
	}
}

string Disk_Drive::getTracks (string mrl)
{
    g_pPlutoLogger->Write(LV_STATUS, "Finding CD tracks.");

//     time_t startTime=time(NULL);

    int fd = -1, status;
    string tracks = "";

    try {
        g_pPlutoLogger->Write(LV_STATUS, "Opening drive first time.");

        fd = open( m_sDrive.c_str(), O_RDONLY | O_NONBLOCK );
        if (fd < 0)
            throw string ("Failed to open CD device" + string (strerror (errno)));

        g_pPlutoLogger->Write(LV_STATUS, "Checking media with ioctl");
        status = ioctl( fd, CDROM_DISC_STATUS, CDSL_CURRENT );
        if ( status != CDS_AUDIO && status != CDS_MIXED )
            throw string ("Invalid media detected");

		/* Code inspired from cd-discid - Start */
		struct cdrom_tochdr hdr;
		if (ioctl(fd, CDROMREADTOCHDR, &hdr) < 0)
		{
			throw string("Failed to read CDROM TOC.");
		}
		/* Code inspired from cd-discid - End */

		ostringstream sb;
		for (int i = 1; i <= hdr.cdth_trk1; i++)
		{
			sb << mrl << i << endl;
		}

		tracks = sb.str();
    }
    catch (string err)
    {
        g_pPlutoLogger->Write (LV_WARNING, "w1: %s",err.c_str());
    }
    catch (...)
    {
        g_pPlutoLogger->Write (LV_WARNING, "Unknown error in getTracks()");
    }

	close(fd);
    return tracks;
}

void Disk_Drive::DisplayMessageOnOrbVFD(string sMessage)
{
	DeviceData_Base *pDevice_OSD = m_pData->FindFirstRelatedDeviceOfCategory(DEVICECATEGORY_Orbiter_CONST);
	DeviceData_Base *pDevice_VFD = m_pData->FindFirstRelatedDeviceOfCategory(DEVICECATEGORY_LCDVFD_Displays_CONST);

	g_pPlutoLogger->Write(LV_STATUS,"Displaying on OSD: %d VFD: %d %s",
		(pDevice_OSD ? pDevice_OSD->m_dwPK_Device : 0),
		(pDevice_VFD ? pDevice_VFD->m_dwPK_Device : 0),
		sMessage.c_str());

	string sResponse; // Get Return confirmation so we know the message gets through before continuing
	if( pDevice_OSD )
	{
		DCE::CMD_Display_Message_DT CMD_Display_Message_DT(m_dwPK_Device,DEVICETEMPLATE_Orbiter_Plugin_CONST,BL_SameHouse,
			sMessage,"","","5",StringUtils::itos(pDevice_OSD->m_dwPK_Device));
		SendCommand(CMD_Display_Message_DT,&sResponse);
	}
	if( pDevice_VFD )
	{
		DCE::CMD_Display_Message CMD_Display_Message(m_dwPK_Device,pDevice_VFD->m_dwPK_Device,
			sMessage,
			StringUtils::itos(VL_MSGTYPE_RUNTIME_NOTICES),"app serve","5","");
		SendCommand(CMD_Display_Message,&sResponse);
	}
}
