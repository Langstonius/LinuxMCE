/*
 Main

 Copyright (C) 2004 Pluto, Inc., a Florida Corporation

 www.plutohome.com
 

 Phone: +1 (877) 758-8648


 This program is distributed according to the terms of the Pluto Public License, available at:
 http://plutohome.com/index.php?section=public_license

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the Pluto Public License for more details.

 */
#ifndef __SCREEN_HANDLER_H__
#define __SCREEN_HANDLER_H__
//-----------------------------------------------------------------------------------------------------
#include "Gen_Devices/AllScreens.h"
#include "Orbiter.h"
#include "CallBackData.h"
#include "pluto_main/Define_DesignObj.h"
#include "pluto_main/Define_Variable.h"
#include "pluto_media/Define_MediaSource.h"

//-----------------------------------------------------------------------------------------------------
typedef bool (ScreenHandler::*ScreenHandlerCallBack)(CallBackData *pData);
//-----------------------------------------------------------------------------------------------------
typedef class ScreenHandler * (* RAOP_FType) (class Orbiter *,  map<int,int> *p_MapDesignObj, Logger *);
//-----------------------------------------------------------------------------------------------------

class MediaFileBrowserOptions
{
public:
	int m_PK_MediaType,m_PK_AttributeType_Sort,m_PK_Users,m_iPK_Screen;
	int m_iLastViewed; // 0=no, 1=yes, 2=either
	int m_iPK_DesignObj_Browser; // The Design obj for the current browse screen
	string m_sPK_MediaSubType,m_sPK_FileFormat,m_sPK_Attribute_Genres,m_sSources,m_sPK_Users_Private;
	string m_sSelectedFile;  // Used to hold the selected value
	list<int> m_listPK_AttributeType_Sort_Prior;
	list< pair<int,string> >m_listPK_Attribute_Description;  // For drilling down on attributes
	map< pair<int,string>, DesignObj_Orbiter * > m_mapObjectsValues;
	Orbiter *m_pOrbiter;
	DesignObj_DataGrid *m_pObj_ListGrid,*m_pObj_PicGrid;

	MediaFileBrowserOptions(Orbiter *pOrbiter) 
	{ 
		m_pOrbiter=pOrbiter; 
		m_pObj_ListGrid=m_pObj_PicGrid=NULL;
		m_iLastViewed=2;
		m_iPK_DesignObj_Browser = 0;
		ClearAll(0,0,0); 
	}

	string ToString()
	{
		string sResult = StringUtils::itos(m_PK_MediaType) + "|" + m_sPK_MediaSubType + "|" + m_sPK_FileFormat + "|" + m_sPK_Attribute_Genres + "|" + m_sSources +
			"|" + m_sPK_Users_Private + "|" + StringUtils::itos(m_PK_AttributeType_Sort) + "|" + StringUtils::itos(m_PK_Users) + " | "
			+ StringUtils::itos(m_iLastViewed) + " | ";
		if( m_listPK_Attribute_Description.size() )
			sResult += StringUtils::itos(m_listPK_Attribute_Description.front().first);
		return sResult;
	}

	string HumanReadable();

	void ClearAll(int PK_MediaType,int PK_Screen,int PK_AttributeType_Sort)
	{
		m_iPK_Screen=PK_Screen;
		m_PK_MediaType=PK_MediaType;
		m_PK_Users=0;
		m_PK_AttributeType_Sort = PK_AttributeType_Sort;
		m_listPK_AttributeType_Sort_Prior.clear();
		m_listPK_Attribute_Description.clear();
		m_sPK_MediaSubType=""; m_sPK_FileFormat=""; m_sPK_Attribute_Genres=""; m_sPK_Users_Private="0";
		m_sSources=TOSTRING(MEDIASOURCE_Hard_Drives_CONST) "," TOSTRING(MEDIASOURCE_Discs__Jukeboxes_CONST);
		m_pOrbiter->CMD_Set_Variable(VARIABLE_PK_MediaType_CONST, StringUtils::itos(m_PK_MediaType)); 
		m_mapObjectsValues.clear();
		if( PK_Screen )
			m_iPK_DesignObj_Browser = m_pOrbiter->m_mapDesignObj[PK_Screen];

		if( !m_pObj_ListGrid && m_iPK_DesignObj_Browser )
			m_pObj_ListGrid=(DesignObj_DataGrid *) m_pOrbiter->FindObject(StringUtils::itos(m_iPK_DesignObj_Browser) + ".0.0." TOSTRING(DESIGNOBJ_dgFileList2_CONST));
		if( !m_pObj_PicGrid && m_iPK_DesignObj_Browser )
			m_pObj_PicGrid=(DesignObj_DataGrid *) m_pOrbiter->FindObject(StringUtils::itos(m_iPK_DesignObj_Browser) + ".0.0." TOSTRING(DESIGNOBJ_dgFileList2_Pics_CONST));
		ReacquireGrids();
	}

