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

#include "DCE/Logger.h"
#include "OrbiterLinux.h"
#include "pluto_main/Define_DesignObj.h"

#include <iomanip>
#include <sstream>
#include <sys/time.h>

#include <X11/Xutil.h>
#include <X11/Xproto.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>

#include <SDL/SDL_syswm.h>

#include "pluto_main/Define_Button.h"
#include "PlutoUtils/PlutoButtonsToX.h"
#include "OSDScreenHandler.h"

using namespace std;

OrbiterLinux::OrbiterLinux(int DeviceID, int PK_DeviceTemplate,
                           string ServerAddress, string sLocalDirectory,
                           bool bLocalMode,
                           int nImageWidth, int nImageHeight)
    : OrbiterSDL(DeviceID, PK_DeviceTemplate, ServerAddress, sLocalDirectory, bLocalMode, nImageWidth, nImageHeight),
      // defaults
      /**
       * @hack to make it work for the short term. We need to find a way to set the class name properly or use the window ID if we can find it.
       * The reason this is hack is because there could be potentially multiple SDL_Applications running at the same time which could break the controlling code.
       */
      m_strWindowName("Orbiter"),
      m_strDisplayName(getenv("DISPLAY")),

      // initializations
      desktopInScreen(0),
      XServerDisplay(NULL),
      m_pProgressWnd(NULL)
{
  XInitThreads();
  openDisplay();

  m_pRecordHandler = new XRecordExtensionHandler(m_strDisplayName);

  m_nProgressWidth = 400;
  m_nProgressHeight = 200;

  m_sCurrentAppDesktopName = "";


  // Disable DPMS and screen saver
  system("/usr/bin/X11/xset -display :0 -dpms s off");
}

void *HackThread(void *p)
{
  // For some reason X can fail to properly die????  TODO - HACK
  cout << "Inside Hacktrhead" << endl;
  Sleep(10000);
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

  delete m_pRecordHandler;
  closeDisplay();
}

void OrbiterLinux::reinitGraphics()
{
  if ( ! XServerDisplay && ! openDisplay() )
    return;

  commandRatPoison(":set winname class");
  commandRatPoison(":desktop off");

  commandRatPoison(string(":select ") + m_strWindowName);
  commandRatPoison(":desktop on");
  commandRatPoison(":keystodesktop on");
  commandRatPoison(":keybindings off");
  setDesktopVisible(false);

  OrbiterCallBack callback = (OrbiterCallBack)&OrbiterLinux::setInputFocusToMe;
  CallMaintenanceInMiliseconds( 3000, callback, NULL, pe_ALL );
}

