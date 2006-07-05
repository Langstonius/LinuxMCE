#ifndef IRReceiverBase_h
#define IRReceiverBase_h

/* Since we will have multiple IRReceiver devices, put all the common functionality regarding looking
up codes in a shared base class */

#include "DCE/Command_Impl.h"
#include "DCE/Virtual_Device_Translator.h"

typedef map< pair<char,char>,Message *> MapKeysToMessages;  // ScreenType,RemoteLayout

namespace DCE
{
	class IRReceiverBase
	{
	protected:
		map<int,char> m_mapRemoteLayout;
		Command_Impl *m_pCommand_Impl;
		Virtual_Device_Translator m_Virtual_Device_Translator;
		map<string,MapKeysToMessages *> m_mapKeyMapping;
		char m_cCurrentScreen;
 		std::map<std::string, std::string> m_mapAVWCommands;

		IRReceiverBase(Command_Impl *pCommand_Impl);
		~IRReceiverBase();
        
		void ReceivedCode(int PK_Device_Remote,const char *pCode);
		void GetConfig(DeviceData_Impl *pData);
		void ForceKeystroke(string sCommand, string sAVWHost, int iAVWPort);
		bool SendSocketText(int Socket, std::string Text) const;
		std::string GetSocketText(int Socket) const;
	};
}

#endif