	void ReacquireGrids()
	{
		if( m_pObj_ListGrid )
		{
			if( !m_pObj_ListGrid->m_bParsed )
				m_pOrbiter->ParseGrid(m_pObj_ListGrid);
			m_pObj_ListGrid->m_GridCurCol = m_pObj_ListGrid->m_iInitialColNum;
			m_pObj_ListGrid->m_GridCurRow = m_pObj_ListGrid->m_iInitialRowNum;
			m_pObj_ListGrid->m_bFlushOnScreen=false;
			m_pObj_ListGrid->Flush();
		}
		if( m_pObj_PicGrid )
		{
			if( !m_pObj_PicGrid->m_bParsed )
				m_pOrbiter->ParseGrid(m_pObj_PicGrid);
			m_pObj_PicGrid->m_GridCurCol = m_pObj_PicGrid->m_iInitialColNum;
			m_pObj_PicGrid->m_GridCurRow = m_pObj_PicGrid->m_iInitialRowNum;
			m_pObj_PicGrid->m_bFlushOnScreen=false;
			m_pObj_PicGrid->Flush();
		}
	}

	void SelectArrays(DesignObj_Orbiter *pObj,int PK_Array,string &sValues);
	void SelectArrays(DesignObj_Orbiter *pObj,int PK_Array,int &iValue);
	void SelectedArray(DesignObj_Orbiter *pObj,int PK_Array,string &sValues,bool bTreatZeroAsAll);
	void SelectedArray(DesignObj_Orbiter *pObj,int PK_Array,int &iValue);
};

class ScreenHandler : public ScreenHandlerBase
{
protected:
	Orbiter *m_pOrbiter;
	map<CallBackType, ScreenHandlerCallBack> m_mapCallBack;
	map<CallBackType, CallBackData *> m_mapCallBackData;
	string m_sActiveApplication_Description,m_sActiveApplication_Window;
	int m_PK_Screen_ActiveApp_OSD,m_PK_Screen_ActiveApp_Remote;
	int m_iStage; // Some screens go through stages and need a variable to track that
	time_t m_tLastDeviceAdded;

	int m_PK_Software;  // For add software grid
	string m_sInstallationStatus;  // For add software grid

	pluto_pthread_mutex_t m_MapMutex; /** < Protected the access to our maps */

	char *m_pData_LastThumbnail;
	int m_iData_Size_LastThumbnail;

	virtual void DisplayMessageOnOrbiter(int PK_Screen,
		string sMessage, bool bPromptToResetRouter = false,
		string sTimeout = "0", bool bCantGoBack = false, 
		string sOption1 = "", string sMessage1 = "",
		string sOption2 = "", string sMessage2 = "", 
		string sOption3 = "", string sMessage3 = "",
		string sOption4 = "", string sMessage4 = "",
		string sID="");


public: 
	ScreenHandler(Orbiter *pOrbiter, map<int,int> *p_MapDesignObj);
	virtual ~ScreenHandler();

	//used to register as plugin
	virtual bool Register() { return 0; }

	//callback related stuff
	void RegisterCallBack(CallBackType aCallBackType, ScreenHandlerCallBack aScreenHandlerCallBack, 
		CallBackData *pCallBackData);
	virtual void ResetCallBacks();
	ScreenHandlerCallBack m_mapCallBack_Find(CallBackType aCallBackType);
	CallBackData *m_mapCallBackData_Find(CallBackType aCallBackType);

	map<string,string> m_mapKeywords;  // Used for storing keywords to be substituded into text
	string m_mapKeywords_Find(string sKeyword) { PLUTO_SAFETY_LOCK( vm, m_pOrbiter->m_VariableMutex ); map<string,string>::iterator it = m_mapKeywords.find(sKeyword); return it==m_mapKeywords.end() ? "" : (*it).second; }

	//helper functions
	int GetCurrentScreen_PK_DesignObj();
	void RefreshDatagrid(long PK_DesignObj_Datagrid);
	void BadGotoScreen(int PK_Screen);

