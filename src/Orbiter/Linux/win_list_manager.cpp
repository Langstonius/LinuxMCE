// Window List Manager

#include "win_list_manager.h"
#include "DCE/Logger.h"
using namespace DCE;

WinListManager::WinListManager(WMControllerImpl *pWMController, const string &sSdlWindowName)
        : m_WindowsMutex("Windows List Mutex")
        , m_pWMController(pWMController)
        , m_sSdlWindowName(sSdlWindowName)
		, m_bHideSdlWindow(false)
{
    pthread_mutexattr_init( &m_WindowsMutexAttr );
    pthread_mutexattr_settype( &m_WindowsMutexAttr,  PTHREAD_MUTEX_RECURSIVE_NP );
    m_WindowsMutex.Init(&m_WindowsMutexAttr);
}

WinListManager::~WinListManager()
{
    pthread_mutex_destroy(&m_WindowsMutex.mutex);
    pthread_mutexattr_destroy(&m_WindowsMutexAttr);
}

void WinListManager::ResetOrbiterWindow()
{
    PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);
    m_pWMController->SetLayer(m_sSdlWindowName, LayerAbove);
}

void WinListManager::ActivateWindow(const string& sWindowsName)
{
	PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);
	m_pWMController->ActivateWindow(sWindowsName);
}

void WinListManager::ActivateSdlWindow()
{
    PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);
#ifdef DEBUG
    g_pPlutoLogger->Write(LV_STATUS, "WinListManager::ActivateSdlWindow()");
#endif
    m_pWMController->ActivateWindow(m_sSdlWindowName);
}

void WinListManager::ShowSdlWindow(bool bExclusive)
{
    PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);
#ifdef DEBUG
    g_pPlutoLogger->Write(LV_STATUS, "WinListManager::ShowSdlWindow(%s)", bExclusive ? "true" : "false");
#endif
    string sLastWindow;
    for (list<string>::iterator it = m_listVisibleWindows.begin(); it != m_listVisibleWindows.end(); ++it)
        sLastWindow = *it;

	// when Orbiter is fullscreen no other dialog can be on top of
    // it, so it will be maximized instead
    m_pWMController->SetVisible(m_sSdlWindowName, !m_bHideSdlWindow);
    m_pWMController->SetMaximized(m_sSdlWindowName, true);
    m_pWMController->SetLayer(m_sSdlWindowName, bExclusive ? LayerAbove : LayerBelow);

    if (!m_bHideSdlWindow)
		ActivateSdlWindow();

    // TODO: possible bugfix: no FullScreen attribute
    m_pWMController->SetFullScreen(m_sSdlWindowName, bExclusive);

	if(m_sExternApplicationName != "")
	    m_pWMController->ActivateWindow(m_sExternApplicationName);

    return;
}

void WinListManager::ShowWindow(const string &sWindowName)
{
    PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);
#ifdef DEBUG
    g_pPlutoLogger->Write(LV_STATUS, "WinListManager::ShowWindow(%s)", sWindowName.c_str());
#endif
    m_pWMController->SetVisible(sWindowName, true);
    m_listVisibleWindows.push_back(sWindowName);
}

void WinListManager::MaximizeWindow(const string &sWindowName)
{
    PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);
#ifdef DEBUG
    g_pPlutoLogger->Write(LV_STATUS, "WinListManager::MaximizeWindow(%s)", sWindowName.c_str());
#endif
#ifdef ORBITER_OPENGL
    // TODO: possible bugfix: extra, bad call to LayerBelow
	m_pWMController->SetLayer(sWindowName, LayerBelow);
	m_pWMController->SetLayer(sWindowName, LayerNormal);
#endif	
    // TODO: possible bugfix: no FullScreen attribute
    // TODO: possible bugfix: replace SetFullScreen with SetMaximized
	m_pWMController->SetFullScreen(sWindowName, true);
    ShowWindow(sWindowName);
}

void WinListManager::PositionWindow(const string &sWindowName, int x, int y, int w, int h)
{
    PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);
#ifdef DEBUG
    g_pPlutoLogger->Write(LV_STATUS, "WinListManager::PositionWindow(%s)", sWindowName.c_str());
#endif
    m_pWMController->SetFullScreen(sWindowName, false);
	m_pWMController->SetMaximized(sWindowName, false);
	m_pWMController->SetPosition(sWindowName, x, y, w, h);
#ifdef ORBITER_OPENGL	
	m_pWMController->SetLayer(sWindowName, LayerBelow);
    m_pWMController->SetLayer(sWindowName, LayerNormal);
#endif	
    ShowWindow(sWindowName);
}

void WinListManager::HideAllWindows()
{
    PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);
#ifdef DEBUG
    g_pPlutoLogger->Write(LV_STATUS, "WinListManager::HideAllWindows()");
#endif
    for (list<string>::iterator it = m_listVisibleWindows.begin(); it != m_listVisibleWindows.end(); ++it)
    {
        m_pWMController->SetVisible(*it, false);
    }
    m_listVisibleWindows.clear();
}

string WinListManager::GetExternApplicationName()
{
    PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);
    return m_sExternApplicationName;
}

void WinListManager::SetExternApplicationName(const string &sWindowName)
{
#ifdef DEBUG
    g_pPlutoLogger->Write(LV_STATUS, "WinListManager::SetExternApplicationName(%s)", sWindowName.c_str());
#endif
    PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);
    m_sExternApplicationName = sWindowName;
}

void WinListManager::GetExternApplicationPosition(PlutoRectangle &coord)
{
    PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);
    coord = m_coordExternalApplication;
}

void WinListManager::SetExternApplicationPosition(const PlutoRectangle &coord)
{
#ifdef DEBUG
    g_pPlutoLogger->Write(LV_STATUS, "WinListManager::SetExternApplicationPosition(), name=%s", m_sExternApplicationName.c_str());
#endif
    PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);
    m_coordExternalApplication = coord;
}

bool WinListManager::IsEmpty()
{
    PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);
    return (m_listVisibleWindows.size() == 0);
}

bool WinListManager::IsWindowAvailable(const string &sClassName)
{
    PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);
    list<WinInfo> listWinInfo;
    if (! m_pWMController->ListWindows(listWinInfo))
        return false;
    for (list<WinInfo>::iterator it = listWinInfo.begin(); it !=  listWinInfo.end(); ++it)
    {
        if (string::npos != it->sClassName.find(sClassName))
            return true;
    }
    return false;
}

bool WinListManager::HideWindow(const string &sClassName)
{
	PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);
	m_pWMController->SetVisible(sClassName, false);
    return true;
}

void WinListManager::GetWindows(list<WinInfo>& listWinInfo)
{
	PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);
	m_pWMController->ListWindows(listWinInfo);
}

void WinListManager::SetSdlWindowVisibility(bool bValue)
{
	PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);
	m_bHideSdlWindow = !bValue;
	m_pWMController->SetVisible(m_sSdlWindowName, bValue);
}

