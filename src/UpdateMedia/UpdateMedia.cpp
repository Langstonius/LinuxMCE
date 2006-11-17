#include "PlutoUtils/FileUtils.h"
#include "PlutoUtils/StringUtils.h"
#include "PlutoUtils/Other.h"
#include "Logger.h"
#include "UpdateMedia.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <list>

#ifndef WIN32
#include <dirent.h>
#include <attr/attributes.h>
#endif

#include "pluto_main/Table_MediaType.h"
#include "pluto_main/Table_DeviceTemplate_MediaType.h"

#include "pluto_media/Table_File.h"
#include "pluto_media/Table_Picture_File.h"
#include "pluto_media/Table_Attribute.h"
#include "pluto_media/Table_Picture_Attribute.h"
#include "pluto_media/Table_File_Attribute.h"

#include "id3info/id3info.h"
#include "PlutoMediaFile.h"
#include "pluto_main/Table_Installation.h"
#include "pluto_main/Table_Device_DeviceData.h"
#include "pluto_main/Define_DeviceData.h"

#ifdef WIN32
#include <direct.h>
#include <conio.h>
#define chdir _chdir  // Why, Microsoft, why?
#define mkdir _mkdir  // Why, Microsoft, why?
#else
#endif


#define  VERSION "<=version=>"
//#define  USE_DEVEL_DATABASES 

using namespace std;
using namespace DCE;

#ifdef WIN32
	#include <signal.h>
#endif

namespace UpdateMediaSig
{
	static bool bSignalTrapCaught = false;
};

void sigtrap_hook(int sig)
{
	g_pPlutoLogger->Write(LV_CRITICAL, "Signal %d caught! Exiting...",  sig);
	UpdateMediaSig::bSignalTrapCaught = true;
}

UpdateMedia::UpdateMedia(string host, string user, string pass, int port,string sDirectory, bool bSyncFilesOnly)
{
	PlutoMediaFile::SetupSyncFilesOnly(bSyncFilesOnly);

#ifndef WIN32
	signal(SIGTRAP, &sigtrap_hook);
#endif

	ReadConfigFile();

    string sPlutoMediaDbName = "pluto_media";
    string sPlutoMainDbName = "pluto_main";

#ifdef USE_DEVEL_DATABASES
    sPlutoMediaDbName = "pluto_media_devel";
    sPlutoMainDbName = "pluto_main_devel";
#endif

    //connect to the databases
	m_pDatabase_pluto_media = new Database_pluto_media(g_pPlutoLogger);
	if( !m_pDatabase_pluto_media->Connect( host, user, pass, sPlutoMediaDbName, port ) )
	{
		g_pPlutoLogger->Write( LV_CRITICAL, "Cannot connect to database!" );
		return;
	}

	m_pDatabase_pluto_main = new Database_pluto_main(g_pPlutoLogger);
	if( !m_pDatabase_pluto_main->Connect( host, user, pass, sPlutoMainDbName, port ) )
	{
		g_pPlutoLogger->Write( LV_CRITICAL, "Cannot connect to database!" );
		return;
	}

	m_bAsDaemon = false;
    PlutoMediaIdentifier::Activate(m_pDatabase_pluto_main);

	m_sDirectory = StringUtils::Replace(&sDirectory,"\\","/");  // Be sure no Windows \'s
    FileUtils::ExcludeTrailingSlash(m_sDirectory);

	SetupInstallation();
}

UpdateMedia::UpdateMedia(Database_pluto_media *pDatabase_pluto_media, 
	Database_pluto_main *pDatabase_pluto_main, string sDirectory) 
{
#ifndef WIN32
	signal(SIGTRAP, &sigtrap_hook);
#endif

	ReadConfigFile();

    //reusing connections
    m_pDatabase_pluto_main = pDatabase_pluto_main;
    m_pDatabase_pluto_media = pDatabase_pluto_media;

	SetupInstallation();

	m_bAsDaemon = true;
	PlutoMediaIdentifier::Activate(m_pDatabase_pluto_main);

    m_sDirectory = StringUtils::Replace(&sDirectory,"\\","/");  // Be sure no Windows \'s
    FileUtils::ExcludeTrailingSlash(m_sDirectory);
}

