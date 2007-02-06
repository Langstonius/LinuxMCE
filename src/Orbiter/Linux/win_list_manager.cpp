// Window List Manager

#include "win_list_manager.h"
#include "WindowContext.h"
#include "DCE/Logger.h"
using namespace DCE;

WinListManager::WinListManager(const string &sSdlWindowName)
        : m_WindowsMutex("Windows List Mutex")
		, m_bHideSdlWindow(false)
        , m_sSdlWindowName(sSdlWindowName)
{
    pthread_mutexattr_init( &m_WindowsMutexAttr );
    pthread_mutexattr_settype( &m_WindowsMutexAttr,  PTHREAD_MUTEX_RECURSIVE_NP );
    m_WindowsMutex.Init(&m_WindowsMutexAttr);

	m_pWMController = new WMControllerImpl();
	m_bExternalChange=false;
	m_bKeepSdlWindowActive=false;
}

WinListManager::~WinListManager()
{
	g_pPlutoLogger->Write(LV_WARNING, "WinListManager: deleting Window controller");
	delete m_pWMController;
	m_pWMController = NULL;

    pthread_mutex_destroy(&m_WindowsMutex.mutex);
    pthread_mutexattr_destroy(&m_WindowsMutexAttr);
}

bool WinListManager::ResetOrbiterWindow()
{
#ifdef MAEMO_NOKIA770
	//no window manager action with nokia 770
	return true;
#endif

    PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);
    //special code for xfwm
    g_pPlutoLogger->Write(LV_STATUS, "Special code for xfwm : setting Orbiter to below, then above layer!");

	const int cnMaxRetries = 100;
	for(int i = 0; i < cnMaxRetries; ++i)
	{
		g_pPlutoLogger->Write(LV_STATUS, "Trying to set Orbiter to above layer! (%d)", i);
		if(m_pWMController->SetLayer(m_sSdlWindowName, LayerBelow) && m_pWMController->SetLayer(m_sSdlWindowName, LayerAbove))
		{
			g_pPlutoLogger->Write(LV_STATUS, "Orbiter is on the above layer now!");
			return true;
		}

		Sleep(10);
	}

    g_pPlutoLogger->Write(LV_CRITICAL, "Failed to set Orbiter to above layer!");
    return false;
}

void WinListManager::ActivateWindow(const string& sWindowsName)
{
	PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);
	PendingContext(sWindowsName).Activate(true);
}

void WinListManager::ActivateSdlWindow()
{
    PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);
	PendingContext(m_sSdlWindowName).Activate(true);
}

void WinListManager::SetLayer(const string &sClassName, WindowLayer layer)
{
	PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);
	PendingContext(sClassName).Layer(layer);
}

void WinListManager::ShowSdlWindow(bool bExclusive, bool bYieldInput)
{
    PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);

	// when Orbiter is fullscreen no other dialog can be on top of
    // it, so it will be maximized instead
	PendingContext(m_sSdlWindowName).Visible(!m_bHideSdlWindow);
	PendingContext(m_sSdlWindowName).Maximize(true);
	PendingContext(m_sSdlWindowName).Layer(bExclusive ? LayerAbove : LayerBelow);

    if(!m_bHideSdlWindow)
		PendingContext(m_sSdlWindowName).Activate(true);

    if (bYieldInput)
    {
		if(m_sExternApplicationName != "")
		{
			PendingContext(m_sExternApplicationName).Layer(bExclusive ? LayerBelow : LayerAbove); 
			PendingContext(m_sExternApplicationName).Activate(true);
		}
    }
}

void WinListManager::ShowWindow(const string &sWindowName)
{
#ifdef DEBUG
	g_pPlutoLogger->Write(LV_STATUS, "WinListManager::ShowWindow %s",sWindowName.c_str());
#endif
    PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);
	PendingContext(sWindowName).Visible(true);
}

void WinListManager::MaximizeWindow(const string &sWindowName)
{
    PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);
	PendingContext(sWindowName).Maximize(true);
    PendingContext(sWindowName).Visible(true);
}

void WinListManager::PositionWindow(const string &sWindowName, int x, int y, int w, int h)
{
    PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);
	PendingContext(sWindowName).FullScreen(false);
	PendingContext(sWindowName).Maximize(false);
	PendingContext(sWindowName).Position(PlutoRectangle(x, y, w, h));
	PendingContext(sWindowName).Visible(true);
}

void WinListManager::HideAllWindows()
{
#ifdef DEBUG
	g_pPlutoLogger->Write(LV_STATUS, "WinListManager::HideAllWindows");
#endif
	
	PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);
	for(WindowsContext::iterator it_pending = m_PendingContext.begin(); it_pending != m_PendingContext.end(); ++it_pending)
	{
		if( StringUtils::StartsWith(it_pending->first,"Orbiter",true) )  // Doesn't apply to orbiter
			continue;
		it_pending->second.Visible(false);
	}
