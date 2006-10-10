#ifndef MediaStream_h
#define MediaStream_h

#include <map>
#include <deque>

#include "MediaAttributes.h"
#include "MediaHandlerInfo.h"
#include "DeviceData_Router.h"

#include "MediaFile.h"
#include "MediaTitle.h"
#include "MediaSection.h"

using namespace std;

class Row_MediaType_DesignObj;
class Row_DeviceTemplate_MediaType_DesignObj;

namespace DCE
{

	class RemoteControlSet 
	{
	public:
		int 	m_iPK_Screen_Remote; 		/** What screen to use as the remote control */
        int 	m_iPK_DesignObj_Remote_Popup; 		/** In UI2, this is the context sensitive media options bar on the main menu */
        int 	m_iPK_Screen_FileList; 			/** What screen to use as the file list */
        int 	m_iPK_Screen_OSD; 				/** What screen to use as the remote if this is an on screen display */
		int		m_iPK_Screen_Alt_Remote;				/** The Alt versions are used for example for DVD Menu's, or non-live TV */
		int		m_iPK_Screen_Alt_OSD;
		int		m_iPK_Screen_OSD_Speed;		/** The screen used for letting the user change the speed on screen */
		int		m_iPK_Screen_OSD_Track;		/** The screen used for letting the user change the track on screen */
 
		RemoteControlSet(Row_MediaType_DesignObj *);
		RemoteControlSet(Row_DeviceTemplate_MediaType_DesignObj *);
	};


    /** @brief
     * An instance of media, such as a DVD or a CD.  If 2 people start watching the same DVD individually, that will be 2 media streams,
     * because they are independent.
     * However, if 1 person starts watching and another joins in, or the media is broadcast, it becomes a single stream with multiple destinations.  In this case, all
     * the destinations are seeing the same thing, and the appropriate plug-in must concern itself with proper audio/video sync,
     * and ensure that if 1 person changes the stream, they all stay in sync.
     * It is possible to do this even with non-stored media, like Broadcast TV.
     * Whatever media plugin first created the media is considered the 'responsible' plug-in.  A MediaStream can contain a 'playlist' or group
     * of files or media clips.  However, all must be of the same type since there is only 1 MediaHandlerInfo for a stream.  You cannot, therefore, mix
     * audio and video clips in a playlist, or some stored audio (WAV, MP3, etc) with CD's or DVD's.
    */
    class MediaStream
    {
        int m_iStreamID; /** A unique number to identify this stream -- cannot change this */

    public:
        class MediaHandlerInfo *m_pMediaHandlerInfo; /** Which handler has primary responsibility for this media stream */

		class OH_Orbiter *m_pOH_Orbiter_StartedMedia;    	   /** Which orbiter started this stream in the first place */

        map<int, class EntertainArea *> m_mapEntertainArea; /** The entertainment areas where this stream is playing */
		map<int,int> m_mapPK_Attribute;  /** An external media identification script may set attributes here, PK_AttributeType,Section=PK_Attribute */
		map< pair<int,int>,string > m_mapSections; /** pair<Chapter,Title/0> to section description.  These are provided by the media player independent of what we have in the database */
        int m_mapPK_Attribute_Find(int PK_AttributeType) { map<int,int>::iterator it = m_mapPK_Attribute.find(PK_AttributeType); return it==m_mapPK_Attribute.end() ? 0 : (*it).second; }

        /**
         * As more 'play media' commands come in to this stream, it will add them to the queue so the user can save as a play list.
         * If it's a mounted media, like dvd, that won't happen
         */
        deque<MediaFile *>  m_dequeMediaFile;        /** The filenames we're playing */
        deque<MediaTitle *>  m_dequeMediaTitle;      /** The titles, if this is something like a DVD */
        deque<MediaSection *>  m_dequeMediaSection;      /** The titles, if this is something like a DVD */
        unsigned int m_iDequeMediaFile_Pos;   /** The play position in the m_dequeFilename deque. */
        int m_iDequeMediaTitle_Pos;  /** The play position in the m_dequeMediaTitle deque. */
        int m_iDequeMediaSection_Pos;/** The play position in the m_dequeMediaSection deque. */
        int m_iPK_Playlist;          /** the ID of the playlist. nonZero if the playlist was loaded from database, zero otherwise. */
        string m_sPlaylistName;      /** the name of the playlist which was loaded from the database. */
		int m_discid;  /** A unique number to identify the disc inserted, if this is from a removable disc (CD/DVD) */
		bool m_bIdentifiedDisc;
		
		/** When this media is identified, the priority of the identifying module is stored here so if another
		module also identifies it, we can take the one with the highest priority */
		int m_IdentifiedPriority; 
		int m_dwPK_Disc;  /** 0 if this isn't a removable disc media */

		MediaDevice		*m_pMediaDevice_Source;      /** The device which is the source of this media stream. */

		// If this is a generic media device, like a DVD player, that is connected to a capture card which is being
		// accessed by a media playing overlay device, like Xine, the following will be non null
		MediaDevice		*m_pMediaDevice_CaptureCard;      /** The device which is the capture card for this media stream. */
		MediaDevice		*m_pMediaDevice_Overlay;      /** The device which is the overlay media device of this media stream. */

		int 			 m_iPK_MediaType;        	/** The type of media in this stream. */
		int				 m_iPK_MediaProvider;		/** The media provider in this stream. */

		char 		*m_pPictureData;
        size_t 		 m_iPictureSize;

