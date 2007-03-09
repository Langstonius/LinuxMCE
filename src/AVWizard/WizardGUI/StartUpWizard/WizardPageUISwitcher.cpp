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
//
// C++ Implementation: WizardPageUISwitcher
//
// Description: 
//
//
// Author: Ciprian Mustiata,,, <ciplogic@ciplogic-desktop>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "WizardPageUISwitcher.h"

#include "WizardPageDTSTest.h"

#include "GUIWizardUtils.h"

#include <iostream>

#include "WizardWidgetLabel.h"
#include "WizardWidgetScrollList.h"
#include "WizardWidgetImage.h"
#include "Wizard.h"

WizardPageUISwitcher::WizardPageUISwitcher(GenericBackEnd* FrontEnd, std::string Name)
: WizardPage(FrontEnd, Name)
{
#ifdef DEBUG
	std::cout<<"WizardPageDTSTest::WizardPageDTSTest()"<<std::endl;
#endif
	Buttons["UI Version 2"] = 1;
	Buttons["Simple Interface"] = 2;
	OutputValue = "Yes, I can hear it";
	Selected = NULL;
}

WizardPageUISwitcher::~WizardPageUISwitcher(void)
{
#ifdef DEBUG
	std::cout<<"WizardPageDTSTest::~WizardPageDTSTest()"<<std::endl;
#endif
}

/*virtual*/ int WizardPageUISwitcher::DoApplySetting(SettingsDictionary* Dictionary)
{
#ifdef DEBUG
	std::cout<<"WizardPageDTSTest::DoApplySetting()"<<std::endl;
#endif
	WizardWidgetScrollList* List = dynamic_cast<WizardWidgetScrollList*> (Page->GetChildRecursive("UIScroll"));
	if(NULL == List)
	{
#ifdef DEBUG
		std::cout<<"Warning: WizardPageDTSTest::DefaultSetup()"<<std::endl;
#endif
		return -1;
	}

	OutputValue = List->GetSelectedValue();
	Dictionary->Set("UIVersion", OutputValue);
	return 0;
}

/*virtual*/ void WizardPageUISwitcher::DefaultSetup(SettingsDictionary* AVWizardSettings)
{
#ifdef DEBUG
	std::cout<<"WizardPageDTSTest::DefaultSetup()"<<std::endl;
#endif
	WizardWidgetScrollList* List = dynamic_cast<WizardWidgetScrollList*> (Page->GetChildRecursive("UIScroll"));
	if(NULL == List)
	{
#ifdef DEBUG
		std::cout<<"Warning: WizardPageDTSTest::DefaultSetup()"<<std::endl;
#endif
		return;
	}
	List->AddItem("Static images, no overlay (lightweight)", "UI1");
	List->AddItem("OpenGL with overlay (medium)", "UI2_med");
	List->AddItem("OpenGL with alpha blending (high-end)", "UI2_hi");
	List->SetFocus(true);
}

/*virtual*/ void WizardPageUISwitcher::DoIncreaseSetting()
{
	WizardWidgetScrollList* List = dynamic_cast<WizardWidgetScrollList*> (Page->GetChildRecursive("UIScroll"));
	if(NULL == List)
	{
#ifdef DEBUG
		std::cout<<"Warning: WizardPageDTSTest::DefaultSetup()"<<std::endl;
#endif
		return;
	}
	List->SetItemIndex(List->GetItemIndex()+1);
}

/*virtual*/ void WizardPageUISwitcher::DoDecreaseSetting()
{
	WizardWidgetScrollList* List = dynamic_cast<WizardWidgetScrollList*> (Page->GetChildRecursive("UIScroll"));
	if(NULL == List)
	{
#ifdef DEBUG
		std::cout<<"Warning: WizardPageDTSTest::DefaultSetup()"<<std::endl;
#endif
		return;
	}
	List->SetItemIndex(List->GetItemIndex()-1);
}

/*virtual*/ void WizardPageUISwitcher::DoNextFocusItem()
{
	WizardWidgetScrollList* List = dynamic_cast<WizardWidgetScrollList*> (Page->GetChildRecursive("UIScroll"));
	if(NULL == List)
	{
#ifdef DEBUG
		std::cout<<"Warning: WizardPageDTSTest::DefaultSetup()"<<std::endl;
#endif
		return;
	}
	List->SetItemIndex(List->GetItemIndex()+1);
}

/*virtual*/ void WizardPageUISwitcher::DoPreviousFocusItem()
{
	WizardWidgetScrollList* List = dynamic_cast<WizardWidgetScrollList*> (Page->GetChildRecursive("UIScroll"));
	if(NULL == List)
	{
#ifdef DEBUG
		std::cout<<"Warning: WizardPageDTSTest::DefaultSetup()"<<std::endl;
#endif
		return;
	}
	List->SetItemIndex(List->GetItemIndex()-1);
}

/*virtual*/ void WizardPageUISwitcher::DoClickWidget(WizardWidgetBase *pWidget)
{
	std::vector<WizardWidgetBase*>::iterator Item;
	WizardWidgetButton* NewSelectedButton = NULL;
	WizardWidgetImage* NewSelectedImage = NULL;
	for (Item = Page->Children.begin(); Item < Page->Children.end(); ++Item)
	{
		if (*Item == pWidget)
		{
			NewSelectedButton = dynamic_cast<WizardWidgetButton*>(pWidget);
			NewSelectedImage = dynamic_cast<WizardWidgetImage*>(pWidget);
			if (NewSelectedButton == NULL && NewSelectedImage == NULL)
				break;

			if (NewSelectedButton != NULL)
			{
				if (NewSelectedButton->GetName() == "BtnOK")
				{
					SDL_Event Event;
					Event.type = SDL_KEYUP;
					Event.key.state = SDL_RELEASED;
					Event.key.keysym.sym = SDLK_RETURN;
					SDL_PushEvent(&Event);
				}
			}
			else if (NewSelectedImage != NULL)
			{
				if (NewSelectedImage->GetName() == "UI_Up")
				{
					SDL_Event Event;
					Event.type = SDL_KEYUP;
					Event.key.state = SDL_RELEASED;
					Event.key.keysym.sym = SDLK_UP;
					SDL_PushEvent(&Event);
				}
				else if (NewSelectedImage->GetName() == "UI_Down")
				{
					SDL_Event Event;
					Event.type = SDL_KEYUP;
					Event.key.state = SDL_RELEASED;
					Event.key.keysym.sym = SDLK_DOWN;
					SDL_PushEvent(&Event);
				}
			}
		}
	}
}
