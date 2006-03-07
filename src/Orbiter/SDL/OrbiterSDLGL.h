#ifndef __OrbiterSDL_H__
#define __OrbiterSDL_H__

//-----------------------------------------------------------------------------------------------------
#include <iostream> 
using namespace std; 

#include <SDL.h>

//-----------------------------------------------------------------------------------------------------
#include "../Orbiter.h"
#include "../DesignObj_Orbiter.h"
//-----------------------------------------------------------------------------------------------------
namespace DCE
{

class OrbiterSDLGL : public Orbiter
{

protected: // (mtoader) I want access to them in the OrbiterLinuxDesktop
	bool m_bFullScreen;
	pthread_cond_t m_GLThreadCond;

public:
	OrbiterSDLGL(int DeviceID, int PK_DeviceTemplate, string ServerAddress, string sLocalDirectory, 
        bool bLocalMode, int nImageWidth, int nImageHeight, bool bFullScreen = false, 
        pluto_pthread_mutex_t *pExternalScreenMutex = NULL);
	virtual ~OrbiterSDLGL();
    virtual bool GetConfig();

	// Public virtual methods

	// Drawing routines
	virtual void SolidRectangle(int x, int y, int width, int height, PlutoColor color, int Opacity = 100);
	virtual void HollowRectangle(int X, int Y, int Width, int Height, PlutoColor color);
	virtual void DrawLine(int x, int y, int width, int height, PlutoColor color, int Opacity = 100) {};
	virtual void ReplaceColorInRectangle(int x, int y, int width, int height, PlutoColor ColorToReplace, PlutoColor ReplacementColor);
	virtual void FloodFill(int x, int y, PlutoColor ColorToReplace, PlutoColor ReplacementColor) {};
	virtual void RenderText(string &sTextToDisplay,class DesignObjText *Text,class TextStyle *pTextStyle, PlutoPoint point = PlutoPoint(0, 0));
	virtual void SaveBackgroundForDeselect(DesignObj_Orbiter *pObj, PlutoPoint point);
	virtual PlutoGraphic *GetBackground( PlutoRectangle &rect );
	virtual PlutoGraphic *CreateGraphic();

	// Rendering
	virtual void RenderScreen();
	virtual void RedrawObjects();
	virtual void DisplayImageOnScreen(SDL_Surface *m_pScreenImage);

	virtual void RenderGraphic(class PlutoGraphic *pPlutoGraphic, PlutoRectangle rectTotal, bool bDisableAspectRatio, PlutoPoint point = PlutoPoint(0, 0));

	virtual void BeginPaint();
	virtual void EndPaint();
	virtual void UpdateRect(PlutoRectangle rect, PlutoPoint point=PlutoPoint(0,0));

	virtual void OnQuit();

	// Other
	virtual void Initialize(GraphicType Type, int iPK_Room=0, int iPK_EntertainArea=0);
	virtual void SetTime(char *ServerTimeString) {};
	
	void OnIdle();
	void WakeupFromCondWait();

#ifndef WIN32
	virtual void DoResetRatpoison() {}
#endif

public:
	SDL_Surface * m_pScreenImage;
	SDL_Surface * Screen;
	pluto_pthread_mutex_t m_GLThreadMutex;  
	class OrbiterGL3D *m_Desktop;
	
	bool PaintDesktop;	
	auto_ptr<PlutoGraphic> m_spBeforeGraphic;
	auto_ptr<PlutoGraphic> m_spAfterGraphic;
};

}
//-----------------------------------------------------------------------------------------------------
#endif //__OrbiterSDL_H__
