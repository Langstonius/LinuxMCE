#include "WizardPageVideoOutput.h"

#include "GUIWizardUtils.h"

WizardPageVideoOutput::WizardPageVideoOutput(GenericBackEnd* FrontEnd, std::string Name)
: WizardPage(FrontEnd, Name)
{
	Buttons["Composite"] = 1;
	Buttons["S-Video"] = 2;
	Buttons["Component"] = 3;
	Buttons["VGA"] = 4;
	Buttons["DVI"] = 5;
	OutputValue = "VGA";
	Selected = NULL;
}

WizardPageVideoOutput::~WizardPageVideoOutput(void)
{
}


/*virtual*/ int WizardPageVideoOutput::DoApplySetting(SettingsDictionary* Dictionary)
{
	if(Dictionary == NULL)
		return -1;
	OutputValue = Selected->GetCaption();
	Dictionary->Set("VideoOutput", OutputValue);
	return 0;
}

/*virtual*/ void WizardPageVideoOutput::DefaultSetup(SettingsDictionary* AVWizardSettings)
{
	if(!AVWizardSettings->Exists("VideoOutput"))
	{
		SearchSelectedItem();
		return;
	}
	OutputValue = AVWizardSettings->GetValue("VideoOutput");
	SearchSelectedItem();
}

/*virtual*/ void WizardPageVideoOutput::DoIncreaseSetting()
{
	std::string IndexText = Utils::CopyStr(Selected->GetName().c_str(), 3, 1);
	int ButtonIndex = Utils::StringToInt32(IndexText);
	if(ButtonIndex == 5)
		return;
	Selected->SetFocus(false);
	ButtonIndex++;
	std::string ButtonName = "Btn"+Utils::Int32ToString(ButtonIndex);
	Selected = dynamic_cast<WizardWidgetButton*> (Page->GetChildRecursive(ButtonName));
	Selected->SetFocus(true);	
}

/*virtual*/ void WizardPageVideoOutput::DoDecreaseSetting()
{
	std::string IndexText = Utils::CopyStr(Selected->GetName().c_str(), 3, 1);
	int ButtonIndex = Utils::StringToInt32(IndexText);
	if(ButtonIndex == 1)
		return;
	Selected->SetFocus(false);
	ButtonIndex--;
	std::string ButtonName = "Btn"+Utils::Int32ToString(ButtonIndex);
	Selected = dynamic_cast<WizardWidgetButton*> (Page->GetChildRecursive(ButtonName));
	Selected->SetFocus(true);	
}

/*virtual*/ void WizardPageVideoOutput::SearchSelectedItem()
{
	int ButtonIndex = Buttons[OutputValue];
	if(ButtonIndex == 0)
		return;
	std::string ButtonName = "Btn"+Utils::Int32ToString(ButtonIndex);
	Selected = dynamic_cast<WizardWidgetButton*> (Page->GetChildRecursive(ButtonName));
	if(Selected != NULL)
		Selected->SetFocus(true);	
}

/*virtual*/ void WizardPageVideoOutput::DoClickWidget(WizardWidgetBase *pWidget)
{
	std::vector<WizardWidgetBase*>::iterator Item;
	WizardWidgetButton* NewSelectedButton = NULL;
	for (Item = Page->Children.begin(); Item < Page->Children.end(); ++Item)
	{
		if (*Item == pWidget)
		{
			NewSelectedButton = dynamic_cast<WizardWidgetButton*>(pWidget);
			if (NewSelectedButton)
			{
				Selected->SetFocus(false);
				Selected = NewSelectedButton;
				Selected->SetFocus(true);

				SDL_Event Event;
				Event.type = SDL_KEYUP;
				Event.key.state = SDL_RELEASED;
				Event.key.keysym.sym = SDLK_RETURN;
				SDL_PushEvent(&Event);
			}
		}
	}
}
