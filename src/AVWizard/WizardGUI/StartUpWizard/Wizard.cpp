#include "Wizard.h"

#include "WM_Event.h"

#ifdef WIN32
#define LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "WizardCommandLineParser.h"
#include "AVWizardConfParser.h"

#include "WizardPagesFactory.h"

#include <iostream>

#include "GUIWizardUtils.h"

#include "WizardCommandLineParser.h"
#include "SafetyLock.h"
#include "GenerateWizardConfigDefaults.h"
#include "WizardPageVideoAdjustSize.h"
#ifdef ALSA_DETECT
#include <alsa/asoundlib.h>
#endif

#ifndef WIN32
void signal_handler(int signal)
{
	WizardCommandLineParser *CmdLineParser = WizardCommandLineParser::GetInstance();
	switch (signal)
	{
		case SIGPIPE:
			std::cout<<"Signal SIGPIPE treated"<<std::endl;
			break;
		case SIGUSR1:
			std::cout<<"Signal SIGUSR1 treated at step " << Wizard::GetInstance()->CurrentPage <<std::endl;
			Wizard::GetInstance()->SetExitWithCode(2);
			Wizard::GetInstance()->AVWizardOptions->GetDictionary()->Set("InterruptedStep", Wizard::GetInstance()->CurrentPage);
			
			Wizard::GetInstance()->AVWizardOptions->SaveToXMLFile(CmdLineParser->ConfigFileDefault);
			break;
		case SIGUSR2:
			std::cout<<"Signal SIGUSR2 treated at step " << Wizard::GetInstance()->CurrentPage <<std::endl;
			Wizard::GetInstance()->SetExitWithCode(2);
			Wizard::GetInstance()->AVWizardOptions->GetDictionary()->Set("InterruptedStep", -Wizard::GetInstance()->CurrentPage);
			
			Wizard::GetInstance()->AVWizardOptions->SaveToXMLFile(CmdLineParser->ConfigFileDefault);
			break;
	}
}
#endif