	//screens functions
	virtual void GotoDesignObj(int PK_DesignObj,string sID="",bool bStore_Variables=false,bool bCant_Go_Back=false);
	virtual void SCREEN_NewPnpDevice(long PK_Screen, string sDescription, int iPK_PnpQueue);
	virtual void SCREEN_New_Pnp_Device_One_Possibility(long PK_Screen, int iPK_Room, int iPK_DHCPDevice, string sDescription, string ssComments, int iPK_PnpQueue);
	bool Pnp_ObjectSelected(CallBackData *pData);
	virtual void SCREEN_CDTrackCopy(long PK_Screen, int iPK_Users, string sName, int iDriveID); 
	MediaFileBrowserOptions mediaFileBrowserOptions;  // Current sort/filter options for the media browser

//	virtual void SCREEN_FileList_PlayLists(long PK_Screen);
	virtual void SCREEN_FileList_Music_Movies_Video(long PK_Screen);
	bool MediaBrowsre_Intercepted(CallBackData *pData);
	bool MediaBrowser_ObjectSelected(CallBackData *pData);
	string GetFileBrowserPopup(DesignObj_Orbiter *pObj_MenuPad);
	bool MediaBrowser_DatagridSelected(CallBackData *pData);
	void SelectedMediaFile(string sFile);
	bool FileList_GridRendering(CallBackData *pData);
	void SelectedAttributeCell(DataGridCell *pCell);
	bool MediaBrowser_Render(CallBackData *pData);
	void SetMediaSortFilterSelectedObjects();
	void GetAttributesForMediaFile(const char *pFilename);
	bool FileList_KeyDown(CallBackData *pData);

	virtual void SCREEN_NewPhoneDetected(long PK_Screen, string sMac_address, string sDescription, int iPK_PnpQueue);
	virtual void SCREEN_WhatModelMobileOrbiter(long PK_Screen, int iPK_Users, string sMac_address);
	virtual void SCREEN_Pick_Room_For_Device(long PK_Screen, int iPK_Device, string sDescription, string ssComments);
	virtual void SCREEN_SensorsNotReady(long PK_Screen, string sDescription);
	virtual void SCREEN_ModeChanged(long PK_Screen, string sPK_HouseMode, string sHouseModeTime, 
		string sExitDelay, string sAlerts);
	virtual void SCREEN_CurrentlyActiveRemote(long PK_Screen);
	virtual void SCREEN_PopupMessage(long PK_Screen, string sText, string sCommand_Line, string sDescription, string sPromptToResetRouter, string sTimeout, string sCannotGoBack);
	virtual void SCREEN_GenericKeyboard(long PK_Screen, string sText, string sCommand_Line, string sDescription, string sCannotGoBack){ SCREEN_PopupMessage(PK_Screen, sText, sCommand_Line, sDescription, "0", "0", sCannotGoBack); }  // Treat this like the popup message
	virtual void SCREEN_Power(long PK_Screen);
	bool Power_ObjectSelected(CallBackData *pData);

	virtual void SCREEN_GenericAppController(long PK_Screen);
	virtual void SCREEN_GenericAppFullScreen(long PK_Screen);
	virtual void SCREEN_Computing(long PK_Screen);
	bool Computing_ObjectSelected(CallBackData *pData);
	bool Computing_DatagridSelected(CallBackData *pData);

	//dialogs
	virtual void SCREEN_DialogCannotPlayMedia(long PK_Screen, string sErrors);
	virtual void SCREEN_DialogRippingInProgress(long PK_Screen, string sPK_DeviceFrom, string sPK_RippingDevice);
	virtual void SCREEN_DialogCheckingDrive(long PK_Screen);
	virtual void SCREEN_DialogUnableToSavePlaylist(long PK_Screen);
	virtual void SCREEN_DialogPlaylistSaved(long PK_Screen);
	virtual void SCREEN_DialogUnableToLoadPlaylist(long PK_Screen);
	virtual void SCREEN_DialogRippingError(long PK_Screen, string sDescription, string sTimeout	);
	virtual void SCREEN_DialogRippingInstructions(long PK_Screen);
	virtual void SCREEN_DialogGenericError(long PK_Screen, string sDescription, string sPromptToResetRouter, 
		string sTimeout, string sCannotGoBack);
	virtual void SCREEN_DialogCannotBookmark(long PK_Screen, string sErrors);
	virtual void SCREEN_DialogAskToResume(long PK_Screen, string sPK_Device_From, string sPK_Device_MediaSource, 
		string sStreamID_String, string sPosition, string sUsers, string sPK_MediaType_String, int iPK_Screen_GoTo);
	virtual void SCREEN_DialogGC100Error(long PK_Screen, string sDescription, string sCannotGoBack);
	virtual void SCREEN_DialogPhoneInstructions(long PK_Screen, string sInstructions, string sPhoneName);
	virtual void SCREEN_DialogSendFileToPhoneFailed(long PK_Screen, string sMacAddress, string sCommandLine, 
		string sPK_DeviceFrom, string sPhoneName, string sPK_Device_AppServer);
	virtual void SCREEN_Floorplan(long PK_Screen, string sPK_DesignObj);