void UpdateMedia::ReadConfigFile()
{
	ifstream fUpdateMediaConfig;
	string sConfFileValue;
	string::size_type sPosBeginIdx, sPosEndIdx;
	string sConfVar, sConfVal;

	sPosBeginIdx=0;
	sPosEndIdx=0;

	fUpdateMediaConfig.open("/etc/UpdateMedia.conf");
	if(fUpdateMediaConfig.is_open())
	{
		while(!fUpdateMediaConfig.eof())
		{
			getline(fUpdateMediaConfig, sConfFileValue);
			if(sConfFileValue.empty());
			else
			{
				sPosEndIdx=sConfFileValue.find_first_of("=");
				sConfVar=sConfFileValue.substr(sPosBeginIdx, sPosEndIdx);
				sPosBeginIdx = sPosEndIdx + 1;
				sPosEndIdx=sConfFileValue.length();
				sConfVal=sConfFileValue.substr(sPosBeginIdx, sPosEndIdx);

				if(sConfVar=="SyncId3Files")
				{
					if(sConfVal=="false" || sConfVal=="0")
					{
						PlutoMediaFile::SetupSyncId3Files(false);
					}	
				}
			}
		}
		fUpdateMediaConfig.close();
	}
}

void UpdateMedia::SetupInstallation()
{
	m_nPK_Installation = 0;
	vector<Row_Installation *> vectRow_Installation;
	m_pDatabase_pluto_main->Installation_get()->GetRows("1=1", &vectRow_Installation);
	if(vectRow_Installation.size() > 0)
		m_nPK_Installation = vectRow_Installation[0]->PK_Installation_get();
}

void UpdateMedia::DoIt()
{
	if( !m_pDatabase_pluto_media->m_bConnected )
	{
		cerr << "Cannot connect to database" << endl;
		exit(1);
	}

	ReadDirectory(m_sDirectory, !m_bAsDaemon);
    SyncDbWithDirectory(m_sDirectory); //mark missing/not-missing files, recursively
}

