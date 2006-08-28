/*
  OrbiterLinux

  Copyright (C) 2004 Pluto, Inc., a Florida Corporation

  www.plutohome.com

  Phone: +1 (877) 758-8648

  This program is distributed according to the terms of the Pluto Public License, available at:
  http://plutohome.com/index.php?section=public_license

  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the Pluto Public License for more details.

*/

#include <stdlib.h>

#include "DCE/Logger.h"
#include "OrbiterLinux.h"
#include "pluto_main/Define_DesignObj.h"

#include <iomanip>
#include <sstream>
#include <sys/time.h>

#include <X11/Xutil.h>
#include <X11/Xproto.h>
#include <X11/keysymdef.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>

#include <SDL/SDL_syswm.h>

#include "pluto_main/Define_Button.h"
#include "PlutoUtils/PlutoButtonsToX.h"
#include "OSDScreenHandler.h"
#include "MouseBehavior_Linux.h"
#include "pluto_main/Define_DesignObjParameter.h"

#include "../CallBackTypes.h"

#include "../dialog_types.h"

using namespace std;

OrbiterLinux::OrbiterLinux(int DeviceID, int PK_DeviceTemplate,
                           string ServerAddress, string sLocalDirectory,
                           bool bLocalMode,
                           int nImageWidth, int nImageHeight,
						   bool bFullScreen)

        : Orbiter(DeviceID, PK_DeviceTemplate, ServerAddress, sLocalDirectory, bLocalMode,
                     nImageWidth, nImageHeight, NULL),
          // defaults
          /**
           * @hack to make it work for the short term. We need to find a way to set the class name properly or use the window ID if we can find it.
           * The reason this is hack is because there could be potentially multiple SDL_Applications running at the same time which could break the controlling code.
           */
          m_strWindowName("Orbiter"),
          m_strDisplayName(getenv("DISPLAY"))
    // initializations
        , m_pRecordHandler(NULL)
        , m_nDesktopWidth(0)
        , m_nDesktopHeight(0)
        , m_nProgressWidth(0)
        , m_nProgressHeight(0)
        , m_pProgressWnd(NULL)
        , m_pWaitGrid(NULL)
        , m_bButtonPressed_WaitGrid(false)
        , m_pWaitList(NULL)
        , m_bButtonPressed_WaitList(false)
        , m_pWaitUser(NULL)
        , m_bOrbiterReady(false)
        , m_bIsExclusiveMode(false)
        , m_pDisplay_SDL(NULL)
        , m_pWinListManager(NULL)
        , m_pWMController(NULL)
        , m_pX11(NULL)
{
    m_nProgressWidth = 400;
    m_nProgressHeight = 200;
    m_pWMController = new WMControllerImpl();
    m_pWinListManager = new WinListManager(m_pWMController, m_strWindowName);
}

void *HackThread(void *p)
{
    // For some reason X can fail to properly die????  TODO - HACK
    cout << "Inside Hackthread" << endl;
    Sleep(30000);
    cout << "Big problem -- this app should have died and didn't.  Send ourselves a seg fault so we log a coredump and can fix this" << endl;
    kill(getpid(), SIGSEGV);
    return NULL;
}

OrbiterLinux::~OrbiterLinux()
{
    pthread_t hackthread;
    pthread_create(&hackthread, NULL, HackThread, (void*)this);

    if (m_pProgressWnd)
    {
        m_pProgressWnd->Terminate();
        delete m_pProgressWnd;
    }

    KillMaintThread();

    Destroy();
}

