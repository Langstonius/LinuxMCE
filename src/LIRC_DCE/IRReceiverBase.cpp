#include "IRReceiverBase.h"
#include "DCE/Logger.h"
#include "PlutoUtils/FileUtils.h"
#include "PlutoUtils/StringUtils.h"
#include "PlutoUtils/Other.h"

#include "Gen_Devices/AllCommandsRequests.h"
#include "pluto_main/Define_DeviceTemplate.h"

#include <iostream>
using namespace std;
using namespace DCE;

IRReceiverBase::IRReceiverBase(Command_Impl *pCommand_Impl)
: m_Virtual_Device_Translator()
{
	m_pCommand_Impl=pCommand_Impl;
	m_cCurrentScreen='M';
}

void IRReceiverBase::GetConfig(DeviceData_Impl *pData)
{
	string sMapping;
	DCE::CMD_Get_Remote_Control_Mapping_DT CMD_Get_Remote_Control_Mapping_DT(pData->m_dwPK_Device, DEVICETEMPLATE_Infrared_Plugin_CONST,
		BL_SameHouse, &sMapping);
	m_pCommand_Impl->SendCommand(CMD_Get_Remote_Control_Mapping_DT);  // Get the codes from I/R Plugin

	string::size_type posTab=0;
	while(posTab<sMapping.size())
	{
		char cScreenType = StringUtils::Tokenize(sMapping,"\t",posTab)[0];
		char cRemoteLayout = StringUtils::Tokenize(sMapping,"\t",posTab)[0];
		string sThisMapping = StringUtils::Tokenize(sMapping,"\t",posTab);
		vector<string> vectButtons;
		StringUtils::Tokenize(sThisMapping,"\r\n",vectButtons);
		for(size_t s=0;s<vectButtons.size();++s)
		{
			string::size_type pos=0;
			// We got ourselves a valid one
			string::size_type posSlash=vectButtons[s].find('|');
			while( pos<vectButtons[s].size() )
			{
				string sToken = StringUtils::Tokenize(vectButtons[s],";|",pos);
				if( posSlash!=string::npos )  // Should always be true unless data is malformed
				{
					int iNumberOfArguments;
					char **pArgs = StringUtils::ConvertStringToArgs(vectButtons[s].substr(posSlash+1),iNumberOfArguments);
					if( iNumberOfArguments )  // Should always be true
					{
						g_pPlutoLogger->Write(LV_STATUS,"Layout %c, LIRCBtn %s fires: %s",cScreenType, sToken.c_str(),vectButtons[s].substr(posSlash+1).c_str());
						Message *pMessage = new Message(iNumberOfArguments,pArgs,pData->m_dwPK_Device);
						pMessage->m_dwPK_Device_To = m_Virtual_Device_Translator.TranslateVirtualDevice(pMessage->m_dwPK_Device_To);
						for(size_t s=0;s<pMessage->m_vectExtraMessages.size();++s)
							pMessage->m_vectExtraMessages[s]->m_dwPK_Device_To = m_Virtual_Device_Translator.TranslateVirtualDevice(pMessage->m_vectExtraMessages[s]->m_dwPK_Device_To);

						MapKeysToMessages *pMapKeysToMessages = m_mapKeyMapping[StringUtils::ToUpper(sToken)];
						if( !pMapKeysToMessages )
						{
							pMapKeysToMessages = new MapKeysToMessages();
							m_mapKeyMapping[StringUtils::ToUpper(sToken)] = pMapKeysToMessages;
						}

						(*pMapKeysToMessages)[ make_pair<char,char> (cRemoteLayout,cScreenType) ] = pMessage;
					}
					if( posSlash<=pos )
						break; // We already parsed the last one
				}
			}
		}
	}
}

IRReceiverBase::~IRReceiverBase()
{
	return; // Todo - this crashes???  not too important since it's just memory cleanup on exit
	for(map<string,MapKeysToMessages *>::iterator it=m_mapKeyMapping.begin();it!=m_mapKeyMapping.end();++it)
	{
		MapKeysToMessages *pMapKeysToMessages = it->second;
		if( pMapKeysToMessages )
		{
			for(MapKeysToMessages::iterator it2=pMapKeysToMessages->begin();it2!=pMapKeysToMessages->end();++it)
				delete it2->second;
		}

		delete (*it).second;
	}
}

void IRReceiverBase::ReceivedCode(int PK_Device_Remote,const char *pCode)
{
	char cRemoteLayout = m_mapRemoteLayout[PK_Device_Remote];
	map<string,MapKeysToMessages *>::iterator it = m_mapKeyMapping.find(StringUtils::ToUpper(pCode));
	if( it!=m_mapKeyMapping.end() )
	{
		MapKeysToMessages *pMapKeysToMessages = it->second;
		MapKeysToMessages::iterator itMessage;
		
		itMessage = pMapKeysToMessages->find(make_pair<char,char> (cRemoteLayout,m_cCurrentScreen) );
		if( itMessage==pMapKeysToMessages->end() )
			itMessage = pMapKeysToMessages->find(make_pair<char,char> (cRemoteLayout,0) );
		if( itMessage==pMapKeysToMessages->end() )
			itMessage = pMapKeysToMessages->find(make_pair<char,char> (0,m_cCurrentScreen) );
		if( itMessage==pMapKeysToMessages->end() )
			itMessage = pMapKeysToMessages->find(make_pair<char,char> (0,0) );

		if( itMessage!=pMapKeysToMessages->end() )
		{
			Message *pm = itMessage->second;
			g_pPlutoLogger->Write(LV_STATUS,"Sending Message Type %d ID %d",pm->m_dwMessage_Type,pm->m_dwID);
			m_pCommand_Impl->QueueMessageToRouter(new Message(pm));
		}
		else
			g_pPlutoLogger->Write(LV_WARNING,"No mapping for that code");
	}
	else
		g_pPlutoLogger->Write(LV_WARNING,"Cannot find code %s",pCode);
}

