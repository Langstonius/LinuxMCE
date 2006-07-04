/**
 *	Created by CipLogic < ciprian dot m at plutohome dot com >
 */
#ifndef WizardPageVideoResolution_H_
#define WizardPageVideoResolution_H_

#include "WizardPage.h"

#include "WizardWidgetListBox.h"

class WizardPageVideoResolution :
	public WizardPage
{
	WizardWidgetListBox* Selected;
	SettingsDictionary* GlobalSettings;

	std::string PreviousResolution;
public:

	virtual void DefaultSetup(SettingsDictionary* AVWizardSettings);

	virtual int DoApplySetting(SettingsDictionary* Dictionary);

	virtual void DoIncreaseSetting();
	virtual void DoDecreaseSetting();
	virtual void DoNextFocusItem();
	virtual void DoPreviousFocusItem();

	WizardPageVideoResolution(SDLFrontEnd* FrontEnd, std::string Name);
	virtual ~WizardPageVideoResolution();
};

#endif