int UpdateMedia::ReadDirectory(string sDirectory, bool bRecursive)
{
	if( sDirectory.size()==0 )
		return 0;
	// Strip any trailing /
	if( sDirectory[ sDirectory.size()-1 ] == '/' )
		sDirectory = sDirectory.substr(0,sDirectory.size()-1);

	// Build a list of the files on disk, and a map of those in the database
	int PK_Picture=0;
	list<string> listFilesOnDisk;
	FileUtils::FindFiles(listFilesOnDisk,sDirectory,m_sExtensions,false,false,0,"");

	map<string,pair<Row_File *,bool> > mapFiles;

	vector<Row_File *> vectRow_File;
	m_pDatabase_pluto_media->File_get()->GetRows("Path='" + StringUtils::SQLEscape(sDirectory) + "'",
		&vectRow_File);
	for(size_t s=0;s<vectRow_File.size();++s)
	{
		Row_File *pRow_File = vectRow_File[s];
		mapFiles[pRow_File->Filename_get()] = 
			make_pair<Row_File *, bool>(pRow_File,false);
	}

	cout << "===============" << sDirectory << " disk: " << (int) listFilesOnDisk.size() << " db: " << (int) vectRow_File.size() << endl;

	// Now start matching them up
	for(list<string>::iterator it=listFilesOnDisk.begin();it!=listFilesOnDisk.end();++it)
	{
		if(m_bAsDaemon)
			Sleep(100);
			
		if(UpdateMediaSig::bSignalTrapCaught)
		{
			g_pPlutoLogger->Write(LV_WARNING, "SIGTERM received. Exiting...");
			return 0;
		}
		
		string sFile = *it;

		if(!FileUtils::FileExists(sDirectory + "/" + sFile)) //the file was just being deleted
		{
			g_pPlutoLogger->Write(LV_WARNING, "The file %s/%s was just being deleted. We'll skip it!",
				sDirectory.c_str(), sFile.c_str());
			continue;
		}

		//ignore id3 and lock files
		if(StringUtils::ToLower(FileUtils::FindExtension(sFile)) == "id3" || StringUtils::ToLower(FileUtils::FindExtension(sFile)) == "lock")
			continue;

		string sLockFile = sDirectory + "/" + sFile + ".lock";
		if(FileUtils::FileExists(sLockFile))
		{
			g_pPlutoLogger->Write(LV_WARNING, "Found lock file %s", sLockFile.c_str());

			time_t tFileTimestamp = FileUtils::GetLastModifiedDate(sLockFile);

			time_t tNow = time(NULL);
			// deleting locks if they are >2hours old
			if(tFileTimestamp == -1 || tNow - tFileTimestamp > 7200) //Media_Plugin has more the enough time to add the file in the database
			{
				g_pPlutoLogger->Write(LV_WARNING, "The lock file %s is old (%d seconds). Deleting it!", sLockFile.c_str(), tNow - tFileTimestamp);
				FileUtils::DelFile(sLockFile);
			}
			else
			{
				g_pPlutoLogger->Write(LV_WARNING, "The lock file %s is still active (%d seconds old). Ignoring the protected file!", sLockFile.c_str(), tNow - tFileTimestamp);
				continue;
			}
		}

        PlutoMediaFile PlutoMediaFile_(m_pDatabase_pluto_media, m_nPK_Installation,
            sDirectory, sFile);

		// Is it in the database?
		int PK_File=0;
		map<string,pair<Row_File *,bool> >::iterator itMapFiles = mapFiles.find( *it );
		if( itMapFiles==mapFiles.end() )
		{
			PK_File = PlutoMediaFile_.HandleFileNotInDatabase();
			if(!PK_File)
			{
				if(m_bAsDaemon)
					Sleep(10);
				
				continue; // Nothing to do
			}
		}	
		else
		{
			Row_File *pRow_File = itMapFiles->second.first;
			PK_File = pRow_File->PK_File_get();
cout << sFile << " exists in db as: " << PK_File << endl;
			itMapFiles->second.second = true;  // We have the file
			// It's in the database already.  Be sure the attribute is set
			PlutoMediaFile_.SetFileAttribute(PK_File);  
		}

        if(m_bAsDaemon)
			Sleep(500);
		
		int i = PlutoMediaFile_.GetPicAttribute(PK_File);
		if(!PK_Picture)
			PK_Picture = i;

		if( PK_Picture )
		{
			string sSql="SELECT Attribute.* FROM Attribute"
				" JOIN File_Attribute ON FK_Attribute=PK_Attribute"
				" JOIN File ON FK_File=PK_File"
				" JOIN MediaType_AttributeType ON Attribute.FK_AttributeType=MediaType_AttributeType.FK_AttributeType"
				" AND MediaType_AttributeType.EK_MediaType=File.EK_MediaType"
				" WHERE PK_File=" + StringUtils::itos(PK_File) + " AND Identifier=1";
			vector<Row_Attribute *> vectRow_Attribute;
			m_pDatabase_pluto_media->Attribute_get()->GetRows(sSql,&vectRow_Attribute);
			for(size_t s=0;s<vectRow_Attribute.size();++s)
			{
				Row_Attribute *pRow_Attribute = vectRow_Attribute[s];
				Row_Picture_Attribute *pRow_Picture_Attribute = m_pDatabase_pluto_media->Picture_Attribute_get()->GetRow(PK_Picture,pRow_Attribute->PK_Attribute_get());
				if( !pRow_Picture_Attribute )
				{
					pRow_Picture_Attribute = m_pDatabase_pluto_media->Picture_Attribute_get()->AddRow();
					pRow_Picture_Attribute->FK_Picture_set(PK_Picture);
					pRow_Picture_Attribute->FK_Attribute_set(pRow_Attribute->PK_Attribute_get());
					m_pDatabase_pluto_media->Picture_Attribute_get()->Commit();
				}
			}
		}
	}

	// Now recurse
	list<string> listSubDirectories;
	FileUtils::FindDirectories(listSubDirectories,sDirectory,false,true,0,""
#ifndef WIN32
		,&m_MapInodes
#endif
	);

    PlutoMediaFile PlutoMediaParentFolder(m_pDatabase_pluto_media, m_nPK_Installation,
        FileUtils::BasePath(sDirectory),FileUtils::FilenameWithoutPath(sDirectory), true);

	bool bDirIsDvd = false;

    cout << (int) listSubDirectories.size() << " sub directories" << endl;
	for(list<string>::iterator it=listSubDirectories.begin();it!=listSubDirectories.end();++it)
	{
		string sSubDir = *it;

        if(m_bAsDaemon)
			Sleep(500);
		
        //is sDirectory a ripped dvd ?
		if( 
            StringUtils::ToUpper(FileUtils::FilenameWithoutPath(sSubDir))=="VIDEO_TS" || 
            StringUtils::ToUpper(FileUtils::FilenameWithoutPath(sSubDir))=="AUDIO_TS" 
        )
		{
             cout << sDirectory << " is a ripped dvd" << endl;

			// Add this directory like it were a file
			int PK_File = PlutoMediaParentFolder.HandleFileNotInDatabase(MEDIATYPE_pluto_DVD_CONST);
			Row_File *pRow_File = m_pDatabase_pluto_media->File_get()->GetRow(PK_File);
			pRow_File->IsDirectory_set(false);
			m_pDatabase_pluto_media->File_get()->Commit();

			PlutoMediaParentFolder.SetFileAttribute(PK_File);
			bDirIsDvd = true;
			break; // Don't recurse anymore
		}

		map<string,pair<Row_File *,bool> >::iterator itMapFiles = mapFiles.find(FileUtils::FilenameWithoutPath(sSubDir));
		if( itMapFiles!=mapFiles.end() && !itMapFiles->second.first->IsDirectory_get())
		{
            cout << sDirectory << " is a sub-dir already categoriezed ripped dvd" << endl;

            PlutoMediaFile PlutoMediaSubDir(m_pDatabase_pluto_media, m_nPK_Installation,
                FileUtils::BasePath(sSubDir), "");
            PlutoMediaSubDir.SetFileAttribute(itMapFiles->second.first->PK_File_get());
			continue; // This directory is already in the database 
		}
		
		if(itMapFiles == mapFiles.end())
		{
			PlutoMediaFile PlutoMediaSubDir(m_pDatabase_pluto_media, m_nPK_Installation,
				FileUtils::BasePath(sSubDir), FileUtils::FilenameWithoutPath(sSubDir), true);

			PlutoMediaSubDir.HandleFileNotInDatabase();
		}

		int i = ReadDirectory(sSubDir, bRecursive);
		if( !PK_Picture )
			PK_Picture = i;
	}

	if(!bDirIsDvd)
	{
		string sBaseDirectory = FileUtils::BasePath(sDirectory);
		string sDirectoryName = FileUtils::FilenameWithoutPath(sDirectory);
		string SQL = "SELECT count(*) FROM File WHERE Path = '" + 
			StringUtils::SQLEscape(FileUtils::ExcludeTrailingSlash(sBaseDirectory)) + "' AND "
			" Filename = '" + StringUtils::SQLEscape(FileUtils::ExcludeTrailingSlash(sDirectoryName)) + 
			"' AND Missing = 0";

		PlutoSqlResult allresult;
		MYSQL_ROW row;
		bool bAlreadyinDb = false;
        if((allresult.r = m_pDatabase_pluto_media->mysql_query_result(SQL)))
        {
        	row = mysql_fetch_row(allresult.r);
            if(row && atoi(row[0]) > 0)
				bAlreadyinDb = true;
		}

		if(!bAlreadyinDb)
		{
			g_pPlutoLogger->Write(LV_WARNING, "Adding parent folder to db: %s", sDirectory.c_str());
			PlutoMediaParentFolder.HandleFileNotInDatabase(0);
		}
		else
		{
			g_pPlutoLogger->Write(LV_WARNING, "Parent folder already in the database: %s", sDirectory.c_str());
		}
	}

	// Whatever was the first picture we found will be the one for this directory
	int PK_Picture_Directory = PlutoMediaParentFolder.GetPicAttribute(0);
	if( PK_Picture_Directory )
		PK_Picture = PK_Picture_Directory;  // This takes priority

	PlutoMediaParentFolder.SetPicAttribute(PK_Picture, "");

	return PK_Picture;
}

