/*
     Copyright (C) 2004 Pluto, Inc., a Florida Corporation

     www.plutohome.com

     Phone: +1 (877) 758-8648
 

     This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License.
     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
     of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

     See the GNU General Public License for more details.

*/
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
#include "XProgressWnd.h"
#include "DCE/Logger.h"

#include <X11/Xutil.h>

using namespace DCE;

XProgressWnd::XProgressWnd()
 : m_pButton(NULL)
 , m_bCanceled(false)
 , m_bDone(false)
 , m_bDestroy(false)
{
    m_nTextX = 0;
    m_nTextY = 0;
    m_nBarX = 0;
    m_nBarY = 0;
    m_nBarWidth = 0;
    m_nBarHeight = 0;

    m_sText = "";
    m_nProgress = 0;

	m_pDisplay = NULL;
}


XProgressWnd::~XProgressWnd()
{
    if (m_pButton) delete m_pButton;
    LoggerWrapper::GetInstance()->Write(LV_STATUS, "XProgressWnd::~XProgressWnd(): pDisplay=%d ->Closing", m_pDisplay);
    if (m_pDisplay) XCloseDisplay(m_pDisplay);
    LoggerWrapper::GetInstance()->Write(LV_STATUS, "XProgressWnd::~XProgressWnd(): pDisplay=%d ->Closed", m_pDisplay);
}


bool XProgressWnd::ShowWindow(bool bShow/* = true*/)
{
    if (!m_wndThis) return false;

    if (m_pButton)
        m_pButton->ShowWindow(bShow);

    X3DWindow::ShowWindow(bShow);
    if (bShow)
    {
        XClassHint classHint;
        classHint.res_name = "progress_window";
        classHint.res_class = "progress_window";
        XSetClassHint(m_pDisplay, m_wndThis, &classHint);
        XStoreName(m_pDisplay, m_wndThis, "progress_window");
        XSync(m_pDisplay, false);
    }
    return true;
}