void OrbiterLinux::setInputFocusToMe(void *)
{
  if ( ! m_bYieldInput )
    commandRatPoison(":keystodesktop on");

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

bool OrbiterLinux::openDisplay()
{
  XServerDisplay = XOpenDisplay(m_strDisplayName.c_str());

  int currentScreen;

  if ( XServerDisplay == NULL )
    return false;

  XLockDisplay(XServerDisplay);
  currentScreen = XDefaultScreen(XServerDisplay);
  m_nDesktopWidth = DisplayWidth(XServerDisplay, currentScreen);
  m_nDesktopHeight = DisplayHeight(XServerDisplay, currentScreen);
  XUnlockDisplay(XServerDisplay);
  return true;
}

bool OrbiterLinux::closeDisplay()
{
  if ( XServerDisplay )
    XCloseDisplay(XServerDisplay);

  XServerDisplay = NULL;

  return true;
}


Display *OrbiterLinux::getDisplay()
{
  if ( ! XServerDisplay )
    openDisplay();

  return XServerDisplay;
}

Window OrbiterLinux::getWindow()
{
  return 0;
}

bool OrbiterLinux::RenderDesktop( class DesignObj_Orbiter *pObj, PlutoRectangle rectTotal, PlutoPoint point )
{
  {
    if(pObj->m_ObjectType == DESIGNOBJTYPE_wxWidgets_Applet_CONST)
      SetCurrentAppDesktopName("dialog");
    else if(pObj->m_ObjectType == DESIGNOBJTYPE_App_Desktop_CONST)
      SetCurrentAppDesktopName("pluto-xine-playback-window");
  }

  vector<int> vectButtonMaps;
  GetButtonsInObject(pObj,vectButtonMaps);

  resizeMoveDesktop(rectTotal.Left(), rectTotal.Top(),
                    rectTotal.Right() - rectTotal.Left(),
                    rectTotal.Bottom() - rectTotal.Top());

  desktopInScreen = true;
  return true;
}

// public interface implementations below
bool OrbiterLinux::resizeMoveDesktop(int x, int y, int width, int height)
{
  if ( ! m_bYieldInput )
  {
    m_pRecordHandler->enableRecording(this, false);
    commandRatPoison(":keystodesktop on");
  }
  else
  {
    commandRatPoison(":keystodesktop off");

    // patch the rectangle to match the actual resolution
    width = m_nDesktopWidth;
    height = m_nDesktopHeight;
    m_pRecordHandler->enableRecording(this);
  }

  if(m_sCurrentAppDesktopName != "")
    commandRatPoison(":select " + m_sCurrentAppDesktopName);

  stringstream commandLine;
  commandLine << ":set padding " << x << " " << y << " "
              << m_nDesktopWidth - x - width << " " << m_nDesktopHeight - y - height;

  commandRatPoison(commandLine.str());
  commandRatPoison(":redisplay");

  return true;
}

bool OrbiterLinux::setDesktopVisible(bool visible)
{
  if ( ! visible )
    return resizeMoveDesktop(m_nDesktopWidth, m_nDesktopHeight, 10, 10);
  else
    return resizeMoveDesktop(0, 0, m_nDesktopWidth - m_iImageWidth, m_nDesktopHeight - m_iImageHeight);
}

void OrbiterLinux::Initialize(GraphicType Type, int iPK_Room, int iPK_EntertainArea)
{
  OrbiterSDL::Initialize(Type,iPK_Room,iPK_EntertainArea);
  reinitGraphics();
}

void OrbiterLinux::RenderScreen()
{
  if ( XServerDisplay == NULL && ! openDisplay() )
  {
    g_pPlutoLogger->Write(LV_WARNING, "Couldn't open the display: \"%s\"", m_strDisplayName.c_str());

    return;
  }

  desktopInScreen = false;

  OrbiterSDL::RenderScreen();

  if ( false == desktopInScreen )
    setDesktopVisible(false);

  XFlush(XServerDisplay);
}

/**
   void OrbiterLinux::RenderObject(
   DesignObj_Controller *pObj, DesignObj_Controller *pObjAttr,
   int XOffset, int YOffset)
   {
   if ( pObj->m_ObjectType == C_DESIGNOBJTYPE_APP_DESKTOP_CONST )
   {
   desktopInScreen = true;

   resizeMoveDesktop(
   pObj->m_rPosition.Left(),
   pObj->m_rPosition.Top(),
   pObj->m_rPosition.Width,
   pObj->m_rPosition.Height);

   return;
   }

   ControllerImageSDL::RenderObject(pObj, pObjAttr, XOffset, YOffset);
   }
*/

bool OrbiterLinux::PreprocessEvent(Orbiter::Event &event)
{
  if ( event.type != Orbiter::Event::BUTTON_DOWN && event.type != Orbiter::Event::BUTTON_UP )
    return false;

  XKeyEvent  kevent;
  KeySym   keysym;
  char   buf[1];

  kevent.type = KeyPress;
  kevent.display = getDisplay();
  kevent.state = 0;
  kevent.keycode = event.data.button.m_iPK_Button;;
  XLookupString(&kevent, buf, sizeof(buf), &keysym, 0);

  switch ( keysym )
  {
    case XK_F1:     event.data.button.m_iPK_Button = BUTTON_F1_CONST; break;
    case XK_F2:     event.data.button.m_iPK_Button = BUTTON_F2_CONST; break;
    case XK_F3:     event.data.button.m_iPK_Button = BUTTON_F3_CONST; break;
    case XK_F4:     event.data.button.m_iPK_Button = BUTTON_F4_CONST; break;
    case XK_F5:     event.data.button.m_iPK_Button = BUTTON_F5_CONST; break;

    case XK_0: case XK_KP_0:    event.data.button.m_iPK_Button = BUTTON_0_CONST; break;
    case XK_1: case XK_KP_1:    event.data.button.m_iPK_Button = BUTTON_1_CONST; break;
    case XK_2: case XK_KP_2:    event.data.button.m_iPK_Button = BUTTON_2_CONST; break;
    case XK_3: case XK_KP_3:    event.data.button.m_iPK_Button = BUTTON_3_CONST; break;
    case XK_4: case XK_KP_4:    event.data.button.m_iPK_Button = BUTTON_4_CONST; break;
    case XK_5: case XK_KP_5:    event.data.button.m_iPK_Button = BUTTON_5_CONST; break;
    case XK_6: case XK_KP_6:    event.data.button.m_iPK_Button = BUTTON_6_CONST; break;
    case XK_7: case XK_KP_7:    event.data.button.m_iPK_Button = BUTTON_7_CONST; break;
    case XK_8: case XK_KP_8:    event.data.button.m_iPK_Button = BUTTON_8_CONST; break;
    case XK_9: case XK_KP_9:    event.data.button.m_iPK_Button = BUTTON_9_CONST; break;

    case XK_Up:         event.data.button.m_iPK_Button = BUTTON_Up_Arrow_CONST; break;
    case XK_Down:       event.data.button.m_iPK_Button = BUTTON_Down_Arrow_CONST; break;
    case XK_Left:       event.data.button.m_iPK_Button = BUTTON_Left_Arrow_CONST; break;
    case XK_Right:      event.data.button.m_iPK_Button = BUTTON_Right_Arrow_CONST; break;

    default:
      if ( XK_a <= keysym && keysym <= XK_z )
        event.data.button.m_iPK_Button = BUTTON_a_CONST + keysym - XK_a;
      else if ( XK_A <= keysym && keysym <= XK_Z )
        event.data.button.m_iPK_Button = BUTTON_A_CONST + keysym - XK_A;
      else
        event.type = Orbiter::Event::NOT_PROCESSED;
  }

#ifdef DEBUG
  g_pPlutoLogger->Write(LV_STATUS, "The keysym was %d, the final event type %d", keysym, event.type);
#endif
}

void OrbiterLinux::CMD_Show_Mouse_Pointer(string sOnOff,string &sCMD_Result,Message *pMessage)
{
  // not need this anymore
  // in Xine_Player : pointer_hide, pointer_show
  //bool bShow = sOnOff=="1";
  //if ( bShow )
  //    commandRatPoison(":banish off");
  //else
  //    commandRatPoison(":banish on");
}

void OrbiterLinux::CMD_Off(int iPK_Pipe,string &sCMD_Result,Message *pMessage)
{
  g_pPlutoLogger->Write(LV_WARNING, "CMD off not implemented on the orbiter yet");
}

void OrbiterLinux::CMD_Activate_Window(string sName,string &sCMD_Result,Message *pMessage)
{
  commandRatPoison(string(":select ") + sName);
}

void OrbiterLinux::CMD_Simulate_Keypress(string sPK_Button,string sName,string &sCMD_Result,Message *pMessage)
{
  if( m_bYieldInput )
  {
    pair<bool,int> XKeySym = PlutoButtonsToX(atoi(sPK_Button.c_str()));
    g_pPlutoLogger->Write(LV_WARNING, "Need to forward pluto key %s to X key %d (shift %d)", sPK_Button.c_str(),XKeySym.second,XKeySym.first);

    Display *dpy = XOpenDisplay (NULL);
    Window rootwindow = DefaultRootWindow (dpy);

    if( XKeySym.first )
      XTestFakeKeyEvent( dpy, XKeysymToKeycode(dpy, XK_Shift_L), True, 0 );

    XTestFakeKeyEvent( dpy, XKeysymToKeycode(dpy, XKeySym.second), True, 0 );
    XTestFakeKeyEvent( dpy, XKeysymToKeycode(dpy, XKeySym.second), False, 0 );

    if( XKeySym.first )
      XTestFakeKeyEvent( dpy, XKeysymToKeycode(dpy, XK_Shift_L), False, 0 );

    XCloseDisplay(dpy);
  }
  Orbiter::CMD_Simulate_Keypress(sPK_Button,sName,sCMD_Result,pMessage);
}

void OrbiterLinux::CMD_Set_Mouse_Position_Relative(int iPosition_X,int iPosition_Y,string &sCMD_Result,Message *pMessage)
{
  Display *dpy = XOpenDisplay (NULL);
  Window rootwindow = DefaultRootWindow (dpy);
  g_pPlutoLogger->Write(LV_STATUS, "Moving mouse (relative %d,%d)",iPosition_X,iPosition_Y);

  XWarpPointer(dpy, rootwindow,0 , 0, 0, 0, 0, iPosition_X, iPosition_Y);
  XCloseDisplay(dpy);
}

void OrbiterLinux::CMD_Simulate_Mouse_Click_At_Present_Pos(string sType,string &sCMD_Result,Message *pMessage)
{
  g_pPlutoLogger->Write(LV_STATUS, "Clicking mouse %s",sType.c_str());

  XTestFakeButtonEvent(XServerDisplay, 1, true, 0);
  XTestFakeButtonEvent(XServerDisplay, 1, false, 0);
  Display *dpy = XOpenDisplay (NULL);
  Window rootwindow = DefaultRootWindow (dpy);
  XTestFakeButtonEvent(dpy, 1, true, 0);
  XTestFakeButtonEvent(dpy, 1, false, 0);
  XCloseDisplay(dpy);
}

bool OrbiterLinux::DisplayProgress(string sMessage, const map<string, bool> &mapChildDevices, int nProgress)
{
  std::cout << "== DisplayProgress( " << sMessage << ", " << nProgress << " );" << std::endl;

  if (m_pProgressWnd && m_pProgressWnd->IsCancelled())
  {
    delete m_pProgressWnd;
    m_pProgressWnd = NULL;
    return true;
  }

  if (nProgress != -1 && !m_pProgressWnd) {
    // Create the progress window ...
    m_pProgressWnd = new XProgressWnd();
    m_pProgressWnd->UpdateProgress(sMessage, nProgress);
    m_pProgressWnd->Run();
    commandRatPoison(":set winname class");
    commandRatPoison(":desktop off");

    commandRatPoison(string(":select ") + m_pProgressWnd->m_wndName);
    commandRatPoison(":desktop on");
    commandRatPoison(":keystodesktop on");
    commandRatPoison(":keybindings off");
    setDesktopVisible(false);
  } else if (nProgress != -1) {
    // Update progress info
    m_pProgressWnd->UpdateProgress(sMessage, nProgress);
    m_pProgressWnd->DrawWindow();
  } else if(m_pProgressWnd) {
    // We are done here ...
    m_pProgressWnd->Terminate();
    m_pProgressWnd = NULL;
    reinitGraphics();
  }

  return false;
}

bool OrbiterLinux::DisplayProgress(string sMessage, int nProgress)
{
  std::cout << "== DisplayProgress( " << sMessage << ", " << nProgress << " );" << std::endl;

  if (m_pProgressWnd && m_pProgressWnd->IsCancelled())
  {
    delete m_pProgressWnd;
    m_pProgressWnd = NULL;
    return true;
  }

  if (nProgress != -1 && !m_pProgressWnd) {
    // Create the progress window ...
    m_pProgressWnd = new XProgressWnd();
    m_pProgressWnd->UpdateProgress(sMessage, nProgress);
    m_pProgressWnd->Run();
    commandRatPoison(":set winname class");
    commandRatPoison(":desktop off");

    commandRatPoison(string(":select ") + m_pProgressWnd->m_wndName);
    commandRatPoison(":desktop on");
    commandRatPoison(":keystodesktop on");
    commandRatPoison(":keybindings off");
    setDesktopVisible(false);
  } else if (nProgress != -1) {
    // Update progress info
    m_pProgressWnd->UpdateProgress(sMessage, nProgress);
    m_pProgressWnd->DrawWindow();
  } else if(m_pProgressWnd) {
    // We are done here ...
    m_pProgressWnd->Terminate();
    m_pProgressWnd = NULL;
    reinitGraphics();
  }

  return false;
}

int OrbiterLinux::PromptUser(string sPrompt,int iTimeoutSeconds,map<int,string> *p_mapPrompts)
{
#if 0
  return PROMPT_CANCEL;
#else
  map<int,string> mapPrompts;
  mapPrompts[PROMPT_CANCEL]    = "Ok";
  if (p_mapPrompts == NULL) {
    p_mapPrompts = &mapPrompts;
  }

  XPromptUser promptDlg(sPrompt, iTimeoutSeconds, p_mapPrompts);
  promptDlg.SetButtonPlacement(XPromptUser::BTN_VERT);

  promptDlg.Init();
  commandRatPoison(":set winname class");
  commandRatPoison(":desktop off");

  commandRatPoison(string(":select ") + promptDlg.m_wndName);
  commandRatPoison(":desktop on");
  commandRatPoison(":keystodesktop on");
  commandRatPoison(":keybindings off");
  setDesktopVisible(false);

  int nUserAnswer = promptDlg.RunModal();

  promptDlg.DeInit();

  return nUserAnswer;
#endif
}

ScreenHandler *OrbiterLinux::CreateScreenHandler()
{
	return new OSDScreenHandler(this, &m_mapDesignObj);
}

void OrbiterLinux::SetCurrentAppDesktopName(string sName)
{
  g_pPlutoLogger->Write( LV_WARNING, "OrbiterLinux::SetCurrentAppDesktopName(%s)", sName.c_str());
	m_sCurrentAppDesktopName = sName;
}
