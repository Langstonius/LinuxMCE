#include "WizardPagesFactory.h"
//---------------------------------------------------------------------------
#include <stdlib.h>
//---------------------------------------------------------------------------
#include "WizardWidgetsFactory.h"
#include "GenerateWizardConfigDefaults.h"
//---------------------------------------------------------------------------
//classes for factory
#include "WizardPageWelcome.h"
#include "WizardPageVideoRatio.h"
#include "WizardPageVideoOutput.h"
#include "WizardPageVideoResolution.h"
#include "WizardPageVideoAdjustSize.h"
#include "WizardPageAudioConnector.h"
#include "WizardPageAudioVolume.h"
#include "WizardPageDolbyTest.h"
#include "WizardPageDTSTest.h"
#include "WizardPageFinalSelections.h"

WizardPagesFactory* WizardPagesFactory::Instance = NULL;



WizardPagesFactory::WizardPagesFactory()
	: WizardWidgetsFactory()
{
	this->RuningMode = 0;

	PageNames[0] = "Welcome.xml";
	PageNames[1] = "VideoRatio.xml";
	PageNames[2] = "VideoResolution.xml";
	PageNames[3] = "VideoOutput.xml";
	PageNames[4] = "VideoAdjustSize.xml";
	PageNames[5] = "AudioConnector.xml";
	PageNames[6] = "AudioVolume.xml";
	PageNames[7] = "DolbyTest.xml";
	PageNames[8] = "DTSTest.xml";
	PageNames[9] = "FinalSelections.xml";

	std::string TmpPrefix = "/tmp/";
	for(int i = 0; i < WIZARD_NO_PAGES; i++)
		PageNames[i] = TmpPrefix + PageNames[i];
}

WizardPagesFactory::~WizardPagesFactory(void)
{
}


/*static*/ WizardPagesFactory* WizardPagesFactory::GetInstance()
{
	if(Instance == NULL)
		Instance = new WizardPagesFactory();
	return Instance;
}

void WizardPagesFactory::SetRunMode(int RunMode)
{
	this->RuningMode = RunMode;
}

void WizardPagesFactory::SetSDLFrontEnd(SDLFrontEnd* FrontEnd)
{
	this->FrontEnd = FrontEnd;
}

WizardPage* WizardPagesFactory::CreateWizardPage(std::string XMLFile)
{
	SettingsDictionaryTree* DialogSettings = new SettingsDictionaryTree();
	if(!DialogSettings->LoadFromXMLFile(XMLFile))
		return NULL;

	WizardPage* Result = CreateWidget(DialogSettings);
	delete DialogSettings;
	DialogSettings = NULL;

    return Result;
}

//---------------------------------------------------------------------------
WizardPage* WizardPagesFactory::CreateWidget(SettingsDictionaryTree* 
													 DialogSource)
{
	if(DialogSource == NULL)
		return NULL;

	SettingsDictionary* Dictionary = DialogSource->GetDictionary();
	std::string Name = Dictionary->GetName();

	WizardWidgetsFactory*Factory =  WizardWidgetsFactory::GetInstance();
	Factory->SetSDLFrontEnd(FrontEnd);

	WizardWidgetBase* Page = dynamic_cast<WizardWidgetBase*> 
		(Factory->CreateWidget(DialogSource));

	Factory->CleanUp();
	if(Page == NULL)
		return NULL;

	Page->ApplyDictionary(Dictionary);

	WizardPage* Result = new WizardPage(FrontEnd, Name);
	Result->SetPageLayout(dynamic_cast<WizardWidgetPage*>(Page));


	return Result;
}


WizardPage* WizardPagesFactory::CreatePredefinedWizardPage(int IndexPage)
{
	if (IndexPage<1 || IndexPage>WIZARD_NO_PAGES)
		return NULL;


	SettingsDictionaryTree* DialogSettings = new SettingsDictionaryTree();
	if(!DialogSettings->LoadFromXMLFile(PageNames[IndexPage-1]))
	{
		delete DialogSettings;
		return NULL;
	}

	WizardPage* Result = NULL;

	SettingsDictionary* Dictionary = DialogSettings->GetDictionary();
	std::string PageName = Dictionary->GetName();

	WizardWidgetsFactory*Factory =  WizardWidgetsFactory::GetInstance();
	Factory->SetSDLFrontEnd(FrontEnd);

	WizardWidgetBase* Page = dynamic_cast<WizardWidgetBase*> 
		(Factory->CreateWidget(DialogSettings));

	Factory->CleanUp();
	if(Page == NULL)
		return NULL;

	Page->ApplyDictionary(Dictionary);


	switch(IndexPage) {
		case PAGE_WELCOME:
			Result = new WizardPageWelcome(FrontEnd, PageName);
			break;
		case PAGE_VIDEORATIO:
			Result = new WizardPageVideoRatio(FrontEnd, PageName);
			break;
		case PAGE_VIDEOOUTPUT:
			Result = new WizardPageVideoOutput(FrontEnd, PageName);
			break;
		case PAGE_VIDEORESOLUTION:
			Result = new WizardPageVideoResolution(FrontEnd, PageName);
			break;
		case PAGE_VIDEOADJUSTSIZE:
			Result = new WizardPageVideoAdjustSize(FrontEnd, PageName);
			break;
		case PAGE_AUDIOCONNECTOR:
			Result = new WizardPageAudioConnector(FrontEnd, PageName);
			break;
		case PAGE_AUDIOVOLUME:
			Result = new WizardPageAudioVolume(FrontEnd, PageName);
			break;
		case PAGE_DOLBYTEST:
			Result = new WizardPageDolbyTest(FrontEnd, PageName);
			break;
		case PAGE_DTSTEST:
			Result = new WizardPageDTSTest(FrontEnd, PageName);
			break;
		case PAGE_FINALSELECTIONS:
			Result = new WizardPageFinalSelections(FrontEnd, PageName);
			break;
	}

	Result->SetPageLayout(dynamic_cast<WizardWidgetPage*>(Page));

	return Result;
}
