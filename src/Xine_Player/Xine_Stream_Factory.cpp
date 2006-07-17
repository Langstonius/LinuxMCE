#include "DCE/Logger.h"
#include "DCE/DCEConfig.h"


#include "Xine_Stream_Factory.h"
#include "Xine_Stream.h"
#include "Xine_Player.h"

#include <xine/xineutils.h>


namespace DCE
{ // DCE namespace begin

Xine_Stream_Factory::Xine_Stream_Factory(Xine_Player *pOwner):
		m_factoryMutex("xine-stream-factory-access-mutex"),
		m_pPlayer(pOwner)
{
	pthread_mutexattr_t mutexAttr;
	pthread_mutexattr_init( &mutexAttr );
	pthread_mutexattr_settype( &mutexAttr,  PTHREAD_MUTEX_RECURSIVE_NP );
	m_factoryMutex.Init( &mutexAttr );
		
	m_bInitialized = false;
	m_pXineLibrary = NULL;
	
	// default drivers names
	m_sXineAudioDriverName = "alsa";
	m_sXineVideoDriverName = "xv";

	DCEConfig * pConfig = new DCEConfig();
	m_sConfigFile = pConfig->ReadString( "XinePlayerConfigurationFile", "/etc/pluto/xine.conf" );
	delete pConfig;
}

Xine_Stream_Factory::~Xine_Stream_Factory()
{
	if (m_bInitialized)
		ShutdownFactory();
}

bool Xine_Stream_Factory::StartupFactory()
{
	PLUTO_SAFETY_LOCK( factoryLock, m_factoryMutex );
	
	// avoid double-initialization
	if (m_bInitialized) 
	{
		g_pPlutoLogger->Write( LV_WARNING, "Double initialization attempted - bad code?");
		return false;
	}
	
	// initing X threads
	if ( !XInitThreads() )
	{
		g_pPlutoLogger->Write( LV_WARNING, "XInitThreads() failed!" );
		return false;
	}

	// connect to libxine
	m_pXineLibrary = xine_new();
	 if (!m_pXineLibrary)
	 {
		g_pPlutoLogger->Write( LV_WARNING, "Cannot connect to Xine Library");
		return false;
	 }

	// loading config
	g_pPlutoLogger->Write( LV_STATUS, "Loading config from %s", m_sConfigFile.c_str() );		
	xine_config_load( m_pXineLibrary, m_sConfigFile.c_str() );
	xine_init( m_pXineLibrary );
	
	// detecting output drivers
	DetectOutputDrivers();
	
	// setting speakers arrangement
	// setOutputSpeakerArrangement(m_pPlayer->DATA_Get_Output_Speaker_arrangement());
	
	m_bInitialized = true;
	return true;
}

bool Xine_Stream_Factory::ShutdownFactory()
{
	// avoid double-deinitialization
	if (!m_bInitialized) 
	{
		g_pPlutoLogger->Write( LV_WARNING, "Double deinitialization attempted - wrong code?");
		return false;
	}
	
	// destroying all streams
	g_pPlutoLogger->Write( LV_WARNING, "Destroying all active streams");
	map<int, Xine_Stream*>::iterator stream;
	while(true)
	{
		int streamID;
		{
			PLUTO_SAFETY_LOCK( factoryLock, m_factoryMutex );
			stream = streamsMap.begin();
			if (stream == streamsMap.end())
				break;
			else
				streamID = (*stream).first;
		}
		DestroyStream( streamID);
	}
	
	
	PLUTO_SAFETY_LOCK( factoryLock, m_factoryMutex );
	
	// disconnect from libxine
	xine_exit(m_pXineLibrary);
	m_pXineLibrary = NULL;
	m_bInitialized = false;
	
	return true;
}

// detecting currently used drivers
void Xine_Stream_Factory::DetectOutputDrivers()
{
	//registering video key param

	xine_cfg_entry_t xineConfigEntry;
	char **driver_ids;
	int i;
	const char *const *driver_names;
	
	driver_names = xine_list_video_output_plugins( m_pXineLibrary );
	
	i = 0;
	while ( driver_names[ i++ ] )
		;
	driver_ids = ( char ** ) xine_xmalloc( sizeof( char * ) * ( i + 1 ) );
	i = 0;
	driver_ids[ i ] = strdup( "auto" );
	
	while ( driver_names[ i ] )
	{
		driver_ids[ i + 1 ] = strdup( driver_names[ i ] );
		i++;
	}
	
	driver_ids[ i + 1 ] = NULL;
	xine_config_register_enum( m_pXineLibrary, "video.driver", 0, driver_ids, "video driver to use", "Choose video driver. NOTE: you may restart xine to use the new driver", 0, NULL, NULL );	
	
	// setting video driver
	if ( xine_config_lookup_entry ( m_pXineLibrary, "video.driver", &xineConfigEntry ) )
	{
		driver_names = xine_list_video_output_plugins( m_pXineLibrary );
		m_sXineVideoDriverName = driver_ids[ xineConfigEntry.num_value ];
		g_pPlutoLogger->Write( LV_STATUS, "Using video driver: %s", m_sXineVideoDriverName.c_str() );
	}
	else
		g_pPlutoLogger->Write( LV_STATUS, "Video driver key was not defined in the config file, using hardcoded default: %s", m_sXineVideoDriverName.c_str() );
	
	free( driver_ids );


	// registering audio keyparam

	driver_names = xine_list_audio_output_plugins( m_pXineLibrary );
	i = 0;
	while ( driver_names[ i++ ] )
		;
	driver_ids = ( char ** ) xine_xmalloc( sizeof( char * ) * ( i + 1 ) );
	i = 0;
	driver_ids[ i ] = strdup( "auto" );
	while ( driver_names[ i ] )
	{
		driver_ids[ i + 1 ] = strdup( driver_names[ i ] );
		i++;
	}
	
	driver_ids[ i + 1 ] = NULL;
	xine_config_register_enum( m_pXineLibrary, "audio.driver", 0, driver_ids, "audio driver to use", "Choose audio driver. NOTE: you may restart xine to use the new driver", 0, NULL, NULL );

	// audio driver
	if ( xine_config_lookup_entry ( m_pXineLibrary, "audio.driver", &xineConfigEntry ) )
	{
		driver_names = xine_list_audio_output_plugins( m_pXineLibrary );
		m_sXineAudioDriverName = driver_ids[ xineConfigEntry.num_value ];
		g_pPlutoLogger->Write( LV_STATUS, "Using audio driver: %s", m_sXineAudioDriverName.c_str() );
	}
	else
		g_pPlutoLogger->Write( LV_STATUS, "Audio driver key was not defined in the config file, using hardcoded default: %s", m_sXineAudioDriverName.c_str() );
	
	free( driver_ids );
}

#if 0
static const char *audio_out_types_strs[] =
{
	"Mono 1.0",
	"Stereo 2.0",
	"Headphones 2.0",
	"Stereo 2.1",
	"Surround 3.0",
	"Surround 4.0",
	"Surround 4.1",
	"Surround 5.0",
	"Surround 5.1",
	"Surround 6.0",
	"Surround 6.1",
	"Surround 7.1",
	"Pass Through",
	NULL
};

void Xine_Stream_Factory::setOutputSpeakerArrangement( string strOutputSpeakerArrangement )
{
	xine_cfg_entry_t xineConfigEntry;

	g_pPlutoLogger->Write( LV_STATUS, "Setting the audio output speaker arrangement: %s", strOutputSpeakerArrangement.c_str() );

	xine_config_register_enum ( m_pXineLibrary, "audio.output.speaker_arrangement", 1, ( char ** ) audio_out_types_strs,
															"Speaker arrangement",
															NULL, 0, NULL, NULL );

	if ( xine_config_lookup_entry( m_pXineLibrary, "audio.output.speaker_arrangement", &xineConfigEntry ) == 0 )
	{
		g_pPlutoLogger->Write( LV_STATUS, "Could not lookup the current Speaker Arrangement Value" );
		return ;
	}

	g_pPlutoLogger->Write( LV_STATUS, "Current value: %s", audio_out_types_strs[ xineConfigEntry.num_value ] );

	g_pPlutoLogger->Write( LV_STATUS, "Trying to set the value to: %s", strOutputSpeakerArrangement.c_str() );
	int i = 0;
	while ( audio_out_types_strs[ i ] != NULL )
	{
		if ( strncmp( strOutputSpeakerArrangement.c_str( ), audio_out_types_strs[ i ], strlen( audio_out_types_strs[ i ] ) ) == 0 )
		{
			xineConfigEntry.num_value = i;
			xine_config_update_entry( m_pXineLibrary, &xineConfigEntry );

			if ( xine_config_lookup_entry( m_pXineLibrary, "audio.output.speaker_arrangement", &xineConfigEntry ) == 0 )
			{
				g_pPlutoLogger->Write( LV_STATUS, "Could not lookup the current Speaker Arrangement Value after update." );
				return ;
			}

			g_pPlutoLogger->Write( LV_STATUS, "Value changed to: %s", audio_out_types_strs[ xineConfigEntry.num_value ] );
			return ;
		}

		i++;
	}

	return ;
}
#endif

// returns pointer to stream if exists/was created or NULL otherwise
Xine_Stream *Xine_Stream_Factory::GetStream(int streamID, bool createIfNotExist, int requestingObject)
{
	{
		PLUTO_SAFETY_LOCK( factoryLock, m_factoryMutex );
		
		map<int, Xine_Stream*>::iterator stream = streamsMap.find(streamID);
		if (stream != streamsMap.end())
		{
			Xine_Stream *pStream = (*stream).second;
			pStream->StartupStream();
			return pStream;
		}
	}
		
	if (createIfNotExist)
	{
		Xine_Stream *new_stream = new Xine_Stream(this, m_pXineLibrary, streamID, m_pPlayer->DATA_Get_Time_Code_Report_Frequency(), requestingObject);
		if ((new_stream!=NULL) && new_stream->StartupStream())
		{
			g_pPlutoLogger->Write(LV_WARNING,"Created new stream with internalID=%i", streamID);
			
			PLUTO_SAFETY_LOCK( factoryLock, m_factoryMutex );
			streamsMap[streamID] = new_stream;
			return new_stream;
		}
		else
		{
			g_pPlutoLogger->Write(LV_WARNING,"Cannot create new stream with internalID=%i", streamID);
			if (new_stream!=NULL)
				delete new_stream;
			return NULL;
		}
	}
	else
		return NULL;
}

void Xine_Stream_Factory::ReportAudioTracks(string sTracks)
{
	m_pPlayer->DATA_Set_Audio_Tracks(sTracks);
}

void Xine_Stream_Factory::ReportSubtitles( string sSubtitles)
{
	m_pPlayer->DATA_Set_Subtitles(sSubtitles);
}

void Xine_Stream_Factory::ReportAVInfo( string sFilename, int iStreamID, string sMediaInfo, string sAudioInfo, string sVideoInfo )
{
	g_pPlutoLogger->Write(LV_WARNING, "Xine_Player::EVENT_Playback_Started(streamID=%i) <= AV info", iStreamID);
	m_pPlayer->EVENT_Playback_Started(sFilename, iStreamID, sMediaInfo, sAudioInfo, sVideoInfo);
}


void Xine_Stream_Factory::ReportTimecode(int iStreamID, int Speed)
{
	m_pPlayer->ReportTimecode( iStreamID, Speed);
}

void Xine_Stream_Factory::DestroyStream(int iStreamID)
{
	Xine_Stream *pStream = NULL;
	map<int, Xine_Stream*>::iterator stream;
	
	{	
		PLUTO_SAFETY_LOCK( factoryLock, m_factoryMutex );
		stream = streamsMap.find(iStreamID);
		
		if (stream != streamsMap.end())
		{
			pStream = (*stream).second;
			streamsMap.erase(stream);
		}
	}
	
	if (pStream)
	{
		delete pStream;
	}
	
	g_pPlutoLogger->Write(LV_WARNING,"Destroyed stream with internalID=%i", iStreamID);
}

void Xine_Stream_Factory::CloseStreamAV(int iStreamID)
{
	Xine_Stream *pStream = NULL;
	map<int, Xine_Stream*>::iterator stream;
	
	{	
		PLUTO_SAFETY_LOCK( factoryLock, m_factoryMutex );
		stream = streamsMap.find(iStreamID);
		
		if (stream != streamsMap.end())
		{
			pStream = (*stream).second;
		}
	}
	
	if (pStream)
	{
		pStream->ShutdownStream();
	}
	
	g_pPlutoLogger->Write(LV_WARNING,"Closed stream AV with internalID=%i", iStreamID);
}

} // DCE namespace end