void OrbiterLinux::HideOtherWindows()
{
	g_pPlutoLogger->Write(LV_WARNING, "OrbiterLinux::HideOtherWindows: Hidding other windows...");

	list<WinInfo> listWinInfo;
	m_pWinListManager->GetWindows(listWinInfo);

	for(list<WinInfo>::iterator it = listWinInfo.begin(); it != listWinInfo.end(); ++it)
	{
		string sClassName = it->sClassName;
        // skip the pointer-constrain window
        if (
            (sClassName == "pointer_constrain_name") ||
            (sClassName == "pointer_constrain_class") ||
            (sClassName == "pointer_constrain_name.pointer_constrain_class")
            )
        {
            continue;
        }
		if(sClassName != "" && string::npos == sClassName.find("Orbiter"))
		{
			g_pPlutoLogger->Write(LV_STATUS, "OrbiterLinux::HideOtherWindows, hidding %s", sClassName.c_str());
			m_pWinListManager->HideWindow(sClassName);
		}
	}
}

void OrbiterLinux::reinitGraphics()
{
    g_pPlutoLogger->Write(LV_WARNING, "OrbiterLinux::reinitGraphics()");

    m_pWinListManager->ShowSdlWindow(m_bIsExclusiveMode);

    OrbiterCallBack callback = (OrbiterCallBack)&OrbiterLinux::setInputFocusToMe;
    CallMaintenanceInMiliseconds( 3000, callback, NULL, pe_ALL );
}

void OrbiterLinux::setInputFocusToMe(void *)
{
    CallMaintenanceInMiliseconds( 7000, (OrbiterCallBack)&OrbiterLinux::setInputFocusToMe, NULL, pe_ALL ); // do this every 7 seconds
}

void OrbiterLinux::setWindowName(string strWindowName)
{
    m_strWindowName = strWindowName;
}

void OrbiterLinux::setDisplayName(string strDisplayName)
{
    m_strDisplayName = strDisplayName;
}

bool OrbiterLinux::X11_Init()
{
    if ((m_pX11 != NULL) && m_pX11->IsInitialized())
    {
        g_pPlutoLogger->Write(LV_WARNING, "OrbiterLinux::X11_Init(): already initialized");
        return true;
    }
    g_pPlutoLogger->Write(LV_STATUS, "OrbiterLinux::X11_Init()");

	// we need to use the same Display and Window with SDL
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version); // this is important!
    bool bResult = SDL_GetWMInfo(&info);
    if (bResult == false)
    {
        g_pPlutoLogger->Write(LV_WARNING, "OrbiterLinux::X11_Init() : error in SDL_GetWMInfo()");
    }
    g_pPlutoLogger->Write(LV_STATUS, "SDL uses pDisplay=%p, Window=%d", info.info.x11.display, info.info.x11.window);
    // save the SDL display
    m_pDisplay_SDL = info.info.x11.display;
    // initialize the X11wrapper
    g_pPlutoLogger->Write(LV_STATUS, "OrbiterLinux::X11_Init() : X11wrapper");
    m_pX11 = new X11wrapper();
    m_pX11->Assign_MainWindow(info.info.x11.window);
    m_pX11->Display_Open();
    m_pX11->GetDisplaySize(m_nDesktopWidth, m_nDesktopHeight);
    // initialize other classes
    g_pPlutoLogger->Write(LV_STATUS, "OrbiterLinux::X11_Init() : done");
    return true;
}

bool OrbiterLinux::X11_Exit()
{
    if (m_pX11 == NULL)
    {
        g_pPlutoLogger->Write(LV_CRITICAL, "OrbiterLinux::X11_Exit() : NULL pointer");
        return false;
    }
    g_pPlutoLogger->Write(LV_STATUS, "OrbiterLinux::X11_Exit()");
    // display was opened by SDL, it should be closed by SDL
    delete m_pX11;
    m_pX11 = NULL;
    m_pDisplay_SDL = NULL;
    g_pPlutoLogger->Write(LV_STATUS, "OrbiterLinux::X11_Exit() : done");
    return true;
}

Display * OrbiterLinux::GetDisplay()
{
    //m_pX11 should be created by now
	if(NULL == m_pX11)
		return NULL;

    return m_pX11->GetDisplay();
}