	//Screens with locations
	virtual void SCREEN_Main(long PK_Screen, string sLocation);

	virtual void SCREEN_Lights(long PK_Screen, string sLocation);
	virtual void SCREEN_Media(long PK_Screen, string sLocation);
	virtual void SCREEN_Climate(long PK_Screen, string sLocation);
	virtual void SCREEN_Security(long PK_Screen, string sLocation);
	virtual void SCREEN_Telephony(long PK_Screen, string sLocation);
	virtual void SCREEN_Misc(long PK_Screen, string sLocation);

	virtual void SCREEN_popLights(long PK_Screen, string sLocation);
	virtual void SCREEN_popMedia(long PK_Screen, string sLocation);
	virtual void SCREEN_popClimate(long PK_Screen, string sLocation);
	virtual void SCREEN_popTelecom(long PK_Screen, string sLocation);
	virtual void SCREEN_popSecurity(long PK_Screen, string sLocation);

	virtual void SCREEN_NAS_Options(long PK_Screen, int iPK_PnpQueue);
	virtual void SCREEN_Get_Username_Password_For_Devices(long PK_Screen, bool bAlready_processed, string sDescription, int iPK_PnpQueue);

	virtual void SCREEN_Download_are_ready_to_install(long PK_Screen, int iPK_Device, string sPK_Device_AppServer);

	virtual void SCREEN_CreateViewBookmarks(long PK_Screen);
	bool Bookmark_GridSelected(CallBackData *pData);

	//cameras
	virtual void SCREEN_SingleCameraViewOnly(long PK_Screen, int iPK_Device);
	virtual void SCREEN_QuadViewCameras(long PK_Screen, string sList_PK_Device);
	virtual void SCREEN_Sensors_Viewed_By_Camera(long PK_Screen, string sOptions, int iPK_PnpQueue);
	bool Sensors_ObjectSelected(CallBackData *pData);

	virtual bool New_Phone_Enter_Number_DeviceConfigured(CallBackData *pData);
	virtual void SCREEN_TV_Channels(long PK_Screen);
	bool TV_Channels_ObjectSelected(CallBackData *pData);
	bool TV_Channels_GridRendering(CallBackData *pData);

	virtual void SCREEN_Add_Software(long PK_Screen);
	bool AddSoftware_ObjectSelected(CallBackData *pData);
	bool AddSoftware_ObjectHighlighted(CallBackData *pData);
	bool AddSoftware_GridRendering(CallBackData *pData);
	bool AddSoftware_GridSelected(CallBackData *pData);

	virtual void SCREEN_Halt_System(long PK_Screen);

	virtual void SCREEN_FileSave(long PK_Screen, string sCaption, string sCommand, bool bAdvanced_options);
	bool FileSave_ObjectSelected(CallBackData *pData);
	bool FileSave_GridSelected(CallBackData *pData);

	void SCREEN_CreateViewBookmarksTV(long PK_Screen);
	bool CreateViewBookmarksTV_ObjectSelected(CallBackData *pData);

	void SCREEN_Thumbnail(long PK_Screen);
	bool Thumbnail_DatagridSelected(CallBackData *pData);

	//helper methods
	void SaveFile_GotoChooseFolderDesignObj();
	void SaveFile_SendCommand(int PK_Users);

	//savefile data
	string m_sSaveFile_MountedFolder;
	string m_sSaveFile_RelativeFolder;
	string m_sSaveFile_Drive;
	string m_sSaveFile_FullBasePath;
	string m_sSaveFile_FileName;
	string m_sSaveFile_Command;
	bool m_bSaveFile_CreatingFolder;
	bool m_bSaveFile_Advanced_options;
	int m_nSaveFile_PK_DeviceDrive;
	int m_nPK_Users_SaveFile;
};
//-----------------------------------------------------------------------------------------------------
#endif
