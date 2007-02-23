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
#ifndef __ORBITER_RENDERER_SDL_WIN32_H__
#define __ORBITER_RENDERER_SDL_WIN32_H__

#ifdef ORBITER_OPENGL
#include "../../OpenGL/OrbiterRenderer_OpenGL.h"
#else
#include "../OrbiterRenderer_SDL.h"
#endif

namespace DCE
{
	class Orbiter;

	class OrbiterRenderer_SDL_Win32 : public 
#ifdef ORBITER_OPENGL
		OrbiterRenderer_OpenGL
#else
		OrbiterRenderer_SDL
#endif
	{
	private:
		friend class OrbiterRendererFactory;
		OrbiterRenderer_SDL_Win32(Orbiter *pOrbiter);

	public:

		~OrbiterRenderer_SDL_Win32();

		void HandleKeyEvents(UINT uMsg, WPARAM wParam, LPARAM lParam);

		bool DisplayProgress(string sMessage, int nProgress);
		int PromptUser(string sPrompt,int iTimeoutSeconds,map<int,string> *p_mapPrompts);

		WNDPROC m_pfOldSDLWindowProc;
		HWND m_hSDLWindow;

		virtual void SetupWindow();
		virtual void Configure();
	};
}
#endif //__ORBITER_RENDERER_SDL_WIN32_H__