/*
	Hmm...  This reports windows in a x.y, and we just use x, so we always re-hide windows even that are not supposed to be
	for(map<unsigned long,string>::iterator it=m_mapKnownWindows.begin();it!=m_mapKnownWindows.end();++it)
	{
		if( StringUtils::StartsWith(it->second,"Orbiter",true) )  // Doesn't apply to orbiter
			continue;
		HideWindow(it->second);
	}
*/
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
#ifdef DEBUG
	g_pPlutoLogger->Write(LV_STATUS, "WinListManager::ShowWindow %s",sClassName.c_str());
#endif
	PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);
	PendingContext(sClassName).Visible(false);
    return true;
}

void WinListManager::GetWindows(list<WinInfo>& listWinInfo)
{
	PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);
	m_pWMController->ListWindows(listWinInfo);

#ifdef DEBUG
for(map<unsigned long,string>::iterator it=m_mapKnownWindows.begin();it!=m_mapKnownWindows.end();++it)
g_pPlutoLogger->Write(LV_STATUS,"WinListManager::GetWindows1 %d/%s",it->first,it->second.c_str());

for(list<WinInfo>::iterator it = listWinInfo.begin(); it != listWinInfo.end(); ++it)
{
g_pPlutoLogger->Write(LV_WARNING,"WinListManager::GetWindows1b id %d desktop %d class %s title %s",it->ulWindowId,it->lDesktop,it->sClassName.c_str(),it->sTitle.c_str());
}
#endif

	bool bChangesDetected=false;

	map<unsigned long,bool> mapFoundWindows;  // Put all known windows in here with 'false' and mark them as true as we find them
	for(map<unsigned long,string>::iterator it=m_mapKnownWindows.begin();it!=m_mapKnownWindows.end();++it)
		mapFoundWindows[ it->first ] = false;

	for(list<WinInfo>::iterator it = listWinInfo.begin(); it != listWinInfo.end(); ++it)
	{
		map<unsigned long,string>::iterator itKnownWindows=m_mapKnownWindows.find( it->ulWindowId );
		if( itKnownWindows == m_mapKnownWindows.end() || itKnownWindows->second != it->sClassName )
		{
#ifdef DEBUG
			g_pPlutoLogger->Write(LV_STATUS,"WinListManager::GetWindows windows %d-%s is new",
				it->ulWindowId,it->sClassName.c_str());
#endif
			m_mapKnownWindows[it->ulWindowId] = it->sClassName;
			bChangesDetected=true;
		}
		else
		{
#ifdef DEBUG
g_pPlutoLogger->Write(LV_WARNING,"WinListManager::GetWindows %d found %d",it->ulWindowId,itKnownWindows == m_mapKnownWindows.end());
#endif
			mapFoundWindows[ it->ulWindowId ] = true;
		}
	}

	for(map<unsigned long,bool>::iterator it=mapFoundWindows.begin();it!=mapFoundWindows.end();++it)
	{
		if( it->second==false )
		{
			bChangesDetected=true;
#ifdef DEBUG
			g_pPlutoLogger->Write(LV_STATUS,"WinListManager::GetWindows windows %d-%s is gone",
				it->first,m_mapKnownWindows[it->first].c_str());
#endif
			m_mapKnownWindows.erase( it->first );
		}
	}

#ifdef DEBUG
for(map<unsigned long,string>::iterator it=m_mapKnownWindows.begin();it!=m_mapKnownWindows.end();++it)
g_pPlutoLogger->Write(LV_WARNING,"WinListManager::GetWindows2 %d/%s",it->first,it->second.c_str());
#endif
	if( bChangesDetected )
	{
		g_pPlutoLogger->Write(LV_STATUS,"WinListManager::GetWindows m_bExternalChange set");
		m_bExternalChange = true;
	}
}

void WinListManager::SetSdlWindowVisibility(bool bValue)
{
	PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);
	m_bHideSdlWindow = !bValue;
	PendingContext(m_sSdlWindowName).Visible(bValue);
}