void UpdateMedia::UpdateSearchTokens()
{
	g_pPlutoLogger->Write(LV_STATUS, "Updating search tokens...");
	
	string SQL = "DELETE FROM SearchToken_Attribute";
	m_pDatabase_pluto_media->threaded_mysql_query(SQL);

	SQL = "SELECT PK_Attribute,Name FROM Attribute";

	PlutoSqlResult allresult,result;
    MYSQL_ROW row,row2;
    if( ( allresult.r=m_pDatabase_pluto_media->mysql_query_result( SQL ) ) )
	{
		while( ( row=mysql_fetch_row( allresult.r ) ) )
		{
			if(m_bAsDaemon)
				Sleep(40);

			string sName = row[1];
			string::size_type pos=0;
			while( pos<sName.size() )
			{
				string Token=StringUtils::UpperAZ09Only( StringUtils::Tokenize( sName, " ", pos ) );
				if( Token.length( )==0 )
					continue;
				SQL = "SELECT PK_SearchToken FROM SearchToken WHERE Token='" +
					StringUtils::SQLEscape( Token ) + "'";
				int PK_SearchToken=0;
				if( ( result.r=m_pDatabase_pluto_media->mysql_query_result( SQL ) ) && ( row2=mysql_fetch_row( result.r ) ) )
				{
					PK_SearchToken = atoi( row2[0] );
				}
				else
				{
					SQL = "INSERT INTO SearchToken( Token ) VALUES( '" +
						StringUtils::SQLEscape( Token ) + "' )";
					PK_SearchToken = m_pDatabase_pluto_media->threaded_mysql_query_withID( SQL );
				}
				if( PK_SearchToken )
				{
					SQL = "INSERT INTO SearchToken_Attribute(FK_SearchToken,FK_Attribute) VALUES( " +
						StringUtils::itos( PK_SearchToken ) + ", " +
						row[0] + " )";
					m_pDatabase_pluto_media->threaded_mysql_query( SQL, true );
				}
			}
		}
	}

	g_pPlutoLogger->Write(LV_STATUS, "Update search tokens ended.");
}

