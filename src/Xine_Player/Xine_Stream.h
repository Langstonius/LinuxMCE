/**
*
* @file Xine_Stream.h
* @brief header for Xine_Stream class
*
*/

#ifndef XINE_STREAM_H
#define XINE_STREAM_H

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

#include "Xine_Stream_Factory.h"


namespace DCE
{
	class Xine_Stream_Factory;
	
	class Xine_Stream
	{
		public:
			typedef enum _playbackSpeedEnum
			{
				PLAYBACK_REW_64 = -64000,
				PLAYBACK_REW_32 = -32000,
				PLAYBACK_REW_16 = -16000,
				PLAYBACK_REW_8 = -8000,
				PLAYBACK_REW_4 = -4000,
				PLAYBACK_REW_2 = -2000,
				PLAYBACK_REW_1 = -1000,
				PLAYBACK_REW_1_2 = -500,
				PLAYBACK_REW_1_4 = -250,

				PLAYBACK_STOP = 0,

				PLAYBACK_FF_1_4 = 250,
				PLAYBACK_FF_1_2 = 500,
				PLAYBACK_FF_1 = 1000,
				PLAYBACK_NORMAL = 1000,
				PLAYBACK_FF_2 = 2000,
				PLAYBACK_FF_4 = 4000,
				PLAYBACK_FF_8 = 8000,
				PLAYBACK_FF_16 = 16000,
				PLAYBACK_FF_32 = 32000,
				PLAYBACK_FF_64 = 64000,
			} PlayBackSpeedType;
		
			// media info
			string m_sCurrentFile;
			int m_iChapter;
			int m_iTitle;
			
			void DisplaySpeedAndTimeCode();
			void Seek(int pos,int tolerance_ms);
			
			//returns complex position construction
			string GetPosition();
			
			//reports timecode
			void ReportTimecode();
			
			static int CalculatePosition(string &sMediaPosition,string *sMRL,int *Subtitle,int *Angle,int *AudioTrack);
			
		private:
			// stream access controlling mutex
			pluto_pthread_mutex_t m_streamMutex;
			
			bool m_bInitialized;
			
			Xine_Stream_Factory *m_pFactory;

			// xine resources for the stream:
			//		A/V output ports
			xine_audio_port_t *m_pXineAudioOutput;
			xine_video_port_t *m_pXineVideoOutput;
			//		x11 visual for video output
			x11_visual_t m_x11Visual;
			//  audio and video drivers
			string m_sXineAudioDriverName;
			string m_sXineVideoDriverName;
			
			// actually used drivers
			string m_sUsedVideoDriverName;
			
			// xine library link
			xine_t *m_pXineLibrary;
			// stream data objects
			xine_stream_t *m_pXineStream;
			xine_event_queue_t *m_pXineStreamEventQueue;
			// post-processing plugins
			xine_post_t *m_pXineVisualizationPlugin, *m_pXineDeinterlacePlugin;
	
			// stream properties
			bool m_bHasVideo;
			bool m_bHasAudio;
			
			int m_iImgWidth;
			int m_iImgHeight;
			int m_iImgXPos;
			int m_iImgYPos;
			string m_sMediaInfo;
			
			int m_iPlaybackSpeed;
			
			void FireMenuOnScreen(int iButtons);
			
			int m_iSeekMuteStatus;
			
			string readMediaInfo();
			void ReadAVInfo();
			void SendAVInfo();
		public:
			int m_iTimeCodeReportFrequency;
			bool m_bIsVDR;   // Is this live tv from VDR?
			string m_sAudioInfo;
			string m_sVideoInfo;
			
		private:
			int m_iSpecialSeekSpeed;
			int m_iSpecialOneTimeSeek;
			int m_iTrickPlaySpeed;

			// X subsystem properties
			string m_sWindowTitle;
			Display *m_pXDisplay;
			int m_iCurrentScreen;
			int m_iCurrentWindow;
			Window windows[ 2 ];
			Cursor cursors[ 2 ];
			Pixmap noCursor;
						
			// pixel aspect ratio
			double m_dPixelAspect;

			// creates X11 windows for output
			bool CreateWindows();
			bool DestroyWindows();
			
			// inits xine A/V output ports
			bool InitXineAVOutput();
			
			// events processing thread
			pthread_t threadEventLoop;
			
			int XServerEventProcessor(XEvent &event );
			
			// stream thread state
			bool m_bExitThread;
			
			public:
			bool m_bIsRendering;
			
			private:
			// Do the seeking in these functions
			void StartSpecialSeek( int Speed );
			void StopSpecialSeek();
			
			void HandleSpecialSeekSpeed();  // Used for handling rewind and fast forward

			// Used to handle rewind and fastforward to keep track of where we last seeked
			timespec m_tsLastSpecialSeek;
			int m_posLastSpecialSeek;

			// Display functions
			void DisplayOSDText( string sText );