void WinListManager::ApplyContext(string sExternalWindowName/*=""*/)
{
#ifdef MAEMO_NOKIA770
	//no window manager action with nokia 770
	return;
#endif

	PLUTO_SAFETY_LOCK(cm, m_WindowsMutex);
	list<WinInfo> listWinInfo;
	GetWindows(listWinInfo);  // Do this first since it may set m_bExternalChange

#ifdef DEBUG
	g_pPlutoLogger->Write(LV_STATUS,"WinListManager::ApplyContext m_bExternalChange %d",(int) m_bExternalChange);
#endif
	
	string sExternalWindowToActivate;
	for(WindowsContext::iterator it_pending = m_PendingContext.begin(); it_pending != m_PendingContext.end(); ++it_pending)
	{
		string sWindowName = it_pending->first;
		WindowContext &pending_context = it_pending->second;

		if(sWindowName.empty())
			continue;

		if(!sExternalWindowName.empty() && sWindowName != sExternalWindowName)
			continue;

		bool bResult = true;  // Will be set to false if any of them fail
		WindowsContext::iterator it_current = m_CurrentContext.find(sWindowName);
		if(it_current != m_CurrentContext.end() && m_bExternalChange==false && pending_context.IsErrorFlag()==false )
		{
			WindowContext &current_context = it_current->second;
#ifdef DEBUG
			g_pPlutoLogger->Write(LV_STATUS, "WinListManager::ApplyContext: applying partial context for '%s': %s vs %s",
				sWindowName.c_str(), pending_context.ToString().c_str(), current_context.ToString().c_str());
#endif

			if(current_context != pending_context)
				g_pPlutoLogger->Write(LV_WARNING, "WinListManager::ApplyContext: applying diff context for '%s': %s",
					sWindowName.c_str(), pending_context.ToString().c_str());

			if(pending_context.Layer() != current_context.Layer())
				bResult = bResult && m_pWMController->SetLayer(sWindowName, pending_context.Layer());

			if(pending_context.IsMaximized() != current_context.IsMaximized())
				bResult = bResult && m_pWMController->SetMaximized(sWindowName, pending_context.IsMaximized());

			if(pending_context.IsFullScreen() != current_context.IsFullScreen())
				bResult = bResult && m_pWMController->SetFullScreen(sWindowName, pending_context.IsFullScreen());


			if(pending_context.IsVisible() != current_context.IsVisible())
				bResult = bResult && m_pWMController->SetVisible(sWindowName, pending_context.IsVisible());

			PlutoRectangle rect = pending_context.Position();
			PlutoRectangle current_rect = current_context.Position();
			if(
				!pending_context.IsMaximized() && !pending_context.IsFullScreen() && 
				rect.Width != -1 && rect.Height != -1 
				&& 
				(
					rect.X != current_rect.X || rect.Y != current_rect.Y ||
					rect.Width != current_rect.Width || rect.Height != current_rect.Height 
				)
			)
				bResult = bResult && m_pWMController->SetPosition(sWindowName, rect.X, rect.Y, rect.Width, rect.Height);

			if(pending_context.IsActivated() && !current_context.IsActivated() )
			{
				if(sWindowName != m_sSdlWindowName)
					sExternalWindowToActivate = sWindowName;

				bResult = bResult && m_pWMController->ActivateWindow(sWindowName);
			}

			if(!pending_context.IsVisible())
			{
				//reset position
                pending_context.Position(PlutoRectangle(-1, -1, -1, -1));
			}
		}
		else
		{
			g_pPlutoLogger->Write(LV_STATUS, "WinListManager::ApplyContext: applying whole context for '%s': %s (ExternalChange: %d ErrorFlag %d)",
				sWindowName.c_str(), pending_context.ToString().c_str(),(int) m_bExternalChange,(int) pending_context.IsErrorFlag());

			bResult = bResult && m_pWMController->SetLayer(sWindowName, pending_context.Layer());
			bResult = bResult && m_pWMController->SetMaximized(sWindowName, pending_context.IsMaximized());
			bResult = bResult && m_pWMController->SetFullScreen(sWindowName, pending_context.IsFullScreen());
			bResult = bResult && m_pWMController->SetVisible(sWindowName, pending_context.IsVisible());

			PlutoRectangle rect = pending_context.Position();
			if(
				!pending_context.IsMaximized() && !pending_context.IsFullScreen() &&
				rect.Width != -1 && rect.Height != -1
			)
				bResult = bResult && m_pWMController->SetPosition(sWindowName, rect.X, rect.Y, rect.Width, rect.Height);

			if(pending_context.IsActivated())
			{
				if(sWindowName != m_sSdlWindowName)
					sExternalWindowToActivate = sWindowName;

				bResult = bResult && m_pWMController->ActivateWindow(sWindowName);
			}
		}
		if( bResult==false )
		{
			g_pPlutoLogger->Write(LV_STATUS,"WinListManager::ApplyContext unable to set %s",sWindowName.c_str());
			pending_context.ErrorFlag(true);
		}
		else
			pending_context.ErrorFlag(false);
	}

	if(!sExternalWindowToActivate.empty())
		m_pWMController->ActivateWindow(sExternalWindowToActivate);

	/* For some reason it's happened that even though the last 'R' command to the window manager WAS
	to activate Orbiter, Orbiter STILL is invisible behind myth until re-actated.  It appears that 
	constantly re-activating Orbiter doesn't cause a flicker or other undesired behavior */
	if( m_bKeepSdlWindowActive )
	{
		g_pPlutoLogger->Write(LV_STATUS, "KeepSdlWindowActive executed!");
		m_pWMController->ActivateWindow(m_sSdlWindowName);
		m_bKeepSdlWindowActive = false;
	}

	m_bExternalChange=false;
	m_CurrentContext.clear();
	m_CurrentContext = m_PendingContext;
}

WindowContext& WinListManager::PendingContext(string sWindowName)
{
	WindowsContext::iterator it = m_PendingContext.find(sWindowName);
	if(it == m_PendingContext.end())
	{
		g_pPlutoLogger->Write(LV_WARNING, "WinListManager::PendingContext: adding new entry %s", sWindowName.c_str());
		m_PendingContext.insert(make_pair(sWindowName, WindowContext(sWindowName)));
		return m_PendingContext.find(sWindowName)->second;
	}

	return it->second;
}

