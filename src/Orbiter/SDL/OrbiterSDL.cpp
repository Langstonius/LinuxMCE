/*
 OrbiterSDL

 Copyright (C) 2004 Pluto, Inc., a Florida Corporation

 www.plutohome.com

 Phone: +1 (877) 758-8648

 This program is distributed according to the terms of the Pluto Public License, available at:
 http://plutohome.com/index.php?section=public_license

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the Pluto Public License for more details.

 */

#include "PlutoUtils/CommonIncludes.h"
#include "PlutoUtils/MultiThreadIncludes.h"
#include "DCE/Logger.h"
#include "OrbiterSDL.h"
#include "SerializeClass/ShapesColors.h"

#include "PlutoSDLDefs.h"
#include "../RendererOCG.h"
#include "SDLRendererOCGHelper.h"

#include "pluto_main/Define_Text.h"
#include "pluto_main/Define_DeviceCategory.h"
#include "Gen_Devices/AllCommandsRequests.h"
#include "DataGrid.h"
#include "SDLGraphic.h"
#include "Splitter/TextWrapper.h"
#include "Orbiter/ScreenHistory.h"

#include "SDL_rotozoom.h"
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <sge.h>
#include <sge_surface.h>

#if !defined(BLUETOOTH_DONGLE) && !defined(PROXY_ORBITER)
#define USE_ONLY_SCREEN_SURFACE
#endif

#ifdef WINCE
    #include "wince.h"
#endif

bool g_bResettingVideoMode;
void *HackThread2(void *p)
{
    OrbiterSDL *pOrbiterSDL = (OrbiterSDL *) p;
    // For some reason X can fail to properly die????  TODO - HACK
    g_pPlutoLogger->Write(LV_STATUS,"Inside Hacktrhead #2");
    Sleep(2000);
    if( g_bResettingVideoMode )
    {
        g_pPlutoLogger->Write(LV_CRITICAL,"Another stupid problem since the kernel upgrade.  SDL_SetVideoMode can just hang on init.  It happened. Kill ourselves");
#ifndef WIN32
        kill(getpid(), SIGSEGV);
        pOrbiterSDL->DoResetRatpoison();
#endif
    }
    return NULL;
}


