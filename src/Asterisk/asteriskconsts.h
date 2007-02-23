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
#ifndef __ASTERISKCONSTS_H_
#define __ASTERISKCONSTS_H_

/* tokens */
#define TOKEN_EVENT		"Event"
#define TOKEN_STATE		"State"
#define TOKEN_CHANNEL		"Channel"
#define TOKEN_CHANNEL1		"Channel1"
#define TOKEN_CHANNEL2		"Channel2"
#define TOKEN_EXTRACHANNEL	"ExtraChannel"
#define TOKEN_RESPONSE		"Response"
#define TOKEN_MESSAGE		"Message"
#define TOKEN_CALLERID		"CallerID"
#define TOKEN_APPLICATION	"Application"
#define TOKEN_APPDATA		"AppData"
#define TOKEN_ACTION		"Action"
#define TOKEN_EXTEN		"Exten"
#define TOKEN_EXTENSION		"Extension"
#define TOKEN_CONTEXT		"Context"
#define TOKEN_PRIORITY		"Priority"

/* events */
#define EVENT_NEWCHANNEL 	"Newchannel"
#define EVENT_NEWEXTEN 		"Newexten"
#define EVENT_HANGUP 		"Hangup"
#define EVENT_LINK		"Link"
#define EVENT_NEWSTATE		"Newstate"

/* states */
#define STATE_RING 		"Ring"
#define STATE_RINGING 		"Ringing"

/* responses */
#define RESPONSE_SUCCESS    	"Success"

/* applications */
#define APPLICATION_DIAL	"Dial"
#define APPLICATION_CONF	"Conference"

/*actions*/
#define ACTION_ORIGINATE	"Originate"
#define ACTION_REDIRECT		"Redirect"
#define ACTION_HANGUP		"Hangup"

#endif