Window OrbiterLinux::GetMainWindow()
{
    //m_pX11 should be created by now
    return m_pX11->GetMainWindow();
}

Display * OrbiterLinux::GetDisplay_MainWindow()
{
    return m_pDisplay_SDL;
}

bool OrbiterLinux::RenderDesktop( class DesignObj_Orbiter *pObj, PlutoRectangle rectTotal, PlutoPoint point )
{
    m_bOrbiterReady = true;
    m_bIsExclusiveMode = false;

    //Orbiter::RenderDesktop(pObj, rectTotal, point);
    vector<int> vectButtonMaps;
    GetButtonsInObject(pObj,vectButtonMaps);

	bool bApplicationInBackground = pObj->m_mapObjParms_Find(DESIGNOBJPARAMETER_In_Background_CONST) == "1";
	
	g_pPlutoLogger->Write(LV_STATUS, "OrbiterLinux::RenderDesktop() : ptr=%p coord=[%d,%d,%d,%d], in background %d",
                          pObj,
                          rectTotal.X, rectTotal.Y, rectTotal.Width, rectTotal.Height,
						  bApplicationInBackground
                          );

	//if an application like xine has to be in background, we'll move orbiter to be on layer above.
	m_bIsExclusiveMode = bApplicationInBackground;

	m_pWinListManager->ShowSdlWindow(bApplicationInBackground);
    if (pObj->m_ObjectType == DESIGNOBJTYPE_App_Desktop_CONST)
    {
        //g_pPlutoLogger->Write(LV_CRITICAL,"OrbiterLinux::RenderDesktop rendering of %s",pObj->m_ObjectID.c_str());
        {
            // TODO : Set now playing is not sent in video wizard
            // we'll assume this is a xine for now
            if (m_pWinListManager->GetExternApplicationName() == "")
                m_pWinListManager->SetExternApplicationName("pluto-xine-playback-window.pluto-xine-playback-window");
        }
        m_pWinListManager->SetExternApplicationPosition(rectTotal);
        ActivateExternalWindowAsync(NULL);
    }

    g_pPlutoLogger->Write(LV_WARNING, "OrbiterLinux::RenderDesktop() : done");
    return true;
}

/*virtual*/ void OrbiterLinux::ActivateExternalWindowAsync(void *)
{
    string sWindowName = m_pWinListManager->GetExternApplicationName();
    PlutoRectangle rectTotal;
    m_pWinListManager->GetExternApplicationPosition(rectTotal);
	//g_pPlutoLogger->Write(LV_WARNING, "OrbiterLinux::ActivateExternalWindowAsync() : IsWindowAvailable(%s)", sWindowName.c_str());
    bool bIsWindowAvailable = m_pWinListManager->IsWindowAvailable(sWindowName);
	g_pPlutoLogger->Write(LV_WARNING, "OrbiterLinux::ActivateExternalWindowAsync() : IsWindowAvailable(%s) ==> %s", sWindowName.c_str(), bIsWindowAvailable ? "Yes, it is!" : "No, it's NOT!");
    if (bIsWindowAvailable)
    {
        if ( (rectTotal.Width == -1) && (rectTotal.Height == -1) )
        {
            g_pPlutoLogger->Write(LV_WARNING, "OrbiterLinux::ActivateExternalWindowAsync() : maximize sWindowName='%s'", sWindowName.c_str());
            m_pWinListManager->MaximizeWindow(sWindowName);
        }
        else
        {
            g_pPlutoLogger->Write(LV_WARNING, "OrbiterLinux::ActivateExternalWindowAsync() : position sWindowName='%s'", sWindowName.c_str());
            // HACK: stop activating xine or wxdialog several times per second
            // TODO: do the proper activating code for windows
            m_pWinListManager->PositionWindow(sWindowName, rectTotal.X, rectTotal.Y, rectTotal.Width, rectTotal.Height);
        }
        m_pWMController->ActivateWindow(m_pWinListManager->GetExternApplicationName());
    }
    else
        CallMaintenanceInMiliseconds( bIsWindowAvailable ? 1000 : 200, (OrbiterCallBack)&OrbiterLinux::ActivateExternalWindowAsync, NULL, pe_ALL );
}

