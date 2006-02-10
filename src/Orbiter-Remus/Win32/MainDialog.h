#if !defined(__MAIN_DIALOG_H__)
#define __MAIN_DIALOG_H__
//-----------------------------------------------------------------------------------------------------
#include "StartOrbiter.h"
//-----------------------------------------------------------------------------------------------------
#ifdef WINCE
	#include "stdafx.h"
	#include <commctrl.h>
#ifndef WINCE_x86
	#include <aygshell.h>
#endif
	#include <sipapi.h>
#else
	#include "windows.h"
	#include <string>
	using namespace std;
#endif

extern HINSTANCE	g_hInst; // The current instance
extern HWND			g_hwndMainDialog;		// The main dialog window handle
//-----------------------------------------------------------------------------------------------------
WORD				MyRegisterClass	(HINSTANCE, LPTSTR);
BOOL				InitInstance	(HINSTANCE, int);
LRESULT CALLBACK	WndProc			(HWND, UINT, WPARAM, LPARAM);

void				WriteStatusOutput(const char* pMessage);
void				ShowMainDialog();
void				ShowSDLWindow();

void				RecordMouseAction(int x, int y);
void				RecordKeyboardAction(long key);
//-----------------------------------------------------------------------------------------------------
void				StartOrbiterThread();
void				LoadUI_From_ConfigurationData();
void				SyncConfigurationData();
//-----------------------------------------------------------------------------------------------------
struct CommandLineParams
{
	string sRouter_IP;
	int PK_Device;
	int PK_DeviceTemplate;
	string sLogger;
	int Width, Height;
	bool bLocalMode; 
	bool bFullScreen;
	string sLocalDirectory;
	string sNestedDisplay;

	bool bRouterIPSpecified;
	bool bDeviceIDSpecified;
};
//-----------------------------------------------------------------------------------------------------
extern CommandLineParams CmdLineParams;
//-----------------------------------------------------------------------------------------------------
#endif //__MAIN_DIALOG_H__