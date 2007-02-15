/**
*
* @file Xine_Stream_Factory.h
* @brief header for Xine_Stream_Factory class
*
*/

#ifndef XINE_STREAM_FACTORY_H
#define XINE_STREAM_FACTORY_H

#include <map>

#include <xine.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/XTest.h>
#include <X11/extensions/xf86vmode.h>
#include <X11/cursorfont.h>

#include "Xine_Stream.h"

namespace DCE
{
	class Xine_Stream;	
	class Xine_Player;
	
	class Xine_Stream_Factory
	{
		friend class Xine_Stream;
		private:
			bool m_bInitialized;
			xine_t *m_pXineLibrary;
			string m_sConfigFile;
			
			string m_sXineAudioDriverName;
			string m_sXineVideoDriverName;
			
			bool m_bUseDeinterlacing;
			string m_sDeinterlacingConfig;
			
			// detects current output video and audio drivers
			void DetectOutputDrivers();
			
			map<int, Xine_Stream*> streamsMap;
			//map<int, Xine_Stream*> broadcastersMap;
			
			// factory access controlling mutex
			pluto_pthread_mutex_t m_factoryMutex;
			
			// X subsystem properties
			string m_sWindowTitle;
			Display *m_pXDisplay;
			int m_iCurrentScreen;
			int m_iCurrentWindow;
			
			// window init routines
			bool CreateWindows();
			bool DestroyWindows();
			
			// X11 windows
			Window windows[ 2 ];
			
			int m_iImgWidth;
			int m_iImgHeight;
			int m_iImgXPos;
			int m_iImgYPos;
			
			// owning player
			Xine_Player *m_pPlayer;
		public:
			Xine_Stream_Factory(Xine_Player *pOwner);
			~Xine_Stream_Factory();
			
			bool StartupFactory();
			bool ShutdownFactory();

			string GetAudioDriver() { return m_sXineAudioDriverName; };
			string GetVideoDriver() { return m_sXineVideoDriverName; };
			
			void setAudioSettings();
			void setVideoDriver(string strVideoDriver);
			
			Xine_Stream *GetStream(int streamID, bool createIfNotExist=false, int requestingObject=-1, bool bBroadcasting=false);
			
			void ReportAudioTracks(string sTracks);
			void ReportSubtitles(string sSubtitles);
			void ReportTimecodeViaIP(int iStreamID, int Speed);
			void ReportAVInfo( string sFilename, int iStreamID, string sMediaInfo, string sAudioInfo, string sVideoInfo );
			
			void CloseStreamAV(int iStreamID);
			
			void DestroyStream(int iStreamID);
			
			void HideStreamWindows(int iStreamID);
			
			int m_iLastRenderingStream;
	};

}

#endif