// public interface implementations below

void OrbiterLinux::Initialize(GraphicType Type, int iPK_Room, int iPK_EntertainArea)
{
    g_pPlutoLogger->Write(LV_WARNING, "OrbiterLinux::Initialize()");
	Orbiter::Initialize(Type, iPK_Room, iPK_EntertainArea);

    //we know here the ui version!
    g_pPlutoLogger->Write(LV_WARNING, "Orbiter UI Version is %d", m_iUiVersion);

    if(UsesUIVersion2())
	{
		m_pMouseBehavior = new MouseBehavior_Linux(this);
	}
    m_pRecordHandler = new XRecordExtensionHandler(this, m_strDisplayName);
    m_pRecordHandler->enableRecording(true);

    reinitGraphics();

    g_pPlutoLogger->Write(LV_WARNING, "OrbiterLinux::Initialize() : done");
}

void OrbiterLinux::Destroy()
{
    g_pPlutoLogger->Write(LV_WARNING, "OrbiterLinux::Destroy()");
    delete m_pRecordHandler;
    m_pRecordHandler = NULL;
    if (m_pMouseBehavior)
    {
        delete m_pMouseBehavior;
        m_pMouseBehavior = NULL;
    }
    X11_Exit();
    delete m_pWinListManager;
    m_pWinListManager = NULL;
    delete m_pWMController;
    m_pWMController = NULL;
    g_pPlutoLogger->Write(LV_WARNING, "OrbiterLinux::Destroy() : done");
}

