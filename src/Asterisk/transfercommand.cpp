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
// C++ Implementation: originatestatemachine
//
// Description: 
//
//
// Author:  <igor@dexx>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "transfercommand.h"

#include "DCE/Logger.h"

#include "asteriskconsts.h"
#include "asteriskmanager.h"
#include "utils.h"

using namespace DCE;

namespace ASTERISK {

TransferCommand::TransferCommand()
{
	g_pPlutoLogger->Write(LV_STATUS, "Transfer command created.");
};

TransferCommand::~TransferCommand() {
	g_pPlutoLogger->Write(LV_STATUS, "Transfer command destroyed.");
};

void 
TransferCommand::setExtenNum(std::string extennum) {
	LOCKED_OP(this->extennum = extennum);
}

void 
TransferCommand::setChannel(std::string channel) {
	LOCKED_OP(this->channel = channel;);
}

void 
TransferCommand::setCommandID(int commandid) {
	LOCKED_OP(this->commandid = commandid;);
}

void
TransferCommand::handleStartup() {
	Token transtok;
	
	LOCKED_OP(
	transtok.setKey(TOKEN_ACTION, ACTION_REDIRECT);
	transtok.setKey(TOKEN_CHANNEL, channel );
	transtok.setKey(TOKEN_EXTEN, extennum);
	transtok.setKey(TOKEN_CONTEXT, "trusted");
	transtok.setKey("Priority", "1");
	);

	sendToken(&transtok);
	
	AddRef(); // do not destroy until job finished
};

int 
TransferCommand::handleToken(Token* ptoken) {
	return 0;
}

void 
TransferCommand::handleTerminate() {
	g_pPlutoLogger->Write(LV_STATUS, "Transfer completed.");	
    Release();
}

};
