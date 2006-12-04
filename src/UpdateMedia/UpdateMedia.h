#ifndef UpdateMedia_h
#define UpdateMedia_h

#include "pluto_media/Database_pluto_media.h"
#include "pluto_main/Database_pluto_main.h"
#include "MediaState.h"

class UpdateMedia 
{
	string m_sDirectory,m_sExtensions;
	map<string,int> m_mapExtensions;
	bool m_bAsDaemon;
	int m_nPK_Installation;
	MapDbStateEx m_mapCurrentDbState;

	void ReadConfigFile();

	//Helper method
	void SetupInstallation();

#ifndef WIN32
	map<u_int64_t,bool> m_MapInodes;
#endif

public:
    Database_pluto_media *m_pDatabase_pluto_media ;
    Database_pluto_main *m_pDatabase_pluto_main;

	UpdateMedia(string host, string user, string pass, int port,string sDirectory,bool bSyncFilesOnly);
    UpdateMedia(Database_pluto_media *pDatabase_pluto_media, Database_pluto_main *pDatabase_pluto_main, 
        string sDirectory);

	void DoIt();

	int ReadDirectory(string Directory, bool bRecursive); // Returns the picture for the directory

    void UpdateSearchTokens();
	void UpdateThumbnails();
    void SyncDbWithDirectory(string sDirectory);

private:

	bool ConfirmDeviceIsOnline(long EK_Device);			
};
#endif