using namespace DCE;
//-----------------------------------------------------------------------------------------------------
OrbiterSDL::OrbiterSDL(int DeviceID, int PK_DeviceTemplate, string ServerAddress, string sLocalDirectory,
    bool bLocalMode, int nImageWidth, int nImageHeight, bool bFullScreen/*=false*/,
    pluto_pthread_mutex_t *pExternalScreenMutex/*=NULL*/)
 : Orbiter(DeviceID, PK_DeviceTemplate, ServerAddress, sLocalDirectory, bLocalMode, nImageWidth,
    nImageHeight, pExternalScreenMutex)
{
    m_pScreenImage = NULL;
    m_bFullScreen=bFullScreen;
}
//----------------------------------------------------------------------------------------------------
/*virtual*/ bool OrbiterSDL::GetConfig()
{
    if(!Orbiter::GetConfig())
        return false;

    //initializing the engine...
    Uint32 uSDLInitFlags = SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE;

    if(m_bFullScreen)
        uSDLInitFlags |= SDL_FULLSCREEN;

    if (SDL_Init(uSDLInitFlags) == -1)
    {
#ifndef WINCE
                    cerr << "Failed initializing SDL: " << SDL_GetError() << endl;
#else
                    printf("Failed to initialize SDL %s\n", SDL_GetError());
#endif //WINCE

#ifndef WIN32 //linux
        string sCmd = "/usr/pluto/bin/Start_X.sh; /usr/pluto/bin/Start_ratpoison.sh";
        g_pPlutoLogger->Write(LV_CRITICAL, "X is not running! Starting X and ratpoison: %s", sCmd.c_str());
        system(sCmd.c_str());
#endif //linux

        exit(1);
    }

    SDL_WM_SetCaption("OrbiterSDL", "OrbiterSDL");

    atexit(SDL_Quit);
    g_pPlutoLogger->Write(LV_STATUS, "Initialized SDL");

#ifdef USE_ONLY_SCREEN_SURFACE
    Uint32 uVideoModeFlags = SDL_SWSURFACE;

#if !defined(WIN32) || defined(WINCE)
    if(m_bFullScreen)
        uVideoModeFlags |= SDL_FULLSCREEN;
#endif

    g_bResettingVideoMode=true;
    pthread_t hackthread;
    pthread_create(&hackthread, NULL, HackThread2, (void*)this);

    if ((Screen = SDL_SetVideoMode(m_iImageWidth, m_iImageHeight, 0, uVideoModeFlags)) == NULL)
    {
        g_pPlutoLogger->Write(LV_WARNING, "Failed to set video mode (%d x %d): %s", m_iImageWidth, m_iImageHeight, SDL_GetError());
        exit(1);
    }
    g_bResettingVideoMode=false;
#endif

    g_pPlutoLogger->Write(LV_STATUS, "Set video mode to %d x %d Window.", m_iImageWidth, m_iImageHeight);

#ifdef USE_ONLY_SCREEN_SURFACE
    m_pScreenImage = Screen;
#else
    m_pScreenImage = SDL_CreateRGBSurface(SDL_SWSURFACE, m_iImageWidth, m_iImageHeight, 32, rmask, gmask, bmask, amask);
    if (m_pScreenImage == NULL)
    {
        g_pPlutoLogger->Write(LV_WARNING, "SDL_CreateRGBSurface failed! %s",SDL_GetError());
    }
#endif
    m_bWeCanRepeat = true;

    g_pPlutoLogger->Write(LV_STATUS, "Created back screen surface!");
    return true;
}
//----------------------------------------------------------------------------------------------------
/*virtual*/ OrbiterSDL::~OrbiterSDL()
{
    KillMaintThread(); // We need to do this before freeing the surface.  It's a repeat of what's in Orbiter's destructor
g_pPlutoLogger->Write(LV_STATUS, "about to free surface");

#ifndef USE_ONLY_SCREEN_SURFACE
    SDL_FreeSurface(m_pScreenImage);
#endif

    m_pScreenImage = NULL;

g_pPlutoLogger->Write(LV_STATUS, "~OrbiterSDL finished");
}
//-----------------------------------------------------------------------------------------------------
/*virtual*/ void OrbiterSDL::RenderScreen()
{
#ifdef DEBUG
    g_pPlutoLogger->Write(LV_STATUS,"$$$ RENDER SCREEN $$$ %s",(m_pScreenHistory_Current ? m_pScreenHistory_Current->GetObj()->m_ObjectID.c_str() : " NO SCREEN"));
#endif

    if (m_pScreenHistory_Current)
    {
        PLUTO_SAFETY_LOCK(cm, m_ScreenMutex);
        SDL_FillRect(m_pScreenImage, NULL, SDL_MapRGBA(m_pScreenImage->format, 0, 0, 0, 255));
    }

    Orbiter::RenderScreen();
    DisplayImageOnScreen(m_pScreenImage);
}
//-----------------------------------------------------------------------------------------------------
/*virtual*/ void OrbiterSDL::DisplayImageOnScreen(SDL_Surface *m_pScreenImage)
{
    PLUTO_SAFETY_LOCK(cm,m_ScreenMutex);

#ifndef USE_ONLY_SCREEN_SURFACE
    SDL_BlitSurface(m_pScreenImage, NULL, Screen, NULL);
#endif

    SDL_UpdateRect(Screen, 0, 0, 0, 0);
}
//-----------------------------------------------------------------------------------------------------
/*virtual*/ void OrbiterSDL::RedrawObjects()
{
    //PLUTO_SAFETY_LOCK(cm,m_ScreenMutex);
    Orbiter::RedrawObjects();
}
//-----------------------------------------------------------------------------------------------------
/*virtual*/ void OrbiterSDL::RenderText(string &TextToDisplay,DesignObjText *Text,TextStyle *pTextStyle, PlutoPoint point)
{
    SDL_Rect TextLocation;
    TextLocation.x = point.X + Text->m_rPosition.X;
    TextLocation.y = point.Y + Text->m_rPosition.Y;
    TextLocation.w = Text->m_rPosition.Width;
    TextLocation.h = Text->m_rPosition.Height;

#ifdef WIN32
    string BasePath="C:\\Windows\\Fonts\\";
#else
    string BasePath="/usr/pluto/fonts/";
#endif //win32

    try
    {
        WrapAndRenderText(m_pScreenImage, TextToDisplay, TextLocation.x, TextLocation.y, TextLocation.w, TextLocation.h, BasePath,
            pTextStyle,Text->m_iPK_HorizAlignment,Text->m_iPK_VertAlignment, &m_mapTextStyle);
    }
    catch(...)
    {
    }
}
//-----------------------------------------------------------------------------------------------------
/*virtual*/ void OrbiterSDL::HollowRectangle(int X, int Y, int Width, int Height, PlutoColor color)
{
	//clipping
	if(X + Width >= m_iImageWidth)
		Width = m_iImageWidth - X - 1;

	if(Y + Height >= m_iImageHeight)
		Height = m_iImageHeight - Y - 1;

    sge_Rect(m_pScreenImage,X,Y,Width + X,Height + Y,color.m_Value);
}