			// OSD xine object
			xine_osd_t *m_xine_osd_t;
			
			int m_iPrebuffer;
			
			
			public:
			int m_iStreamID;

			private:
			int m_iRequestingObject;
			KeySym translatePlutoKeySymToXKeySym( int plutoButton );

			/**
			 * @brief Will convert a point to coordinates relative to the playback window.
			 */
			int translate_point( int gui_x, int gui_y, int *video_x, int *video_y );
			
			bool setDebuggingLevel( bool newValue );

			bool m_isSlimClient;
			bool m_bTrickModeActive;
		public:
			Xine_Stream(Xine_Stream_Factory* pFactory, xine_t *pXineLibrary, int ID=0, int iTimeCodeReportFrequency=0, int iRequestingObject=0);
			~Xine_Stream();
			
			/**
		 	* @brief Find current position for stream.
			 */
			int getStreamPlaybackPosition( int &positionTime, int &totalTime );

			
			// utility functions
			bool StartupStream();
			bool ShutdownStream();
			
			bool EnableDeinterlacing();
			void DisableDeinterlacing();
			bool EnableVisualizing();
			void DisableVisualizing();

			// thread functions
			//		xine events processor
			static void XineStreamEventListener( void *streamObject, const xine_event_t *event );
			//  X events processor
			static void *EventProcessingLoop( void *arguments );
			
			
			// media functions
			bool OpenMedia(string fileName, string &sMediaInfo, string sMediaPosition);
			bool CloseMedia();

			
			bool playStream( string mediaPosition);
			
    /**
			 * @brief change the stream playback speed.
		 */

			void changePlaybackSpeed( PlayBackSpeedType newSpeed );

    /**
			 * @brief get current playback speed
		 */
			PlayBackSpeedType getPlaybackSpeed();
			

    /**
			 * @brief restart the stream playback.
		 */

			void restartMediaStream();

    /**
			 * @brief pause stream playback.
		 */

			void pauseMediaStream();

			void resume();

			
    /**
 		 * @brief Enable broadcasting on this stream
		 */
			int enableBroadcast( int iStreamID );

    /**
			 * @brief send a Xine event to the engine
		 */
			void sendInputEvent( int eventID );
			
    /**
			 * @brief Take a screenshot of the currenly playing stream.
		 */
			void make_snapshot( string sFormat, int iWidth, int iHeight, bool bKeepAspect, char*&pData, int &iDataSize );
			
			//void setPlaybackSpeed( int speed );
			
			bool setSubtitle( int Value );
			bool setAudio( int Value );
			int getSubtitle();
			int getAudio();
			
			const char *TranslateLanguage( const char *abbreviation );

			void playbackCompleted( bool bWithErrors );
    
			/**
			 * @brief move the button down in a DVD menu.
		 */
			void selectNextButton();

    /**
			 * @brief move the button up in a DVD menu.
		 */
			void selectPrevButton();

			void pushCurrentButton();
			
    /**
			 * @brief in a DVD playback this will go to the respective menu (RootMenu, etc.)
		 */

			void selectMenu( int iMenuType );

    /**
			 * @brief simlate a mouse click on the current active window.
		 */
			void simulateMouseClick( int X, int Y );

    /**
			 * @brief simlate a mouse click on the current active window.
		 */
			void simulateKeystroke( int plutoButton );

			    /**
			 * @brief callbacks required by the xine engine for video playback.
					 */

			static void destinationSizeCallback( void *data,
																					 int video_width, int video_height,
																					 double video_pixel_aspect,
																					 int *dest_width, int *dest_height,
																					 double *dest_pixel_aspect );

    /**
			 * @see destinationSizeCallback
		 */

			static void frameOutputCallback( void *data,
																			 int video_width, int video_height,
																			 double video_pixel_aspect,
																			 int *dest_x, int *dest_y,
																			 int *dest_width, int *dest_height,
																			 double *dest_pixel_aspect,
																			 int *win_x, int *win_y );


		public:
			class Dynamic_Pointer
			{
				public:
					Dynamic_Pointer(
							Xine_Stream *pOwner,
					Cursor *pCursor_normal,
					Cursor *pCursor_hidden
												 );
					~Dynamic_Pointer();
					void pointer_hide();
					void pointer_show();
					void pointer_check_time();
				private:
					Xine_Stream *m_pOwner;
					Cursor *m_pCursor_normal;
					Cursor *m_pCursor_hidden;
					time_t m_start_time;
			};
			Dynamic_Pointer *m_pDynamic_Pointer;
	
			void getScreenShot( int iWidth, int iHeight, char *&pData, int &iDataSize, string &sFormat, string &sCMD_Result );
			string getRenderingWindowName();
			int getDeviceId();
			bool isSlimClient();
			void setSlimClient( bool isSlimClient );
			
	};
	
	
	
}

#endif