Wizard::Wizard()
	: 
	Quit(false),
	StatusChange(true),
	LeftBorder(ARROWS_BORDER),
	TopBorder(ARROWS_BORDER),
	WizardBorder(ARROWS_BORDER)
{
	this->ExitCode = 0;
	AVWizardConfParser ConfigurationParser;
	this->AVWizardOptions = ConfigurationParser.ParseFile();
	this->FrontEnd = NULL;
	this->Width = 640;
	this->Height = 480;

	this->FullScreen = false;
	MainPage = NULL;
	CurrentPage = 1;
	AVWizardOptions->GetDictionary()->SetName("AVSettings");
	AVWizardOptions->GetDictionary()->SetType("Config_file");

	pthread_mutexattr_t m_MutexAttr;
	pthread_mutexattr_init( &m_MutexAttr );	
	pthread_mutexattr_settype( &m_MutexAttr,  PTHREAD_MUTEX_RECURSIVE_NP );
	pthread_mutex_init(&SafeMutex, &m_MutexAttr);
	pthread_mutexattr_destroy(&m_MutexAttr);

#ifndef WIN32
	signal(SIGUSR1, signal_handler);
	signal(SIGUSR2, signal_handler);
	signal(SIGPIPE, signal_handler);
#endif

	AVWizardOptions->GetDictionary()->Set("NoAudioDevice", 0);
#ifdef ALSA_DETECT
	int err;
	short buf[128];
	snd_pcm_t *playback_handle;
	snd_pcm_hw_params_t *hw_params;

	if ((err = snd_pcm_open (&playback_handle, "hw:0,0", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		fprintf (stderr, "cannot open audio device %s (%s)\n",
				"hw:0,0",
				snd_strerror (err));
		AVWizardOptions->GetDictionary()->Set("NoAudioDevice", 1);
	}
	else
		snd_pcm_close(playback_handle);
#endif
}

Wizard::~Wizard()
{
	pthread_mutex_destroy(&SafeMutex);
#ifndef WIN32
	Server.Finish = true;
#endif
	delete FrontEnd;
	FrontEnd = NULL;
}

//#define DEBUG
void Wizard::MainLoop()
{
//Remove, previously done
//	Resize(false);
	WizardCommandLineParser *CmdLineParser = WizardCommandLineParser::GetInstance();

	AVWizardOptions->GetDictionary()->Set("CurrentStep", Utils::Int32ToString(CurrentPage));
	AVWizardOptions->SaveToXMLFile(CmdLineParser->ConfigFileDefault);

	
	if(AVWizardOptions->GetDictionary()->Exists("RemoteAVWizardServerPort"))
	{
		int Port = Utils::StringToInt32( AVWizardOptions->GetDictionary()->GetValue("RemoteAVWizardServerPort"));
#ifndef WIN32
		Server.StartServer(Port);
#endif

		if (AVWizardOptions->GetDictionary()->Exists("RemoteCmd"))
		{
			RemoteCmd = AVWizardOptions->GetDictionary()->
				GetValue("RemoteCmd");
#ifndef WIN32
			m_WizardRemote.Start(RemoteCmd);
#endif
		}
	}

	AVWizardOptions->GetDictionary()->Set("NoResolutions", 5);
	AVWizardOptions->GetDictionary()->Set("Res1", "640x480");
	AVWizardOptions->GetDictionary()->Set("Res2", "641x480");
	AVWizardOptions->GetDictionary()->Set("Res3", "642x480");
	AVWizardOptions->GetDictionary()->Set("Res4", "643x480");
	AVWizardOptions->GetDictionary()->Set("Res5", "644x480");

	if(AVWizardOptions->GetDictionary()->Exists("WizardBorder"))
	{
		WizardBorder = Utils::StringToInt32( AVWizardOptions->GetDictionary()->
			GetValue("WizardBorder"));
	}

	if(AVWizardOptions->GetDictionary()->Exists("LeftBorder"))
	{
		LeftBorder = Utils::StringToInt32( AVWizardOptions->GetDictionary()->
			GetValue("LeftBorder"));
	}

	if(AVWizardOptions->GetDictionary()->Exists("TopBorder"))
	{
		TopBorder = Utils::StringToInt32( AVWizardOptions->GetDictionary()->
			GetValue("TopBorder"));
	}

	PaintStatus();
	while(!Quit)
	{
		while (!StatusChange && !Quit && (FrontEnd->HasEventPending()))
		{
			WM_Event Event;
			FrontEnd->TranslateEvent(Event);

			if(Event.Type)
				PushEvent(Event);
		}

		EvaluateEvents();

		if(!Quit)
		{
			if(StatusChange)
			{
				SafetyLock Lock(&SafeMutex);
				PaintStatus();
			}
			else
			{
				wizSleep(10);
			}
		}
	}
	CmdLineParser->CleanUp();
}

void Wizard::DoDecreaseAction()
{
	StatusChange = true;
	MainPage->DoDecreaseSetting();
}

void Wizard::DoIncreaseAction()
{
	StatusChange = true;
	MainPage->DoIncreaseSetting();
}

void Wizard::DoChangeActionBefore()
{
	MainPage->DoPreviousFocusItem();
	StatusChange = true;
}

void Wizard::DoChangeActionAfter()
{
	MainPage->DoNextFocusItem();
	StatusChange = true;
}

void Wizard::DoApplyScreen(SettingsDictionary* Settings)
{
	WizardCommandLineParser *CmdLineParser = WizardCommandLineParser::GetInstance();

	if(MainPage == NULL)
		return;
	AVWizardOptions->LoadFromXMLFile(CmdLineParser->ConfigFileDefault);
	int Result = MainPage->DoApplySetting(Settings);

	delete MainPage;
	MainPage = NULL;

	if (Quit)
		return;

	if(Result != 0)
		CurrentPage -=2;

	CurrentPage ++ ;

	if(CurrentPage == 8)
	{
		if(AVWizardOptions->GetDictionary()->Exists("NoAudioDevice"))
		{
			std::string AudioDevice = AVWizardOptions->GetDictionary()->GetValue("NoAudioDevice");
			bool IsAudioDevice = Utils::StringToInt32( AudioDevice) != 0;
			if (IsAudioDevice)
				CurrentPage+=3;
		}
	}

	if (CurrentPage == 9)
		if(IsAnalogSound)
			CurrentPage += 2;
	if(CurrentPage == WIZARD_NO_PAGES+1)
	{
		AVWizardOptions->SaveToXMLFile(CmdLineParser->ConfigFileDefault);
		SetExitWithCode(ExitCode);
	}
	CreateDialogs();
	if(MainPage == NULL)
	{
#ifdef DEBUG
		std::cout<<"Warning, XML asociated page doesn't exist!"<<std::endl;
#endif
	}
	else
	{
		MainPage->GetPageLayout()->Paint();
		FrontEnd->Flip(LeftBorder, TopBorder, WizardBorder);
		
	}	
}

void Wizard::DoCancelScreen()
{
	if(MainPage == NULL)
		return;
	MainPage->DoCancelSetting();

	delete MainPage;
	MainPage = NULL;
	if(CurrentPage == WIZARD_NO_PAGES)
	{
		bool IsNoAudioDevice = false;
		if(AVWizardOptions->GetDictionary()->Exists("NoAudioDevice"))
		{
			std::string AudioDevice = AVWizardOptions->GetDictionary()->GetValue("NoAudioDevice");
			IsNoAudioDevice = Utils::StringToInt32( AudioDevice) != 0;
			if (IsNoAudioDevice)
				CurrentPage-=3;
		}
		if(!IsNoAudioDevice)
			if(IsAnalogSound)
				CurrentPage -= 2;
		

	}
	CurrentPage -- ;
	CreateDialogs();
	StatusChange = true;
}


void Wizard::PushEvent(WM_Event& Event)
{
	SafetyLock Lock(&SafeMutex);
	Events.push_back(Event);
}

void Wizard::EvaluateEvents()
{	
	WM_Event Event;

	while(Events.size())
	{
		{
			SafetyLock Lock(&SafeMutex);

			Event = Events.front();
			Events.pop_front();
		}

		if(Event.Type == WMET_QUIT)
		{
			Quit = true;
			return;
		}

#ifdef DEBUG
		std::cout<<"Key pressed: ";
#endif
		StatusChange = true;
		switch(Event.Type) {
	case WMET_QUIT:
#ifdef DEBUG
		std::cout<<"Quit"<<std::endl;
#endif
		Quit = true;
		break;
	case WMET_LEFT_KEY:
#ifdef DEBUG
		std::cout<<"Left"<<std::endl;
#endif
		DoDecreaseAction();
		break;
	case WMET_RIGHT_KEY:
#ifdef DEBUG
		std::cout<<"Right"<<std::endl;
#endif
		DoIncreaseAction();
		break;
	case WMET_UP_KEY:
#ifdef DEBUG
		std::cout<<"Up"<<std::endl;
#endif
		DoChangeActionBefore();
		break;
	case WMET_DOWN_KEY:
#ifdef DEBUG
		std::cout<<"Down"<<std::endl;
#endif
		DoChangeActionAfter();
		break;
	case WMET_ENTER_KEY:
#ifdef DEBUG
		std::cout<<"Enter"<<std::endl;
#endif
		DoApplyScreen(AVWizardOptions->GetDictionary());
		break;
	case WMET_PLUS_KEY:
#ifdef DEBUG
		std::cout<<"ZoomIn"<<std::endl;
#endif
		ZoomIn();
		break;
	case WMET_MINUS_KEY:
#ifdef DEBUG
		std::cout<<"ZoomOut"<<std::endl;
#endif
		ZoomOut();
		break;
	case WMET_SAVE:
#ifdef DEBUG
		std::cout<<"Save exit code"<<std::endl;
#endif
		AVWizardOptions->GetDictionary()->Set("ExitCode", this->ExitCode);
		AVWizardOptions->SaveToXMLFile(WizardCommandLineParser::GetInstance()->ConfigFileDefault);
		break;
	case WMET_ESCAPE_KEY:
#ifdef DEBUG
		std::cout<<"Escape"<<std::endl;
#endif
		DoCancelScreen();
		break;
	default:
		StatusChange = false;
		}

	}
}

void Wizard::PaintStatus()
{
	StatusChange = false;
	if(MainPage)
		MainPage->GetPageLayout()->Paint();
	FrontEnd->Flip(LeftBorder, TopBorder, WizardBorder);
}

int Wizard::ParseCommandLineParameters(int argc, char** argv)
{
	WizardCommandLineParser *CmdLineParser = WizardCommandLineParser::GetInstance();

	std::vector<std::string> Arguments = CmdLineParser->CommandLineSplit(argc, argv);
	CmdLineParser->ParseArguments(Arguments, *(AVWizardOptions->GetDictionary()));

	if (CmdLineParser->NeedQuit)
	{
		AVWizardOptions->SaveToXMLFile(CmdLineParser->ConfigFileDefault);
		exit(0);
	}

	if(CmdLineParser->StartStep != -1)
	{
		CurrentPage = CmdLineParser->StartStep;
	}

	CmdLineParser->CleanUp();
	CmdLineParser = NULL;


	return 0;
}

void Wizard::CreateDialogs()
{
	WizardPagesFactory* Factory = WizardPagesFactory::GetInstance();

	Factory->SetSDLFrontEnd(FrontEnd);

	if(MainPage!= NULL)
	{
		delete MainPage;
		MainPage = NULL;
	}

	MainPage = Factory->CreatePredefinedWizardPage(CurrentPage);

	if(MainPage)
	{
		WizardCommandLineParser *CmdLineParser = WizardCommandLineParser::GetInstance();
		if (StatusChange)
		{
			AVWizardOptions->GetDictionary()->Set("CurrentStep", Utils::Int32ToString(CurrentPage));
			AVWizardOptions->SaveToXMLFile(CmdLineParser->ConfigFileDefault);
		}

		MainPage->SetWizard(this);
	}
	Factory->CleanUp();
}

/*static*/ Wizard* Wizard::Instance = NULL;

/*static*/ Wizard* Wizard::GetInstance()
{
	if(Instance == NULL)
		Instance = new Wizard();
	return Instance;
}

void Wizard::CleanUp()
{
	if(Instance != NULL)
	{
		delete Instance;
		Instance = NULL;
	}

}

void Wizard::StartSDLVideoMode()
{
	if(FrontEnd != NULL)
		return;

	//FrontEnd = new SDLFrontEnd();

	FrontEnd = BackEndFactory::GetInstance()->CreateBackEnd(Simple);

	Resize(FullScreen);
#ifndef WIN32
	// Disable DPMS, screen blanking
	system("/usr/bin/X11/xset -dpms s off");
	system("/usr/bin/X11/xset r off");
#endif
}

void Wizard::GenerateCustomEvent(WM_Event Event)
{
	PushEvent(Event);
}

int Wizard::GetExitCode()
{
	return this->ExitCode;
}

void Wizard::SetExitWithCode(int Code)
{
	AVWizardOptions->GetDictionary()->Set("ExitCode", Code);
	AVWizardOptions->SaveToXMLFile(WizardCommandLineParser::GetInstance()->ConfigFileDefault);

	ExitCode = Code;
	WM_Event Event;
	Event.Quit();
	GenerateCustomEvent(Event);
}

void Wizard::Resize(bool FullScreen)
{
	Width = 640;
	if(AVWizardOptions->GetDictionary()->Exists( "WindowWidth"))
		Width = Utils::StringToInt32(AVWizardOptions->GetDictionary()->GetValue("WindowWidth"));
	Height = 480;
	if(AVWizardOptions->GetDictionary()->Exists( "WindowHeight"))
		Height = Utils::StringToInt32(AVWizardOptions->GetDictionary()->GetValue("WindowHeight"));

	// if there is the same screen resolution, there is no need of resize

	this->Width = Width;
	this->Height = Height;

	FrontEnd->StartVideoMode(Width, Height, FullScreen);	
}

void Wizard::ZoomIn()
{
	WizardPageVideoAdjustSize* ZoomPage = dynamic_cast <WizardPageVideoAdjustSize*> (MainPage);
	if (!ZoomPage)
		return;
	WizardBorder -= BORDER_JUMP;
	if(WizardBorder <= 0)
		WizardBorder = 0;

	LeftBorder-= BORDER_JUMP;
	if(LeftBorder<0)
		LeftBorder = 0;

	TopBorder-= BORDER_JUMP;
	if(TopBorder<0)
		TopBorder = 0;

	if(TopBorder >= 2*WizardBorder)
		TopBorder = 2*WizardBorder;
	if(LeftBorder >= 2*WizardBorder)
		LeftBorder = 2*WizardBorder;
	AVWizardOptions->GetDictionary()->Set("WizardBorder", WizardBorder);
	AVWizardOptions->GetDictionary()->Set("LeftBorder", LeftBorder);
	AVWizardOptions->GetDictionary()->Set("TopBorder", TopBorder);

	ZoomPage->UpdateSelected();	
}

void Wizard::ZoomOut()
{
	WizardPageVideoAdjustSize* ZoomPage = dynamic_cast <WizardPageVideoAdjustSize*> (MainPage);
	if (!ZoomPage)
		return;

	if(WizardBorder*5 >= Width)
		return;
	WizardBorder += BORDER_JUMP;
	LeftBorder+= BORDER_JUMP;
	TopBorder+= BORDER_JUMP;
	AVWizardOptions->GetDictionary()->Set("WizardBorder", WizardBorder);
	AVWizardOptions->GetDictionary()->Set("LeftBorder", LeftBorder);
	AVWizardOptions->GetDictionary()->Set("TopBorder", TopBorder);
	ZoomPage->UpdateSelected();	
}

void Wizard::SetAnalogSoundMode(bool IsAnalogSound)
{
	this->IsAnalogSound = IsAnalogSound;
}

bool Wizard::GetAnalogSoundMode()
{
	return IsAnalogSound;
}


