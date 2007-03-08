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
// C++ Implementation: ringdetectstatemachine
//
// Description: 
//
//
// Author:  <igor@dexx>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "ringdetecthandler.h"

#include "DCE/Logger.h"

#include "asteriskconsts.h"
#include "asteriskmanager.h"
#include "runtimeconfig.h"
#include "utils.h"

#define MARK_DIALING " DIAL/"

using namespace std;
using namespace DCE;

namespace ASTERISK {

RingDetectHandler::RingDetectHandler()
{
	LoggerWrapper::GetInstance()->Write(LV_STATUS, "Ring Detect SM created.");
};

RingDetectHandler::~RingDetectHandler() {
	LoggerWrapper::GetInstance()->Write(LV_STATUS, "Ring Detect SM destroyed.");
};

	
void 
RingDetectHandler::handleStartup() {
	AddRef();
}

int 
RingDetectHandler::handleToken(Token* ptoken) {
	AsteriskManager* manager = AsteriskManager::getInstance();
	if(ptoken->getKey(TOKEN_EVENT) == EVENT_NEWEXTEN &&
		ptoken->getKey(TOKEN_APPLICATION) == APPLICATION_DIAL)
	{
		string party = ptoken->getKey(TOKEN_APPDATA);
		while (party != "")
		{
			string rest=party;
			string extension;
			if(!Utils::ParseParty(party, &extension,&rest)) {
				if(atoi(extension.c_str())>0)
				{
					string channel = ptoken->getKey(TOKEN_CHANNEL);
					map_ringext[extension] += string(" ")+channel;
					LoggerWrapper::GetInstance()->Write(LV_STATUS, "Will connect channel %s to extension %s", channel.c_str(),extension.c_str());
				}
				else
				{
					string channel = ptoken->getKey(TOKEN_CHANNEL);
					string ringphoneid;
					if(!Utils::ParseChannel(channel, &ringphoneid))
					{
						if(map_ringext.find(ringphoneid) != map_ringext.end())
						{
							int pos = party.rfind('/');
							if(pos>=0)
							{
								string number = party.substr(pos+1,party.length());
								map_ringext[ringphoneid] += string(MARK_DIALING)+number+string("/ ");
								LoggerWrapper::GetInstance()->Write(LV_STATUS, "Will mark channel %s as dialing to %s", channel.c_str(),number.c_str());								
							}
						}
					}
				}
			} else {
				LoggerWrapper::GetInstance()->Write(LV_CRITICAL, "Error parsing party:%s", party.c_str());
				return 0;
			}
			party = rest;
		}
	}
	if(ptoken->getKey(TOKEN_EVENT) == EVENT_HANGUP)
	{
		string channel = ptoken->getKey(TOKEN_CHANNEL);
		bool success=false;
		int pos = channel.find("AsyncGoto");
		int pos1 = channel.find("Local/");
		int pos2 = channel.find("@trusted");
		int newpos = 0;
		
		LoggerWrapper::GetInstance()->Write(LV_STATUS, "Hangup: %s, %d, %d, %d", channel.c_str(), pos, pos1, pos2);
		
		// Don't hangup if it's a conference transfer
		// Don't hangup if it's a local trusted channel, it comes before joining to a conference
		// See the dial plan for more details
		if((pos1 != 0 || pos2 == string::npos) && pos == string::npos)
		{
			list<map<string,string>::iterator> delete_list;		
			for(map<string,string>::iterator it=map_ringext.begin();it!=map_ringext.end();it++)
			{
				string exten=(*it).first;
				string newchan=(*it).second;
				pos = newchan.find(channel);
				if( pos >=0)
				{
					LoggerWrapper::GetInstance()->Write(LV_STATUS, "Will delete channel %s from extension %s", channel.c_str(),(*it).first.c_str());
					newchan = newchan.substr(0,pos)+newchan.substr(pos+channel.length(),newchan.length());
					LoggerWrapper::GetInstance()->Write(LV_STATUS, "Change from [%s] to [%s]",(*it).second.c_str(),newchan.c_str());
					(*it).second = newchan;

					if ((*it).second.find_first_not_of(' ')<0)
					{
						delete_list.push_back(it);
						LoggerWrapper::GetInstance()->Write(LV_STATUS, "Will delete as empty");
					}
					else
					{
					
						string ringphoneid;
						if(!Utils::ParseChannel(channel, &ringphoneid))
						{
						    if(ringphoneid == exten)
						    {
							delete_list.push_back(it);
							LoggerWrapper::GetInstance()->Write(LV_STATUS, "Will delete as same extension");
							continue;
						    }
						}
						
						
						pos=newchan.find(MARK_DIALING);
						if(pos>=0)
						{
							newpos = newchan.find(' ',pos+1);
							newchan=newchan.substr(0,pos)+newchan.substr(newpos,newchan.length());
							if((newpos=newchan.find_first_not_of(' '))<0)
							{
								delete_list.push_back(it);
								LoggerWrapper::GetInstance()->Write(LV_STATUS, "Will delete empty DIAL");
	    						}
							else
							{
								LoggerWrapper::GetInstance()->Write(LV_STATUS, "DIAL is not empty '%s' %d",newchan.c_str(),newpos);
							}
						}
					}
					success=true;
				}
				string ringphoneid;
				Utils::ParseChannel(channel, &ringphoneid);
				LoggerWrapper::GetInstance()->Write(LV_STATUS, "Will notify hangup on %s",ringphoneid.c_str());
				manager->NotifyHangup(ringphoneid);
			}
			for(list<map<string,string>::iterator>::iterator it=delete_list.begin();it!=delete_list.end();it++)
			{
				map_ringext.erase(*it);
			}
			if(!success)
			{
				if(channel.find("Local")<0)
				{
					LoggerWrapper::GetInstance()->Write(LV_CRITICAL, "Hangup on unknown channel %s", channel.c_str());
				}
				return 0;
			}
		}
	}

	if((ptoken->getKey(TOKEN_EVENT) == EVENT_NEWCHANNEL || ptoken->getKey(TOKEN_EVENT) == EVENT_NEWSTATE ) && (ptoken->getKey(TOKEN_STATE) == STATE_RINGING || ptoken->getKey(TOKEN_STATE) == STATE_RING)) 
	{
		string channel = ptoken->getKey(TOKEN_CHANNEL);

		LoggerWrapper::GetInstance()->Write(LV_STATUS, "Channel Ringing: %s", channel.c_str());
		int pos=channel.find("Local");
		if(pos>=0)
		{
			LoggerWrapper::GetInstance()->Write(LV_STATUS, "Ignore call on Local channel, wait for real one %d in %s ",pos,channel.c_str());
			return 0;
		}
		string ringphoneid;
		if(!Utils::ParseChannel(channel, &ringphoneid)) {
			if(map_ringext.find(ringphoneid) == map_ringext.end())
			{
//					LoggerWrapper::GetInstance()->Write(LV_WARNING, "No previos ring to this channel !!!");
				map_ringext[ringphoneid]="";
//					return 0;
			}
			map_ringext[ringphoneid] += string(" ")+channel;
			if(ptoken->getKey(TOKEN_STATE) == STATE_RINGING)
			{
				string callerid = ptoken->getKey(TOKEN_CALLERID);
				if(callerid.find_first_of("0123456789") < 0)
				{
					callerid = "";	
					channel = map_ringext[ringphoneid];
					int oldpos=0;				

					string tmp1, tmp2;
					do
					{
						pos = channel.find(' ',oldpos);
						if(pos <0)
						{
							tmp1 = channel.substr(oldpos, channel.length());
						}
						else
						{
							tmp1= channel.substr(oldpos, pos - oldpos);
						}
						if(!Utils::ParseChannel(tmp1, &tmp2))
						{
							if(tmp2 != ringphoneid)
							{
								callerid += tmp2+" ";
							}
						}
						oldpos = pos+1;

					}
					while(pos>=0);
				}
				if(RuntimeConfig::getInstance()->isCallOriginating(ringphoneid)) {
					LoggerWrapper::GetInstance()->Write(LV_STATUS, "Phone %s is originating a call. Skipping issue Ring event.",
							ringphoneid.c_str());
				} else {
					manager->NotifyRing(callerid, ringphoneid, map_ringext[ringphoneid]);
					LoggerWrapper::GetInstance()->Write(LV_STATUS, "Phone %s is Ringing. Fire Ring event. with callerid %s",ringphoneid.c_str(),callerid.c_str());
				}
			}

			/* as idea 	:  we need both  map_ringext[ringphoneid] and channel, and use one or another or both depending on situation */
		}
		else {
			LoggerWrapper::GetInstance()->Write(LV_CRITICAL, "Error parsing channel:%s", channel.c_str());
		}
	}
	if(ptoken->getKey(TOKEN_EVENT) == EVENT_LINK) 
	{
		string channel1 = ptoken->getKey(TOKEN_CHANNEL1);
		string channel2 = ptoken->getKey(TOKEN_CHANNEL2);
		

		LoggerWrapper::GetInstance()->Write(LV_STATUS, "Linking channels %s and %s", channel1.c_str(),channel2.c_str());
		
		string ringphoneid;
		int chan=1;
		if(Utils::ParseChannel(channel1, &ringphoneid)) {
			chan=2;
			if(Utils::ParseChannel(channel2, &ringphoneid)) {
				LoggerWrapper::GetInstance()->Write(LV_CRITICAL, "Could not extract phone id from channels");
				return 0;
			}
		}
		if(map_ringext.find(ringphoneid) == map_ringext.end())
		{
			LoggerWrapper::GetInstance()->Write(LV_WARNING, "No previos ring to this channel !!!");
			return 0;
		}
		if(chan==1)
		{
			map_ringext[ringphoneid] += string(" ")+channel2;
			for(map<string,string>::iterator it=map_ringext.begin();it!=map_ringext.end();it++)
			{
				string thechan=(*it).second;
				if(thechan == channel2)
				map_ringext.erase(it);
				break;
			}
		}
		else
		{
			map_ringext[ringphoneid] += string(" ")+channel1;
			for(map<string,string>::iterator it=map_ringext.begin();it!=map_ringext.end();it++)
			{
				string thechan=(*it).second;
				if(thechan == channel1)
				map_ringext.erase(it);
				break;
			}
		}
		LoggerWrapper::GetInstance()->Write(LV_STATUS, "Extension %s has channels %s",ringphoneid.c_str(), map_ringext[ringphoneid].c_str());
		manager->NotifyRing("", ringphoneid, map_ringext[ringphoneid]);
	}
	if((ptoken->getKey(TOKEN_EVENT) == EVENT_NEWEXTEN) && ptoken->getKey(TOKEN_APPLICATION) == APPLICATION_CONF)
	{
		string channel = ptoken->getKey(TOKEN_CHANNEL);
		string extension = ptoken->getKey(TOKEN_EXTENSION);
		string ringphoneid;
		if(Utils::ParseChannel(channel, &ringphoneid)) {
			LoggerWrapper::GetInstance()->Write(LV_CRITICAL, "Could not extract phone id from channels");
			return 0;
		}
		LoggerWrapper::GetInstance()->Write(LV_STATUS, "Conference event in room %s with channel %s", extension.c_str(),channel.c_str());		
		if(map_ringext.find(extension) == map_ringext.end())
		{
			map_ringext[extension] = string("C")+extension;
		}
		LoggerWrapper::GetInstance()->Write(LV_STATUS, "Change from %s",map_ringext[extension].c_str());
		map_ringext[extension] += string(" ")+channel;
		LoggerWrapper::GetInstance()->Write(LV_STATUS, "         to %s",map_ringext[extension].c_str());
		manager->NotifyRing("", ringphoneid, map_ringext[extension]);
	}
	return 0;
}

void 
RingDetectHandler::handleTerminate() {
	Release();
}

};
