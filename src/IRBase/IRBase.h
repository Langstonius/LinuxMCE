#ifndef IRBASE
#define IRBASE

#include "DCE/Command_Impl.h"
#include "MessageTranslation/AVMessageTranslation.h"
#include "PlutoUtils/MultiThreadIncludes.h"
#include "PlutoUtils/Other.h"

using namespace DCE;

string ConvertRC5_6(string sCode);  // From GenerateRcX

class IRBase : public AVMessageProcessor {
	Command_Impl *m_pCommand_Impl;
public:
	IRBase() { m_bMustConvertRC5_6=false; }
	virtual ~IRBase() {};

public:
	typedef pair<long, long> longPair;
	map<longPair, std::string>& getCodeMap() {
		return codemap_;
	}
	
		
protected:
	virtual bool Translate(MessageReplicator& inrepl, MessageReplicatorList& outrepls);
	virtual void DispatchMessage(Message* pmsg);
	Command_Impl *getCommandImpl() { return m_pCommand_Impl; };
	void setCommandImpl(Command_Impl *pCommand_Impl) { m_pCommand_Impl=pCommand_Impl; };
	
protected:
	/*exposed methods*/
	virtual void SendIR(string Port, string IRCode) = 0;

	virtual void handleStart();
	virtual void handleStart(Command_Impl *pCommand_Impl);
	virtual void handleStop();

private:
	void PushSendCodeMessage(Message* porigmsg, MessageReplicatorList& outrepls, 
				long cmdid, const std::string& port, int count = 1, int delay = 0);
	
private:
	map<longPair, std::string> codemap_;		// maps device,command pair to IR code
	std::map<int, int> map_DigitDelay;
	std::map<int, string> map_NumericEntry;

protected:
	bool m_bMustConvertRC5_6;
};

#endif