bool XProgressWnd::DrawWindow()
{
    //XLockDisplay(m_pDisplay);

    bool bRet = X3DWindow::DrawWindow();

    if (bRet && m_pButton) {
        bRet = m_pButton->DrawWindow();
    }

    if (bRet) 
	{
        unsigned long mask = GCForeground | GCBackground | GCLineWidth;
        XGCValues values;
		
		const int nDesktopBackground = 0x000080;
		const int nBarBorderColor = 0xF0F0F0;
		const int nBarBorderShadowColor = 0x000000;
		const int nBarBackgroundColor = 0xFFFF80;
		const int nBarFillColor = 0x00A040;
		const int nBarBorderWidth = 2;
		const int nBarBorderShadowDepth = 4;
		const int nBarBorderShadowWidth = 6;
        int nBarWidth = (m_nBarWidth - nBarBorderWidth * 2)  * m_nProgress / 100;	
				
		//bar - background
        values.line_width = nBarBorderWidth;
        values.foreground = nDesktopBackground;
        values.background = nDesktopBackground;
        GC gcBackground = XCreateGC(m_pDisplay, m_wndThis, mask, &values);
        XFillRectangle(m_pDisplay, m_wndThis, gcBackground, 2, 2, m_nWidth-4, m_nHeight-4);
		XFreeGC(m_pDisplay, gcBackground);
		
		//bar - border - shadow
		values.line_width = nBarBorderShadowWidth;
        values.foreground = nBarBorderShadowColor;
        values.background = nBarBorderShadowColor;
        GC gcBorderShadow = XCreateGC(m_pDisplay, m_wndThis, mask, &values);
        XDrawRectangle(m_pDisplay, m_wndThis, gcBorderShadow, 
			m_nBarX + nBarBorderShadowDepth, m_nBarY + nBarBorderShadowDepth, 
			m_nBarWidth, m_nBarHeight);
		XFreeGC(m_pDisplay, gcBorderShadow);
				
		//bar - border
		values.line_width = 0;
        values.background = nBarBorderColor;
		values.foreground = nBarBorderColor;
        GC gcBorder = XCreateGC(m_pDisplay, m_wndThis, mask, &values);
        XDrawRectangle(m_pDisplay, m_wndThis, gcBorder, m_nBarX, m_nBarY, m_nBarWidth, m_nBarHeight);	
		XFreeGC(m_pDisplay, gcBorder);
		
		//bar - background color
        values.background = nBarBackgroundColor;
		values.foreground = nBarBackgroundColor;
        GC gcBarBackground = XCreateGC(m_pDisplay, m_wndThis, mask, &values);
        XFillRectangle(m_pDisplay, m_wndThis, gcBarBackground, m_nBarX+2, m_nBarY+2, 
			m_nBarWidth-4, m_nBarHeight-4);
		XFreeGC(m_pDisplay, gcBarBackground);
		
		//bar - fill color
        values.background = nBarFillColor;
		values.foreground = nBarFillColor;
        GC gcBar = XCreateGC(m_pDisplay, m_wndThis, mask, &values);
        XFillRectangle(m_pDisplay, m_wndThis, gcBar, m_nBarX + 2, m_nBarY + 2, 
			nBarWidth - 4 > 0 ? nBarWidth - 4 : 0, m_nBarHeight - 4);
		XFreeGC(m_pDisplay, gcBar);
		
		//render caption text
        Font font = XLoadFont(m_pDisplay, "-*-helvetica-bold-R-Normal--*-240-75-75-*-*");
        mask = GCForeground | GCBackground | GCLineWidth | GCFont;
        values.foreground = 0xFFFFFF;
        values.background = 0xFFFFFF;
        values.font = font;
        GC gcText = XCreateGC(m_pDisplay, m_wndThis, mask, &values);
        XDrawString(m_pDisplay, m_wndThis, gcText, m_nTextX, m_nTextY, m_sText.c_str(), m_sText.length());
		XUnloadFont(m_pDisplay, font);
		XFreeGC(m_pDisplay, gcText);

		//render progress text		
        font = XLoadFont(m_pDisplay, "-*-helvetica-bold-R-Normal--*-240-75-75-*-*");
        mask = GCForeground | GCBackground | GCLineWidth | GCFont;
        values.foreground = 0x000000;
        values.background = 0x000000;
        values.font = font;
        gcText = XCreateGC(m_pDisplay, m_wndThis, mask, &values);
		string sPercentText = StringUtils::ltos(m_nProgress) + "%";
		XDrawString(m_pDisplay, m_wndThis, gcText, m_nBarX + m_nBarWidth / 2, m_nBarY + 30, sPercentText.c_str(), sPercentText.length());
		XUnloadFont(m_pDisplay, font);
		XFreeGC(m_pDisplay, gcText);		
    }

    //XUnlockDisplay(m_pDisplay);
    return bRet;
}

bool XProgressWnd::EventLoop()
{
    XEvent event;
    while (not m_bDone) {
        if (XCheckIfEvent(m_pDisplay, &event, &CheckIfEvent, (char*)this)) {
        	//process event ...
		if (LoggerWrapper::GetInstance()) LoggerWrapper::GetInstance()->Write(LV_STATUS, "Event received");

        	switch (event.type) {
        	case Expose:
		    if (LoggerWrapper::GetInstance()) LoggerWrapper::GetInstance()->Write(LV_STATUS, "Expose event received");
        	    DrawWindow();
        	    break;
        	case ButtonRelease:
		    if (LoggerWrapper::GetInstance()) LoggerWrapper::GetInstance()->Write(LV_STATUS, "ButtonRelease event received");
        	    XButtonEvent *pButtonEvent = (XButtonEvent *)&event;
        	    if (m_pButton && m_pButton->HitTest(pButtonEvent->x, pButtonEvent->y)) {
                	m_bDone = true;
			m_bCanceled = true;
        	    }
        	    break;
        	}
	} else {
		usleep(250000);
	}
    }

    return true;
}

Bool XProgressWnd::CheckIfEvent(Display *pDisplay, XEvent *pEvent, XPointer arg)
{
    XProgressWnd *pThis = (XProgressWnd *)arg;
    XAnyEvent *pAnyEvent = (XAnyEvent *)pEvent;

    bool bFound = false;
    if (pAnyEvent->window == pThis->m_wndThis) bFound = true;
    std::list<Window>::iterator it = pThis->m_childs.begin();
    while (!bFound && it != pThis->m_childs.end()) {
        if (*it == pAnyEvent->window) {
            bFound = true;
            break;
        }
        it++;
    }

    return true;
}