bool OrbiterLinux::PreprocessEvent(Orbiter::Event &event)
{
    if ( event.type != Orbiter::Event::BUTTON_DOWN && event.type != Orbiter::Event::BUTTON_UP )
        return false;

    XKeyEvent  kevent;
    KeySym   keysym;
    char   buf[1];

    kevent.type = KeyPress;
    kevent.display = GetDisplay();
    kevent.state = 0;
    kevent.keycode = event.data.button.m_iPK_Button;;
    XLookupString(&kevent, buf, sizeof(buf), &keysym, 0);

#ifdef ENABLE_MOUSE_BEHAVIOR
	if(UsesUIVersion2())
	{
		switch (event.data.button.m_iPK_Button)
		{
			case 182:  event.data.button.m_iPK_Button = BUTTON_Mouse_6_CONST; g_pPlutoLogger->Write(LV_CRITICAL, "Mouse button 6 (gyro) %s", event.type == Orbiter::Event::BUTTON_DOWN ? "down" : "up"); return true;
			case 183:  event.data.button.m_iPK_Button = BUTTON_Mouse_7_CONST; g_pPlutoLogger->Write(LV_CRITICAL, "Mouse button 7 (gyro) %s", event.type == Orbiter::Event::BUTTON_DOWN ? "down" : "up"); return true;
			case 184:  event.data.button.m_iPK_Button = BUTTON_Mouse_8_CONST; g_pPlutoLogger->Write(LV_CRITICAL, "Mouse button 8 (gyro) %s", event.type == Orbiter::Event::BUTTON_DOWN ? "down" : "up"); return true;

			default:
				break;
		}
	}
#endif

	printf("KEYCode: %d %x \n", (int)keysym, (unsigned int)keysym);

    switch ( keysym )
    {
        case XK_Shift_L:   case XK_Shift_R:
			event.data.button.m_iPK_Button = BUTTON_left_shift_CONST;
			g_pPlutoLogger->Write(LV_STATUS, "shift %s", event.type == Orbiter::Event::BUTTON_DOWN ? "down" : "up");
            m_bShiftDown = ( event.type == Orbiter::Event::BUTTON_DOWN );
			m_bShiftDownOnScreenKeyboard = false;
            break;
        case XK_Control_L:    case XK_Control_R:
            m_bControlDown = ( event.type == Orbiter::Event::BUTTON_DOWN );
            break;
        case XK_Alt_L:    case XK_Alt_R:
            m_bAltDown = ( event.type == Orbiter::Event::BUTTON_DOWN );
            break;
        case XK_Caps_Lock:
            if ( event.type == Orbiter::Event::BUTTON_UP )
                m_bCapsLock = ! m_bCapsLock;
            break;

        case XK_F1:     event.data.button.m_iPK_Button = BUTTON_F1_CONST; break;
        case XK_F2:     event.data.button.m_iPK_Button = BUTTON_F2_CONST; break;
        case XK_F3:     event.data.button.m_iPK_Button = BUTTON_F3_CONST; break;
        case XK_F4:     event.data.button.m_iPK_Button = BUTTON_F4_CONST; break;
        case XK_F5:     event.data.button.m_iPK_Button = BUTTON_F5_CONST; break;
		case XK_F6:     event.data.button.m_iPK_Button = BUTTON_F6_CONST; g_pPlutoLogger->Write(LV_CRITICAL, "Key F6 %s", event.type == Orbiter::Event::BUTTON_DOWN ? "down" : "up"); break;
		case XK_F7:     event.data.button.m_iPK_Button = BUTTON_F7_CONST; g_pPlutoLogger->Write(LV_CRITICAL, "Key F7 %s", event.type == Orbiter::Event::BUTTON_DOWN ? "down" : "up"); break;
		case XK_F8:     event.data.button.m_iPK_Button = BUTTON_F8_CONST; g_pPlutoLogger->Write(LV_CRITICAL, "Key F8 %s", event.type == Orbiter::Event::BUTTON_DOWN ? "down" : "up"); break;

        case XK_0: case XK_KP_0:
            if(m_bShiftDown)
                event.data.button.m_iPK_Button = BUTTON_right_parenthesis_CONST;
            else
                event.data.button.m_iPK_Button = BUTTON_0_CONST;
            break;
        case XK_1: case XK_KP_1:
            if(m_bShiftDown)
                event.data.button.m_iPK_Button = BUTTON_exclamation_point_CONST;
            else
                event.data.button.m_iPK_Button = BUTTON_1_CONST;
            break;
        case XK_2: case XK_KP_2:
            if(m_bShiftDown)
                event.data.button.m_iPK_Button = BUTTON_at_sign_CONST;
            else
                event.data.button.m_iPK_Button = BUTTON_2_CONST;
            break;
        case XK_3: case XK_KP_3:
           if(m_bShiftDown)
                event.data.button.m_iPK_Button = BUTTON_Pound_CONST;
            else
                event.data.button.m_iPK_Button = BUTTON_3_CONST;
            break;
        case XK_4: case XK_KP_4:
            if(m_bShiftDown)
                event.data.button.m_iPK_Button = BUTTON_dollar_CONST;
            else
                event.data.button.m_iPK_Button = BUTTON_4_CONST;
            break;
        case XK_5: case XK_KP_5:
            if(m_bShiftDown)
			{
                event.data.button.m_iPK_Button = BUTTON_percent_CONST;
				g_pPlutoLogger->Write(LV_STATUS, "% key");
			}
            else
			{
                event.data.button.m_iPK_Button = BUTTON_5_CONST;
				g_pPlutoLogger->Write(LV_STATUS, "5 key");
			}
            break;
        case XK_6: case XK_KP_6:
            if(m_bShiftDown)
                event.data.button.m_iPK_Button = BUTTON_caret_CONST;
            else
                event.data.button.m_iPK_Button = BUTTON_6_CONST;
            break;
        case XK_7: case XK_KP_7:
            if(m_bShiftDown)
                event.data.button.m_iPK_Button = BUTTON_ampersand_CONST;
            else
                event.data.button.m_iPK_Button = BUTTON_7_CONST;
            break;
        case XK_8: case XK_KP_8:
            if(m_bShiftDown)
                event.data.button.m_iPK_Button = BUTTON_Asterisk_CONST;
            else
                event.data.button.m_iPK_Button = BUTTON_8_CONST;
            break;
        case XK_9: case XK_KP_9:
            if(m_bShiftDown)
                event.data.button.m_iPK_Button = BUTTON_left_parenthesis_CONST;
            else
                event.data.button.m_iPK_Button = BUTTON_9_CONST;
            break;

        case XK_Up:
			if(m_bShiftDown)
				event.data.button.m_iPK_Button = BUTTON_Shift_Up_Arrow_CONST;
			else
				event.data.button.m_iPK_Button = BUTTON_Up_Arrow_CONST;
			break;
        case XK_Down:
			if(m_bShiftDown)
				event.data.button.m_iPK_Button = BUTTON_Shift_Down_Arrow_CONST;
			else
				event.data.button.m_iPK_Button = BUTTON_Down_Arrow_CONST;
			break;
        case XK_Left:
			if(m_bShiftDown)
				event.data.button.m_iPK_Button = BUTTON_Shift_Left_Arrow_CONST;
			else
		      	event.data.button.m_iPK_Button = BUTTON_Left_Arrow_CONST;
            break;
        case XK_Right:
			if(m_bShiftDown)
				event.data.button.m_iPK_Button = BUTTON_Shift_Right_Arrow_CONST;
			else
				event.data.button.m_iPK_Button = BUTTON_Right_Arrow_CONST;
			break;

/* modificari */
        case XK_F9:     event.data.button.m_iPK_Button = BUTTON_F9_CONST; break;
        case XK_F10:    event.data.button.m_iPK_Button = BUTTON_F10_CONST; break;
        case XK_Escape: event.data.button.m_iPK_Button = BUTTON_escape_CONST; break;
        case XK_Tab:    event.data.button.m_iPK_Button = BUTTON_tab_CONST; break;
        case XK_KP_Enter: case XK_Return:       event.data.button.m_iPK_Button = BUTTON_Enter_CONST;  break;
		case XK_KP_Space: case XK_space:        event.data.button.m_iPK_Button = BUTTON_space_CONST; break;
        case XK_BackSpace:      event.data.button.m_iPK_Button = BUTTON_Back_CONST;  break;
        case XK_underscore:     event.data.button.m_iPK_Button = BUTTON_underscore_CONST;  break;
        case XK_minus:  event.data.button.m_iPK_Button = BUTTON_dash_CONST;  break;
        case XK_equal:  case XK_KP_Equal:
			event.data.button.m_iPK_Button = BUTTON_equals_sign_CONST;
			break;
	case XK_slash:	event.data.button.m_iPK_Button = BUTTON_slash_CONST;  break;
        case XK_period: event.data.button.m_iPK_Button = BUTTON_dot_CONST;  break;
        case XK_comma:  case XK_KP_Separator:   event.data.button.m_iPK_Button =  BUTTON_comma_CONST; break;
        case XK_colon:  event.data.button.m_iPK_Button = BUTTON_colon_CONST; break;
        case XK_semicolon:      event.data.button.m_iPK_Button = BUTTON_semicolumn_CONST; break;
        case XK_quotedbl:       event.data.button.m_iPK_Button = BUTTON_double_quote_CONST; break;
			
/* end modificari */

        default:
            if ( XK_a <= keysym && keysym <= XK_z )
			{
				event.data.button.m_iPK_Button = BUTTON_a_CONST + keysym - XK_a;
			}
            else
            {
                event.type = Orbiter::Event::NOT_PROCESSED;
            }
    }

#ifdef DEBUG
    g_pPlutoLogger->Write(LV_STATUS, "The keysym was %d, the final event type %d, button %d", keysym, event.type, event.data.button.m_iPK_Button);
#endif

	return true;
}

