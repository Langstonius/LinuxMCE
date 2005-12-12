#ifndef OrbiterGenerator_h
#define OrbiterGenerator_h

#include <stdlib.h>

#include "pluto_main/Database_pluto_main.h"
#include "DesignObj_Generator.h"
#include "Orbiter/OrbiterData.h"
#include "PlutoUtils/MySQLHelper.h"
#include "RegenMonitor.h"

class OrbiterGenerator : public OrbiterData, public MySqlHelper
{
public:
	Database_pluto_main mds;

	class Row_Orbiter * m_pRow_Orbiter;
	class Row_Device * m_pRow_Device;
	class Row_Room * m_pRow_Room;
	class Row_EntertainArea * m_pRow_EntertainArea;
	class Row_Screen *m_pRow_Screen_MainMenu,*m_pRow_Screen_Sleeping,*m_pRow_Screen_ScreenSaver;
	class Row_DesignObj *m_pRow_DesignObj_MainMenu,*m_pRow_DesignObj_Sleeping,*m_pRow_DesignObj_ScreenSaver;
	class Row_Skin *m_pRow_Skin;
	class Row_Language *m_pRow_Language;
	class RegenMonitor *m_pRegenMonitor;
	bool m_bNoEffects,m_bUseOCG;
	bool m_bIgnoreSelected, m_bIgnoreHighlighted, m_bIgnoreAlt;


	int m_iLocation;
	map<int,int> m_htUsedStyleVariation;
	map<int,int> m_mapUsedOrbiterCriteria;
	map<string,listDesignObj_Generator *> m_htGeneratedScreens;
	map<int,class TextStyle *> m_htStylesUsedInOrbiterText;
	map<int,bool> m_mapDesignObj_WithArrays;
	map<int,bool> m_mapPopups;
//	Hashtable m_htUsedDevices;
//	StringWriter m_swErrors;
	bool m_bOrbiterChanged,m_bIsMobilePhone;
	string m_iID_Array;
	string m_GraphicsBasePath;
	int m_iPK_DesignObj_Screen;
	int m_iPK_CommandGroup; // If this is an array and we are supposed to execute an action group, this will be non-zero.  Arrays are always customer created command groups
	int m_iFloorplanPage;
	map<int,bool> m_map_PK_DesignObj_SoleScreenToGen; // Only re-generate this one screen
	int m_iPK_Orbiter;
	int m_iScreensTotal,m_iScreensToRender,m_iLastReportedPercentage;  // Keep track of screens we're going to need to render
	bool m_bDontAutoRegenArrays;
	string m_sDesc_Array;
	class Row_Icon * m_pRow_Icon;
	class PlutoSize *m_sizeScreen;
	class Row_Size * m_pRow_Size;
	class Row_UI * m_pRow_UI;
	string m_sFloorPlanData;
	int m_iNumFloorplanItems;

	//	string m_sMySQLHost,m_sMySQLUser,m_sMySQLPass,m_sMySQLDBName;
	string m_sOutputPath,m_sGraphicsBasePath,m_sFontPath;
//	int m_iMySQLPort;
	PlutoSize m_sScaledSize;

	OrbiterGenerator(string sGraphicsFiles,string sFontFiles,string sOutputFiles,int PK_Orbiter,string DBHost,string DBUser,string DBPassword,string DBName,int DBPort)
		: MySqlHelper(DBHost, DBUser, DBPassword, DBName, DBPort)
	{
		m_bOrbiterChanged=false;
		m_sGraphicsBasePath=sGraphicsFiles;
		m_sFontPath=sFontFiles;
		m_sOutputPath=sOutputFiles;
		m_iPK_Orbiter=PK_Orbiter;
		m_pRegenMonitor = new RegenMonitor(this);
/*		m_sMySQLHost=DBHost;
		m_sMySQLUser=DBUser;
		m_sMySQLPass=DBPassword;
		m_sMySQLDBName=DBName;
		m_iMySQLPort=DBPort;
*/

		m_sFloorPlanData="";
		m_iNumFloorplanItems=0;
		Counter=0;
		m_iID_Array="0";
		m_bDontAutoRegenArrays=false;
		m_iPK_DesignObj_Screen=0;
		m_iPK_CommandGroup=0; // If this is an array and we are supposed to execute an action group, this will be non-zero
		m_iFloorplanPage=0;
		m_iScreensToRender=m_iScreensTotal=m_iLastReportedPercentage=0;
		m_pRow_Icon=NULL;

        m_pRow_Orbiter = NULL;
	}

    ~OrbiterGenerator() {}
	int DoIt();
	Row_DesignObj *GetDesignObjFromScreen(int PK_Screen);
	Row_DesignObj *GetDesignObjFromScreen(Row_Screen *pRow_Screen);

private:
	map<int,string> htDevicePages;
	long Counter;

	void OutputScreen(class DesignObj_Generator *ocDesignObj);
	void OutputDesignObjs(class DesignObj_Generator *ocDesignObj,int ArrayPage,bool bIsChild,string ParentScreen);
	void OutputText(class DesignObj_Generator *ocDesignObj,class CGText *ot,bool bPrerender);
	void OutputCriteriaNest(class Row_CriteriaParmNesting *row);
	void SearchForGotos(class DesignObj_Data *pDesignObj_Data);
	void SearchForGotos(class DesignObj_Data *pDesignObj_Data,DesignObjCommandList *alCommands);
	void MatchChildDevicesToLocation(LocationInfo *li,Row_Device *pRow_Device);
	void ScaleCommandList(DesignObj_Generator *pocDesignObj,DesignObjCommandList &CommandList);
	class Row_Size *TranslateSize(string sSize);
	bool CommonControlledVia(Row_Device *pRow_Device1,Row_Device *pRow_Device2);
	string First2Dots(string sDesignObj);
	//void PopulateScreenMap();
};

#endif