bool XProgressWnd::UpdateProgress(std::string sText, int nProgress)
{
   if (nProgress == -1) {
      m_bDone = true;
   }

    m_sText = sText;
    m_nProgress = nProgress;

    return true;
}

static void *MyThreadFunc(void *pWindow)
{
    if (LoggerWrapper::GetInstance()) LoggerWrapper::GetInstance()->Write(LV_STATUS, "Thread func start ...");
    XProgressWnd *pWnd = (XProgressWnd *)pWindow;

    pWnd->EventLoop();

    if (LoggerWrapper::GetInstance()) LoggerWrapper::GetInstance()->Write(LV_STATUS, "Thread func ending ...");
    pWnd->DestroyWindow();

	Display * pDisplay = pWnd->GetDisplay();
	if (pDisplay)
	    XSync(pDisplay, false);
    if (LoggerWrapper::GetInstance()) LoggerWrapper::GetInstance()->Write(LV_STATUS, "Thread func ended.");

    if (pWnd->Destroy())
    {
    	delete pWnd;
    }

    return NULL;
}

pthread_t XProgressWnd::Run()
{
    pthread_t threadID;
    if (LoggerWrapper::GetInstance()) LoggerWrapper::GetInstance()->Write(LV_STATUS, "Starting the thread ...");
    m_bCanceled = false;
    m_bDone = false;

	// don't put this in the thread (again) as that creates a race condition; leave it here :)
    Display *pDisplay = XOpenDisplay(NULL); // needs XCloseDisplay?
    LoggerWrapper::GetInstance()->Write(LV_STATUS, "XProgressWnd::Run(): pDisplay=%d ->Opened", pDisplay);
    int nScreenNo = DefaultScreen(pDisplay);
    int nDesktopX, nDesktopY;
    nDesktopX = DisplayWidth(pDisplay, nScreenNo);
    nDesktopY = DisplayHeight(pDisplay, nScreenNo);
    CreateWindow(pDisplay, nScreenNo, DefaultRootWindow(pDisplay), 0, 0, nDesktopX, nDesktopY);
    ShowWindow();
    DrawWindow();

    int iResult = pthread_create( &threadID, NULL, MyThreadFunc, (void *)this );
    if ( iResult != 0 )
    {
	if (LoggerWrapper::GetInstance()) LoggerWrapper::GetInstance()->Write(LV_CRITICAL, "Fatal error: Cannot start thread!");
    }
    else
    {
        if (LoggerWrapper::GetInstance())
            LoggerWrapper::GetInstance()->Write(LV_STATUS, "Thread started ...");

        // need to join later
        //pthread_detach(threadID);
    }

    m_thisThread = threadID;
    return threadID;
}

void XProgressWnd::Terminate()
{
	m_bDestroy = true;
	m_bDone = true;

    // we need this to avoid a race condition between orbiter's thread
    // sending command to ratpoison and this dialog thread doing
    // X cleanup code like XSync
    pthread_join(m_thisThread, NULL);
}

int XProgressWnd::CreateWindow(Display *pDisplay, int screen, Window wndParent, int x, int y, int cx, int cy)
{
    X3DWindow::CreateWindow(pDisplay, screen, wndParent, x, y, cx, cy);

    m_nTextX = 20;
    m_nTextY = m_nWidth / 2;
    m_nBarX = 20;
    m_nBarY = m_nHeight - 100;
    m_nBarWidth = m_nWidth - 40;
    m_nBarHeight = 50;

    if (LoggerWrapper::GetInstance()) LoggerWrapper::GetInstance()->Write(LV_STATUS, "Constructing ProgressWindow");

    m_wndName = "Progress";
    XClassHint *pClassHint = XAllocClassHint();
    pClassHint->res_name = (char *)m_wndName.c_str();
    pClassHint->res_class = (char *)m_wndName.c_str();
    XSetClassHint(pDisplay, m_wndThis, pClassHint);
    XFree(pClassHint);
    return 0;
}

Display * XProgressWnd::GetDisplay()
{
	return m_pDisplay;
}
