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
// C++ Interface: originatestatemachine
//
// Description: 
//
//
// Author:  <igor@dexx>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASTERISKHANGUPSTATEMACHINE_H
#define ASTERISKHANGUPSTATEMACHINE_H

#include "tokenmanager.h"

#include <string>

namespace ASTERISK {

/**
@author 
*/
class HangupCommand : public TokenManager<HangupCommand>, public ThreadSafeObj
{
	friend class TokenManager<HangupCommand>;
protected:
	HangupCommand();
	virtual ~HangupCommand();

public:
	void setChannel(std::string channel);
	void setCommandID(int commandid);
	
protected:
	virtual void handleStartup();
	virtual int handleToken(Token* ptoken);
	virtual void handleTerminate();

private:
	std::string channel;
	
	int commandid;

private:
};

};

#endif