void UpdateMedia::UpdateThumbnails()
{
	g_pPlutoLogger->Write(LV_STATUS, "Updating thumbs...");
	
	string SQL = "SELECT PK_Picture,Extension FROM Picture";
	PlutoSqlResult result;
    MYSQL_ROW row;
    if( ( result.r=m_pDatabase_pluto_media->mysql_query_result( SQL ) ) )
	{
		while( ( row=mysql_fetch_row( result.r ) ) )
		{
			if(m_bAsDaemon)
				Sleep(40);

			time_t tModTime=0,tTnModTime=0;
#ifndef WIN32
			struct stat64 dirEntryStat;
            if ( stat64((string("/home/mediapics/") + row[0] + "." + row[1]).c_str(), &dirEntryStat) == 0 )
				tModTime = dirEntryStat.st_mtime;

            if ( stat64((string("/home/mediapics/") + row[0] + "_tn." + row[1]).c_str(), &dirEntryStat) == 0 )
				tTnModTime = dirEntryStat.st_mtime;
#endif
			if( tModTime>tTnModTime )
			{
				string Cmd = "convert -sample 75x75 /home/mediapics/" + string(row[0])  + "." + row[1] +
					" /home/mediapics/" + row[0] + "_tn." + row[1];
				int result;
				if( ( result=system( Cmd.c_str( ) ) )!=0 )
					g_pPlutoLogger->Write( LV_CRITICAL, "Thumbnail picture %s returned %d", Cmd.c_str( ), result );
			}
			else if( tModTime==0 && tTnModTime==0 )
			{
				cout << "Picture " << row[0] << " missing.  Deleting all references" << endl;
				m_pDatabase_pluto_media->threaded_mysql_query("DELETE FROM Picture WHERE PK_Picture=" + string(row[0]));
				m_pDatabase_pluto_media->threaded_mysql_query("DELETE FROM Picture_Attribute WHERE FK_Picture=" + string(row[0]));
				m_pDatabase_pluto_media->threaded_mysql_query("DELETE FROM Picture_Disc WHERE FK_Picture=" + string(row[0]));
				m_pDatabase_pluto_media->threaded_mysql_query("DELETE FROM Picture_File WHERE FK_Picture=" + string(row[0]));
				m_pDatabase_pluto_media->threaded_mysql_query("UPDATE Bookmark SET FK_Picture=NULL WHERE FK_Picture=" + string(row[0]));
			}

			g_pPlutoLogger->Write(LV_STATUS, "Thumbs updated!");
		}
	}
}

