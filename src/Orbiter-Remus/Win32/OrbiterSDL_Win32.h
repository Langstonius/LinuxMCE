#ifndef __ORBITER_SDL_WIN_H__
#define __ORBITER_SDL_WIN_H__
//-----------------------------------------------------------------------------------------------------
#include "../SDL/OrbiterSDL.h"
//-----------------------------------------------------------------------------------------------------
class OrbiterSDL_Win32 : public OrbiterSDL
{
private:

	static OrbiterSDL_Win32* m_pInstance; //the one and only instance of OrbiterSDL_Win32
	
	OrbiterSDL_Win32(int DeviceID, int PK_DeviceTemplate, string ServerAddress, string sLocalDirectory, bool bLocalMode, 
		int nImageWidth, int nImageHeight, bool bFullScreen = false);

public:

	bool m_bConnectionLost;

	WNDPROC OldSDLWindowProc;
	HWND hSDLWindow;

	~OrbiterSDL_Win32();

	//base public methods
	void OnQuit();
	void RenderScreen();
	bool SelfUpdate();

	void WriteStatusOutput(const char* pMessage);

	//OrbiterSDL_Win32 public methods
	void HandleKeyEvents(UINT uMsg, WPARAM wParam, LPARAM lParam);

	static void Cleanup();
	static void BuildOrbiter(int DeviceID, int PK_DeviceTemplate, string ServerAddress, string sLocalDirectory, bool bLocalMode, 
		int nImageWidth, int nImageHeight, bool bFullScreen = false);
	static OrbiterSDL_Win32 *GetInstance();

	virtual int PromptUser(string sPrompt,int iTimeoutSeconds=10,map<int,string> *p_mapPrompts=NULL);
    bool DisplayProgress(string sMessage, int nProgress);
};
//-----------------------------------------------------------------------------------------------------
#endif //__ORBITER_SDL_WIN_H__