void OrbiterLinux::CMD_Show_Mouse_Pointer(string sOnOff,string &sCMD_Result,Message *pMessage)
{
	if( sOnOff!="X" )
		return;

    // TODO: CMD_Show_Mouse_Pointer not implemented anywhere else

    // TODO: use this new code as reference
    //       probably this was the intention
    {
        //static bool bShow=false;
        //bShow = !bShow;
        //m_pMouseBehavior->ShowMouse(bShow);
    }

    // TODO: clean this code
    // what was written bellow does not work(now),
    // and does not make any sense, either
    // ..... log ..... GRABBING MOUSE window ?
    // ..... try to hide the mouse, but the cursor was not set to that window
    {
        //Display *dpy = GetDisplay();
        //Window win = DefaultRootWindow (dpy);

        //SDL_SysWMinfo sdlinfo;
        //SDL_VERSION(&sdlinfo.version);
        //int r2=SDL_GetWMInfo(&sdlinfo);
        //Window w2 = sdlinfo.info.x11.wmwindow;
        //Window w3 = sdlinfo.info.x11.window;
        //Window w4 = sdlinfo.info.x11.fswindow;
        //g_pPlutoLogger->Write(LV_CRITICAL, "GRABBING MOUSE window %d, r2: %d, w2: %d w3: %d w5: %d",(int) win,r2,(int) w2,(int) w3,(int) w4);

        //win = w3;
        //Pixmap blank;
        //XColor dummy;
        //char data[1] = {0};
        //Cursor cursor;

        ///* make a blank cursor */
        //X11_Locker lock(GetDisplay());
        //blank = XCreateBitmapFromData (dpy, win, data, 1, 1);
        //if(blank == None) fprintf(stderr, "error: out of memory.\n");
        //cursor = XCreatePixmapCursor(dpy, blank, blank, &dummy, &dummy, 0, 0);
        //XFreePixmap (dpy, blank);
    }
}