//-----------------------------------------------------------------------------------------------------
/*virtual*/ void OrbiterSDL::SolidRectangle(int x, int y, int width, int height, PlutoColor color, int Opacity /*= 100*/)
{
	//clipping
	if(x + width >= m_iImageWidth)
		width = m_iImageWidth - x - 1;

	if(y + height >= m_iImageHeight)
		height = m_iImageHeight - y - 1;

    SDL_Rect Rectangle;
    Rectangle.x = x; Rectangle.y = y; Rectangle.w = width; Rectangle.h = height;

    SDL_FillRect(m_pScreenImage, &Rectangle, SDL_MapRGBA(m_pScreenImage->format, color.R(), color.G(), color.B(), color.A()));
}

//-----------------------------------------------------------------------------------------------------
/*virtual*/ void OrbiterSDL::RenderGraphic(PlutoGraphic *pPlutoGraphic, PlutoRectangle rectTotal,
    bool bDisableAspectRatio, PlutoPoint point)
{
    if(!pPlutoGraphic || pPlutoGraphic->GraphicType_get() != gtSDLGraphic)
        return;//nothing to render or not an sdl graphic

    if(pPlutoGraphic->IsEmpty())
        return;

    SDLGraphic *pSDLGraphic = (SDLGraphic *) pPlutoGraphic;
    SDL_Surface *pSDL_Surface = pSDLGraphic->m_pSDL_Surface;

    //render the sdl surface
    SDL_Rect Destination;
    Destination.x = point.X + rectTotal.X;
    Destination.y = point.Y + rectTotal.Y;
    Destination.w = rectTotal.Width;
    Destination.h = rectTotal.Height;

    if(pSDL_Surface->w == 0 || pSDL_Surface->h == 0) //nothing to render
        return; //malformated image?
    else
        if(pSDL_Surface->w != rectTotal.Width || pSDL_Surface->h != rectTotal.Height) //different size. we'll have to stretch the surface
        {
            double ZoomX = 1;
            double ZoomY = 1;

            SDL_Surface *rotozoom_picture;

            if(bDisableAspectRatio) //no aspect ratio kept
            {
                ZoomX = rectTotal.Width / double(pSDL_Surface->w);
                ZoomY = rectTotal.Height / double(pSDL_Surface->h);
            }
            else //we'll have to keep the aspect
            {
                ZoomX = ZoomY = min(rectTotal.Width / double(pSDL_Surface->w),
                    rectTotal.Height / double(pSDL_Surface->h));
            }

            rotozoom_picture = zoomSurface(pSDL_Surface, ZoomX, ZoomY, SMOOTHING_ON);

            SDL_BlitSurface(rotozoom_picture, NULL, m_pScreenImage, &Destination);
            SDL_FreeSurface(rotozoom_picture);
        }
        else //same size ... just blit the surface
            SDL_BlitSurface(pSDL_Surface, NULL, m_pScreenImage, &Destination);
}
//-----------------------------------------------------------------------------------------------------
/*virtual*/ void OrbiterSDL::SaveBackgroundForDeselect(DesignObj_Orbiter *pObj, PlutoPoint point)
{
    SDL_Rect SourceRect;
    SourceRect.x = point.X + pObj->m_rPosition.Left(); SourceRect.y = point.Y + pObj->m_rPosition.Top();
    SourceRect.w = pObj->m_rPosition.Width; SourceRect.h = pObj->m_rPosition.Height;

    SDL_Surface *pSDL_Surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
        pObj->m_rPosition.Width, pObj->m_rPosition.Height, 32, rmask, gmask, bmask, amask);

    SDL_BlitSurface(m_pScreenImage, &SourceRect, pSDL_Surface, NULL);
    SDL_SetAlpha(pSDL_Surface, SDL_RLEACCEL , SDL_ALPHA_OPAQUE);

    pObj->m_pGraphicToUndoSelect = new SDLGraphic(pSDL_Surface);
}

PlutoGraphic *OrbiterSDL::GetBackground( PlutoRectangle &rect )
{
	//clipping
	if(rect.X + rect.Width >= m_iImageWidth)
		rect.Width = m_iImageWidth - rect.X - 1;

	if(rect.Y + rect.Height >= m_iImageHeight)
		rect.Height = m_iImageHeight - rect.Y - 1;


    SDL_Surface *pSDL_Surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
        rect.Width, rect.Height, 32, rmask, gmask, bmask, amask);

    SDL_Rect SourceRect;
    SourceRect.x = rect.Left(); SourceRect.y = rect.Top();
    SourceRect.w = rect.Width; SourceRect.h = rect.Height;

    SDL_SetAlpha(m_pScreenImage, 0, 0);
    SDL_BlitSurface(m_pScreenImage, &SourceRect, pSDL_Surface, NULL);

    return new SDLGraphic(pSDL_Surface);
}