		map<int,RemoteControlSet *>	m_mapRemoteControlSet;  /** The remote controls to use for this stream, the int=the orbiter 
															Since each orbiter may have a different set of remote controls. 
															We will add remotes here as they come up*/

		bool 	m_bPlaying;        				/** True if the media is now playing */

		int 	m_iPK_Users;        			/** Who started the media */
        bool 	m_bFollowMe;       				/** True if the media is supposed to follow the above user */
		bool	m_bPlugInWillSetDescription;	/** True if the plugin will handle determining the description */

		bool 	m_bIsMovable; 					/** < bCanMove - This if this media stream can be moved in a move media command */
		int 	m_iOrder;  						/** This is used for the floorplans to order and color code the streams */
		int		m_iRepeat;						/** Indicates if this media should repeat */
		bool 	m_bResume; 						/** < True if this media was started with the 'resume' flag, meaning when it ends, resume what was playing previously */
		bool 	m_bUseAltScreens;				/** < These are used */

		bool	m_bStopped;						/** Set when we stop the stream, so that if we later get a playback completed we know we already stopped it and don't need to do anything more */

		time_t	m_tTime;						/** When the user started this media */
		time_t	m_tTime_Parked;					/** When we parked this media -- ie the user left an area with follow me on, so we stop the media, and leave it 'pending' for a while so he can enter another area */

        enum SourceType m_eSourceType;  /** Where the media is coming from */

        string m_sMediaDescription;     /** Some text the plug-in populates to describe the media.  "The Patriot", "Beatles Anthology" are examples.
												For TV or radio this is the channel */
        string m_sSectionDescription;   /** Describe where we are in the media, such as a song title, chapter, track, etc.
                        						For TV or radio this is the name of the show */
        string m_sMediaSynopsis;    	/** A description of what's playing, such as the tv show description */
        string m_sTimecode;    			/** The handler may populate this with a timecode */
        string m_sTotalTime;    		/** The handler may populate this with the total time */
		string m_sLastPosition;			/** The handler may populate this with a seekable position showing the current location.  I should do this in StopMedia if possible so the media can be resumed */
		string m_sStartPosition;		/** The position to start the stream at. */
		string m_sPlaybackSpeed;		/** Like 2x, 1/4x, etc.  Normally leave blank for normal */

		string m_sAppName;				/** The name of the application (window name actually) the will render this on the media director */

		bool m_bContainsTitlesOrSections;  /** The stream has titles or sections, like a dvd, and skipping should skip withing the file, not between files */

        /** @brief constructor*/
        MediaStream(class MediaHandlerInfo *pMediaHandlerInfo, int iPK_MediaProvider, MediaDevice *pMediaDevice, int PK_Users,enum SourceType sourceType,int iStreamID);

        /** @brief virtual destructor */
        virtual ~MediaStream();

        virtual int GetType() { return MEDIASTREAM_TYPE_GENERIC; }
		/**
		 * @brief Goes through the attributes (song title, group, etc.) and determines the
		 * appropriate description for the stream and each file in it
		 */
        virtual void UpdateDescriptions(bool bAllFiles=false,MediaFile *pMediaFile_In=NULL);
        int m_iStreamID_get() { return m_iStreamID; }

        void SetPicture(char *pPictureData,int iPictureSize) { delete[] m_pPictureData; m_pPictureData=pPictureData; m_iPictureSize=iPictureSize; }
		string GetAllOSD();

		virtual bool isMovable();
		virtual void setIsMovable(bool bIsMovable = true);

        virtual string GetFilenameToPlay( string defaultFileName = "" );
        virtual void SetPlaylistPosition(int position);
        virtual void ChangePositionInPlaylist(int iHowMuch);
        virtual void DumpPlaylist();
        virtual void ClearPlaylist();

		virtual bool CanPlayMore();
		virtual bool ContainsVideo();

		// True if the orbiter is an on screen display for this stream.  If ppEntertainArea is not NULL
		// it will be set to the entertainment area for the OSD
		virtual bool OrbiterIsOSD(int PK_Orbiter,EntertainArea **ppEntertainArea=NULL);
		virtual int GetRemoteControlScreen(int PK_Orbiter);
		virtual void GetRenderDevices(map<int, MediaDevice *> *pmapMediaDevices);

		virtual MediaFile *GetCurrentMediaFile()
		{
//			if( m_iDequeMediaFile_Pos < 0 || m_iDequeMediaFile_Pos >= m_dequeMediaFile.size() )
			// (rob.r) m_iDequeMediaFile_Pos is an unsigned int, can't be < 0 
			if( m_iDequeMediaFile_Pos >= m_dequeMediaFile.size() )
			{
				g_pPlutoLogger->Write(LV_CRITICAL,"MediaFile::GetCurrentMediaFile - no file");
				return NULL;
			}

			return m_dequeMediaFile[m_iDequeMediaFile_Pos];
		}

		virtual void DeleteEntryFromPlaylist(int position);
		virtual void MoveEntryInPlaylist(int position, int displacement);
		/**
		 * @brief The stream will change his internal state to match the new location specificated.
		 * @return true if the specification was undestood by the MediaStream, false otherwise.
		 */
		virtual bool ProcessJumpPosition(string sJumpSpecification);
	
		/**
		* return a comma delimited list of all active destination ent areas
		* except the one passed in, or all areas if the parameter is null
		*/
		string GetEntAreasWithout(EntertainArea *pEntertainArea);
		string GetEntAreasWithout(map<int,class EntertainArea *> *p_mapEntertainArea);
	};

    typedef map<int,MediaStream *> MapMediaStream;
}

#endif
