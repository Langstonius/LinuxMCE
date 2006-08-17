#ifndef __PLUTO_MEDIA_FILE_H__
#define __PLUTO_MEDIA_FILE_H__

#include <string>
#include <map>
#include <list>
using namespace std;
//-----------------------------------------------------------------------------------------------------
class Database_pluto_media;
class Database_pluto_main;
class PlutoMediaAttributes;

enum PlutoCustomTag
{
	pctInstallation,
	pctFile,
	pctPicture,
	pctPictureUrl
};

//-----------------------------------------------------------------------------------------------------
//
//  PlutoMediaFile class
//
//-----------------------------------------------------------------------------------------------------
class PlutoMediaFile
{
private:

	Database_pluto_media *m_pDatabase_pluto_media;
	PlutoMediaAttributes *m_pPlutoMediaAttributes;

    string m_sDirectory;
    string m_sFile;
	int m_nOurInstallationID;
	int m_nPK_MediaType;
	bool m_bIsDir;
	static bool m_bSyncFilesOnly;

    //internal helper functions
	//This will add a record in the File table and additional attributes too in related tables
    int AddFileToDatabase(int PK_MediaType);
	string FileWithAttributes(bool bCreateId3File = true);
	string FileWithAttributes_PreviousVersion(); //for upgrading. to be removed next release

	void SavePlutoAttributes(string sFullFileName);
	void LoadPlutoAttributes(string sFullFileName);
	void LoadAttributesFromDB(string sFullFileName, int PK_File);

	void SyncDbAttributes();

public:
    PlutoMediaFile(Database_pluto_media *pDatabase_pluto_media, int PK_Installation,
        string sDirectory, string sFile, bool bIsDir = false);
    ~PlutoMediaFile();

	static void SetupSyncFilesOnly(bool bSyncFilesOnly);

    int HandleFileNotInDatabase(int PK_MediaType = 0);

    void SetFileAttribute(int PK_File);
    int GetFileAttribute(bool bCreateId3File = true);

    void SetPicAttribute(int PK_Picture, string sPictureUrl);
    int GetPicAttribute(int PK_File);
	static bool IsSupported(string sFileName);
};
//-----------------------------------------------------------------------------------------------------
//
//  PlutoMediaIdentifier class
//
//-----------------------------------------------------------------------------------------------------
class PlutoMediaIdentifier
{
private:
    static map<string,int> m_mapExtensions;

    virtual ~PlutoMediaIdentifier() = 0; //don't allow instances
public:
    static void Activate(Database_pluto_main *pDatabase_pluto_main);
    static int Identify(string sFilename);
};
//-----------------------------------------------------------------------------------------------------

#endif
