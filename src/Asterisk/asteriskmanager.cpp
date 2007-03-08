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
// C++ Implementation: asteriskmanager
//
// Description: 
//
//
// Author:  <igor@dexx>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "asteriskmanager.h"
#include "DCE/Logger.h"

#include "Asterisk.h"

#include "logincommand.h"
#include "originatecommand.h"
#include "hangupcommand.h"
#include "transfercommand.h"
#include "conferencecommand.h"
#include "ringdetecthandler.h"

#define DEFAUL_LOGIN_NAME "admin"
#define DEFAUL_LOGIN_SECRET "adminsecret"

#define KEY_RESPONSE "Response"
#define KEY_MESSAGE "Message"

#define RESPONSE_SUCCESS "Success"
#define RESPONSE_ERROR "Error"

using namespace DCE;

namespace ASTERISK {

AsteriskManager* AsteriskManager::sinstance = 0;


AsteriskManager::AsteriskManager() {
}

AsteriskManager::~AsteriskManager() {
	Release();
}


void AsteriskManager::Originate(const string sPhoneNumber, const string sOriginatorNumber,
				const string sOriginatorType, const string sCallerID, int iCommandID) {
	
	/*originate*/
	TokenSMPtr<OriginateCommand> poriginate = OriginateCommand::getInstance();
	poriginate->setPhoneNum(sOriginatorNumber);
	poriginate->setPhoneType(sOriginatorType);
	poriginate->setExtenNum(sPhoneNumber);
	poriginate->setCallerID(sCallerID);
	poriginate->setCommandID(iCommandID);

	poriginate->Run(false);
}

void AsteriskManager::Hangup(const std::string sChannel, 
				int iCommandID) {
	
	/*hangup*/
	TokenSMPtr<HangupCommand> phangup = HangupCommand::getInstance();
	phangup->setChannel(sChannel);
	phangup->setCommandID(iCommandID);

	phangup->Run(false);
}

void AsteriskManager::Transfer(const std::string sChannel, const string sPhoneNumber, int iCommandID) {
	
	/*transfer*/
	TokenSMPtr<TransferCommand> ptransfer = TransferCommand::getInstance();
	ptransfer->setExtenNum(sPhoneNumber);
	ptransfer->setChannel(sChannel);
	ptransfer->setCommandID(iCommandID);

	ptransfer->Run(false);
}

void AsteriskManager::Conference(const std::string sChannel1, const std::string sChannel2, const string sPhoneNumber, int iCommandID) {
	
	/*conference*/
	TokenSMPtr<ConferenceCommand> ptransfer = ConferenceCommand::getInstance();
	ptransfer->setExtenNum(sPhoneNumber);
	ptransfer->setChannel1(sChannel1);
	ptransfer->setChannel2(sChannel2);
	ptransfer->setCommandID(iCommandID);

	ptransfer->Run(false);
}

	
void AsteriskManager::NotifyRing(std::string sCallerID, const std::string sSrcExt, 
												const std::string sChannel) {
	if(!pAsterisk) {
	    return;
	}
	pAsterisk->EVENT_PBX_Ring(sSrcExt, sChannel, sCallerID);
}

void AsteriskManager::NotifyHangup(const std::string sSrcExt) {
	if(!pAsterisk) {
	    return;
	}
	pAsterisk->EVENT_PBX_Hangup(sSrcExt);
}

void AsteriskManager::NotifyResult(int iCommandID, int iResult, 
						const std::string Message) 
{	
	if(!pAsterisk) {
	    return;
	}
	pAsterisk->EVENT_PBX_CommandResult(iCommandID, iResult, Message);
}


AsteriskManager* AsteriskManager::getInstance() {
	static AsteriskManager manager;
	
	LOCK_OBJ(manager);
	
	if(sinstance == 0) {
		sinstance = &manager;
	}
	
	UNLOCK_OBJ(manager);
	
	return sinstance;
}

void AsteriskManager::Initialize(DCE::Asterisk* lpAsterisk) {
	this->pAsterisk = lpAsterisk;

	/*login*/
	/* removed - login is handled automaticaly
	TokenSMPtr<LoginCommand> plogin = LoginCommand::getInstance();
	plogin->setUserName(DEFAUL_LOGIN_NAME);
	plogin->setSecret(DEFAUL_LOGIN_SECRET);
	if(plogin->Run()) {
	       LoggerWrapper::GetInstance()->Write(LV_CRITICAL, "Login to asterisk manager failed");
	       return;
	}

	LoggerWrapper::GetInstance()->Write(LV_STATUS, "Login successfull.");
	*/


    /*initialize persistent SMs*/
	TokenSMPtr<RingDetectHandler> pring = RingDetectHandler::getInstance();
	pring->Run(false);
}

void AsteriskManager::Release() {
	this->pAsterisk = 0;
}
		

};