//-----------------------------------------------------------------------------------------------------
void OrbiterSDL::Initialize(GraphicType Type, int iPK_Room, int iPK_EntertainArea)
{
    Orbiter::Initialize(Type, iPK_Room, iPK_EntertainArea);
}
//-----------------------------------------------------------------------------------------------------
void OrbiterSDL::ReplaceColorInRectangle(int x, int y, int width, int height, PlutoColor ColorToReplace, PlutoColor ReplacementColor)
{
    SDL_PixelFormat * PF = m_pScreenImage->format;
    Uint32 PlutoPixelDest, PlutoPixelSrc, Pixel;

#ifdef DEBUG
    g_pPlutoLogger->Write(LV_STATUS, "ReplaceColor: %u %u %u : %u %u %u",
        ColorToReplace.R(), ColorToReplace.G(), ColorToReplace.B(),
        ReplacementColor.R(), ReplacementColor.G(), ReplacementColor.B());
#endif

    PlutoPixelSrc = (ColorToReplace.R() << PF->Rshift) | (ColorToReplace.G() << PF->Gshift) | (ColorToReplace.B() << PF->Bshift) | (ColorToReplace.A() << PF->Ashift);
    unsigned char *Source = (unsigned char *) &PlutoPixelSrc;
    PlutoPixelDest = ReplacementColor.R() << PF->Rshift | ReplacementColor.G() << PF->Gshift | ReplacementColor.B() << PF->Bshift;//  TODO -- this should work | ReplacementColor.A() << PF->Ashift;

    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            // we may need locking on the surface
            Pixel = SDLGraphic::getpixel(m_pScreenImage, i + x, j + y);
            unsigned char *pPixel = (unsigned char *) &Pixel;
            const int max_diff = 3;
            if ( abs(Source[0]-pPixel[0])<max_diff && abs(Source[1]-pPixel[1])<max_diff && abs(Source[2]-pPixel[2])<max_diff && abs(Source[3]-pPixel[3])<max_diff )
            {
                SDLGraphic::putpixel(m_pScreenImage,i + x, j + y, PlutoPixelDest);
            }

        }
    }

    PlutoRectangle rect(x, y, width, height);
}
//-----------------------------------------------------------------------------------------------------
/*virtual*/ void OrbiterSDL::BeginPaint()
{
}
//-----------------------------------------------------------------------------------------------------
/*virtual*/ void OrbiterSDL::EndPaint()
{
    //if we are using a buffer surface to blit images and text (Bluetooth_Dongle uses this)
    //will have to update the hole screen
    //if not, the user will call UpdateRect function for each rectangle he must invalidate
#ifndef USE_ONLY_SCREEN_SURFACE
    DisplayImageOnScreen(m_pScreenImage);
#endif
}
//-----------------------------------------------------------------------------------------------------
/*virtual*/ void OrbiterSDL::OnQuit()
{
    g_pPlutoLogger->Write(LV_WARNING,"Got an on quit.  Pushing an event into SDL");
    m_bQuit = true;
    pthread_cond_broadcast( &m_listMessageQueueCond );
    SDL_Event *pEvent = new SDL_Event;
    pEvent->type = SDL_QUIT;
    SDL_PushEvent(pEvent);
    delete pEvent;
}
//-----------------------------------------------------------------------------------------------------
/*virtual*/ PlutoGraphic *OrbiterSDL::CreateGraphic()
{
    return new SDLGraphic(this);
}
//-----------------------------------------------------------------------------------------------------
/*virtual*/ void OrbiterSDL::UpdateRect(PlutoRectangle rect, PlutoPoint point)
{
    //clipping the rectangle

    PlutoRectangle localrect = rect;
    localrect.X += point.X;
    localrect.Y += point.Y;

    if(localrect.X < 0)
        localrect.X = 0;

    if(localrect.Y < 0)
        localrect.Y = 0;

    if(localrect.Right() >= m_Width)
        localrect.Width = m_Width - rect.X - 1;

    if(localrect.Bottom() >= m_Height)
        localrect.Height = m_Height - localrect.Y - 1;

    PLUTO_SAFETY_LOCK(cm,m_ScreenMutex);

#ifdef USE_ONLY_SCREEN_SURFACE
    SDL_UpdateRect(Screen, localrect.Left(), localrect.Top(), localrect.Width, localrect.Height);
#endif
}
//-----------------------------------------------------------------------------------------------------