void OrbiterLinux::CMD_Off(int iPK_Pipe,string &sCMD_Result,Message *pMessage)
{
    g_pPlutoLogger->Write(LV_WARNING, "CMD off not implemented on the orbiter yet");
}

void OrbiterLinux::CMD_Activate_Window(string sWindowName,string &sCMD_Result,Message *pMessage)
{
    g_pPlutoLogger->Write(LV_WARNING, "OrbiterLinux::CMD_Activate_Window(%s)", sWindowName.c_str());
	m_pWinListManager->SetExternApplicationName(sWindowName);
}

void OrbiterLinux::CMD_Simulate_Keypress(string sPK_Button,string sName,string &sCMD_Result,Message *pMessage)
{
    g_pPlutoLogger->Write(LV_STATUS, "OrbiterLinux::CMD_Simulate_Keypress() : m_bYieldInput==%d", m_bYieldInput);
    if( m_bYieldInput )
    {
        pair<bool,int> XKeySym = PlutoButtonsToX(atoi(sPK_Button.c_str()));
        g_pPlutoLogger->Write(LV_WARNING, "Need to forward pluto key %s to X key %d (shift %d)", sPK_Button.c_str(),XKeySym.second,XKeySym.first);

        // TODO: clean this code
        Display *dpy = GetDisplay();
        X11_Locker lock(dpy);

		if( XKeySym.first )
            XTestFakeKeyEvent( dpy, XKeysymToKeycode(dpy, XK_Shift_L), True, 0 );

        XTestFakeKeyEvent( dpy, XKeysymToKeycode(dpy, XKeySym.second), True, 0 );
        XTestFakeKeyEvent( dpy, XKeysymToKeycode(dpy, XKeySym.second), False, 0 );

        if( XKeySym.first )
            XTestFakeKeyEvent( dpy, XKeysymToKeycode(dpy, XK_Shift_L), False, 0 );
    }
    Orbiter::CMD_Simulate_Keypress(sPK_Button,sName,sCMD_Result,pMessage);
}