void UpdateMedia::SyncDbWithDirectory(string sDirectory)
{
	g_pPlutoLogger->Write(LV_STATUS, "Sync'ing db with directory...");
	
    bool bRecordsModified = false;

	vector<Row_File *> vectRow_File;
	m_pDatabase_pluto_media->File_get()->GetRows("Path LIKE '" + StringUtils::SQLEscape(sDirectory) + "/%' OR Path = '" + StringUtils::SQLEscape(sDirectory) + "'",
		&vectRow_File);

	for(vector<Row_File *>::iterator it = vectRow_File.begin(), end = vectRow_File.end(); it != end; ++it)
	{
		if(m_bAsDaemon)
			Sleep(40);

		Row_File *pRow_File = *it;
        string sFilePath = pRow_File->Path_get() + "/" + pRow_File->Filename_get();
        bool bFileIsMissing = !FileUtils::FileExists(sFilePath);

		bool bDeviceOnline = true;
		if(pRow_File->EK_Device_get() != 0)
		{
			Row_Device_DeviceData *pRow_Device_DeviceData = m_pDatabase_pluto_main->Device_DeviceData_get()->GetRow(
				pRow_File->EK_Device_get(), DEVICEDATA_Online_CONST);

			if(NULL != pRow_Device_DeviceData)
				bDeviceOnline = (pRow_Device_DeviceData->IK_DeviceData_get() == "1");
		}
		
		if(bDeviceOnline)
		{
			if(bFileIsMissing && !pRow_File->Missing_get()) 
			{
				if(ConfirmDeviceIsOnline(pRow_File->EK_Device_get()))
				{
					pRow_File->Missing_set(1);
					g_pPlutoLogger->Write(LV_STATUS, "Marking record as missing in database: %s", sFilePath.c_str());
					bRecordsModified = true;
				}
			}
			else if(!bFileIsMissing && pRow_File->Missing_get())
			{
				pRow_File->Missing_set(0);
				g_pPlutoLogger->Write(LV_STATUS, "Marking record as NOT missing in database: %s", sFilePath.c_str());
				bRecordsModified = true;
			}
		}
		else
			g_pPlutoLogger->Write(LV_STATUS, "Device is offline, skipping the file %s", sFilePath.c_str());
	}

    if(bRecordsModified)
        m_pDatabase_pluto_media->File_get()->Commit();

	g_pPlutoLogger->Write(LV_STATUS, "DB sync'd with directory!");
}

bool UpdateMedia::ConfirmDeviceIsOnline(long EK_Device)
{
	//TODO: find an elegant way to confirm that a device is online
	return true;
}