void OrbiterLinux::CMD_Set_Mouse_Position_Relative(int iPosition_X,int iPosition_Y,string &sCMD_Result,Message *pMessage)
{
    g_pPlutoLogger->Write(LV_STATUS, "Moving mouse (relative %d,%d)", iPosition_X, iPosition_Y);
    m_pX11->Mouse_SetPosition(iPosition_X, iPosition_Y);
}

void OrbiterLinux::CMD_Simulate_Mouse_Click_At_Present_Pos(string sType,string &sCMD_Result,Message *pMessage)
{
    g_pPlutoLogger->Write(LV_STATUS, "Clicking mouse %s",sType.c_str());

    {
        Display *pDisplay = GetDisplay();
        X11_Locker lock(pDisplay);
        XTestFakeButtonEvent(pDisplay, 1, true, 0);
        XTestFakeButtonEvent(pDisplay, 1, false, 0);
    }
}

void OrbiterLinux::CMD_Surrender_to_OS(string sOnOff, bool bFully_release_keyboard, string &sCMD_Result, Message *pMessage)
{
    // when xine is full-screen, on DVD menu
    // and navigation in the dvd-menu with keyboard is wanted
    // then
    // sOnOff should be 1
    // and bFully_release_keyboard should be true
    g_pPlutoLogger->Write(LV_STATUS, "OrbiterLinux::CMD_Surrender_to_OS(%s, %d)", sOnOff.c_str(), bFully_release_keyboard);
    GrabPointer(sOnOff != "1");
    GrabKeyboard(sOnOff != "1");
    Orbiter::CMD_Surrender_to_OS(sOnOff, bFully_release_keyboard, sCMD_Result, pMessage);
    if (m_bYieldInput)
    {
        g_pPlutoLogger->Write(LV_STATUS, "OrbiterLinux::CMD_Surrender_to_OS() : ActivateWindow('%s')", m_pWinListManager->GetExternApplicationName().c_str());
        m_pWMController->ActivateWindow(m_pWinListManager->GetExternApplicationName());
    }
    g_pPlutoLogger->Write(LV_STATUS, "OrbiterLinux::CMD_Surrender_to_OS(%s, %d) : done", sOnOff.c_str(), bFully_release_keyboard);
}

/*virtual*/ ScreenHandler *OrbiterLinux::CreateScreenHandler()
{
	return new OSDScreenHandler(this, &m_mapDesignObj);
}

void OrbiterLinux::X_LockDisplay()
{
    m_pX11->Lock();
}

void OrbiterLinux::X_UnlockDisplay()
{
    m_pX11->Unlock();
}

void OrbiterLinux::GrabPointer(bool bEnable)
{
    g_pPlutoLogger->Write(LV_STATUS, "OrbiterLinux::GrabPointer(%d)", bEnable);
    if (bEnable)
    {
        m_pX11->Mouse_Grab(GetMainWindow());
    }
    else
    {
        m_pX11->Mouse_Ungrab();
    }
    g_pPlutoLogger->Write(LV_STATUS, "OrbiterLinux::GrabPointer(%d) : done", bEnable);
}

void OrbiterLinux::GrabKeyboard(bool bEnable)
{
    g_pPlutoLogger->Write(LV_STATUS, "OrbiterLinux::GrabKeyboard(%d)", bEnable);
    if (bEnable)
    {
        m_pX11->Keyboard_Grab(GetMainWindow());
    }
    else
    {
        m_pX11->Keyboard_Ungrab();
    }
    g_pPlutoLogger->Write(LV_STATUS, "OrbiterLinux::GrabKeyboard(%d) : done", bEnable);
}
